#include "codegen/llvm_codegen.hpp"
#include "codegen/context.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <stdexcept>

using namespace llvm;

namespace aura::codegen {

// ---------- Init ----------
LLVMCodegen::LLVMCodegen() : builder_(ctx_) {
    init_types();
}

void LLVMCodegen::init_types() {
    kernel_ctx_ty_ = StructType::create(ctx_, "KernelCtx");
    Type* ptr_ty = builder_.getPtrTy();
    Type* i64_ty = builder_.getInt64Ty();

    kernel_ctx_ty_->setBody({
        ptr_ty, ptr_ty, ptr_ty, ptr_ty, ptr_ty,   // open, high, low, close, volume
        ptr_ty,                                    // params
        ptr_ty, ptr_ty, ptr_ty, ptr_ty,            // enter/exit arrays
        ptr_ty,                                    // size
        ptr_ty,                                    // scratch
        i64_ty, i64_ty                             // n, num_scratch
    });
}

llvm::Type* LLVMCodegen::to_llvm_type(lang::Type t) {
    switch (t) {
        case lang::Type::Int:         return builder_.getInt64Ty();
        case lang::Type::Float:       return builder_.getDoubleTy();
        case lang::Type::Bool:        return builder_.getInt1Ty();
        case lang::Type::SeriesFloat: return builder_.getPtrTy();
        case lang::Type::SeriesBool:  return builder_.getPtrTy();
        case lang::Type::Void:        return builder_.getVoidTy();
        default: throw std::runtime_error("invalid type for LLVM");
    }
}

// ---------- Helpers ----------
FunctionCallee LLVMCodegen::get_runtime_fn(Module* mod, const std::string& name,
                                           Type* ret, ArrayRef<Type*> args) {
    FunctionType* ft = FunctionType::get(ret, args, false);
    return mod->getOrInsertFunction(name, ft);
}

Value* LLVMCodegen::alloc_scratch(Value* scratch_base, Value* n_val, int slot_idx) {
    Value* mul = builder_.CreateMul(n_val, builder_.getInt64(slot_idx), "scr_off");
    return builder_.CreateGEP(builder_.getDoubleTy(), scratch_base, mul, "scr_slot");
}

Value* LLVMCodegen::load_ctx_field(Value* ctx, int idx, const std::string& name,
                                    Type* pointee) {
    Value* gep = builder_.CreateStructGEP(kernel_ctx_ty_, ctx, idx, name + "_ptr");
    return builder_.CreateLoad(pointee, gep, name);
}

// ---------- Main codegen ----------
std::unique_ptr<Module> LLVMCodegen::generate(const lang::IRModule& ir, const std::string& name) {
    auto mod = std::make_unique<Module>(name, ctx_);

    // Standardni x86-64 SysV DataLayout (za ispravan align u IR-u)
    mod->setDataLayout(
        "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128");

    // void kernel(const KernelCtx* ctx)
    Type* void_ty   = builder_.getVoidTy();
    Type* ptr_ty    = builder_.getPtrTy();
    Type* i64_ty    = builder_.getInt64Ty();
    Type* double_ty = builder_.getDoubleTy();
    Type* i8_ty     = builder_.getInt8Ty();

    FunctionType* fn_ty = FunctionType::get(void_ty, {ptr_ty}, false);
    Function* fn = Function::Create(fn_ty, Function::ExternalLinkage, "kernel", mod.get());

    BasicBlock* entry = BasicBlock::Create(ctx_, "entry", fn);
    builder_.SetInsertPoint(entry);

    Value* ctx = fn->arg_begin();

    // Učitaj n i scratch_base i params_base
    Value* n_val = load_ctx_field(ctx, 12, "n", i64_ty);
    Value* scratch_base = load_ctx_field(ctx, 11, "scratch", ptr_ty);
    Value* params_base  = load_ctx_field(ctx, 5,  "params",  ptr_ty);

    std::unordered_map<int, Value*> vregs;
    std::unordered_map<int, lang::Type> vreg_types;

    int scratch_slot = 0;
    int param_idx    = 0;

    // --- ensure_series_f64: ako je operand skalar (Int/Float), broadcast-uj u double* ---
    auto ensure_series_f64 = [&](int vreg_idx) -> Value* {
        Value* v = vregs[vreg_idx];
        lang::Type t = vreg_types[vreg_idx];
        if (t == lang::Type::SeriesFloat) return v;
        if (t == lang::Type::Int) {
            v = builder_.CreateSIToFP(v, double_ty, "i2f");
        }
        Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
        auto fnc = get_runtime_fn(mod.get(), "aura_rt_broadcast_f64",
            void_ty, {double_ty, ptr_ty, i64_ty});
        builder_.CreateCall(fnc, {v, out, n_val});
        return out;
    };

    // --- ensure_series_u8: ako je operand scalar Bool, broadcast-uj u uint8_t* ---
    auto ensure_series_u8 = [&](int vreg_idx) -> Value* {
        Value* v = vregs[vreg_idx];
        lang::Type t = vreg_types[vreg_idx];
        if (t == lang::Type::SeriesBool) return v;
        // scalar Bool (i1) → u8 → broadcast
        Value* u8 = builder_.CreateZExt(v, i8_ty, "b2u8");
        Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
        auto fnc = get_runtime_fn(mod.get(), "aura_rt_broadcast_u8",
            void_ty, {i8_ty, ptr_ty, i64_ty});
        builder_.CreateCall(fnc, {u8, out, n_val});
        return out;
    };

    for (const auto& inst : ir.insts) {
        switch (inst.op) {
            // -------- Konstante --------
            case lang::IROp::ConstInt:
                vregs[inst.dst] = builder_.getInt64(inst.i_val);
                vreg_types[inst.dst] = lang::Type::Int;
                break;
            case lang::IROp::ConstFloat:
                vregs[inst.dst] = ConstantFP::get(double_ty, inst.f_val);
                vreg_types[inst.dst] = lang::Type::Float;
                break;
            case lang::IROp::ConstBool:
                vregs[inst.dst] = builder_.getInt1(inst.b_val);
                vreg_types[inst.dst] = lang::Type::Bool;
                break;

            // -------- LoadSeries --------
            case lang::IROp::LoadSeries: {
                int idx = -1;
                if      (inst.name == "open")   idx = 0;
                else if (inst.name == "high")   idx = 1;
                else if (inst.name == "low")    idx = 2;
                else if (inst.name == "close")  idx = 3;
                else if (inst.name == "volume") idx = 4;
                else throw std::runtime_error("unknown series: " + inst.name);

                vregs[inst.dst] = load_ctx_field(ctx, idx, inst.name, ptr_ty);
                vreg_types[inst.dst] = lang::Type::SeriesFloat;
                break;
            }

            // -------- LoadParam --------
            case lang::IROp::LoadParam: {
                Value* gep = builder_.CreateGEP(double_ty, params_base,
                                                builder_.getInt64(param_idx), "param_ptr");
                Value* d = builder_.CreateLoad(double_ty, gep, inst.name + "_d");
                if (inst.type == lang::Type::Int) {
                    vregs[inst.dst] = builder_.CreateFPToSI(d, i64_ty, inst.name + "_i");
                } else {
                    vregs[inst.dst] = d;
                }
                vreg_types[inst.dst] = inst.type;
                param_idx++;
                break;
            }

            // -------- Unary --------
            case lang::IROp::Neg: {
                if (inst.type == lang::Type::SeriesFloat) {
                    Value* a = vregs[inst.args[0]];  // unary neg preserves type → već ptr
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), "aura_rt_neg",
                        void_ty, {ptr_ty, ptr_ty, i64_ty});
                    builder_.CreateCall(fnc, {a, out, n_val});
                    vregs[inst.dst] = out;
                } else {
                    vregs[inst.dst] = builder_.CreateFNeg(vregs[inst.args[0]], "neg");
                }
                vreg_types[inst.dst] = inst.type;
                break;
            }
            case lang::IROp::Not: {
                if (inst.type == lang::Type::SeriesBool) {
                    Value* a = ensure_series_u8(inst.args[0]);
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), "aura_rt_not",
                        void_ty, {ptr_ty, ptr_ty, i64_ty});
                    builder_.CreateCall(fnc, {a, out, n_val});
                    vregs[inst.dst] = out;
                } else {
                    vregs[inst.dst] = builder_.CreateNot(vregs[inst.args[0]], "not");
                }
                vreg_types[inst.dst] = inst.type;
                break;
            }

            // -------- Aritmetika --------
            case lang::IROp::Add:
            case lang::IROp::Sub:
            case lang::IROp::Mul:
            case lang::IROp::Div: {
                if (inst.type == lang::Type::SeriesFloat) {
                    Value* lhs = ensure_series_f64(inst.args[0]);
                    Value* rhs = ensure_series_f64(inst.args[1]);
                    const char* rt_name = nullptr;
                    switch (inst.op) {
                        case lang::IROp::Add: rt_name = "aura_rt_add"; break;
                        case lang::IROp::Sub: rt_name = "aura_rt_sub"; break;
                        case lang::IROp::Mul: rt_name = "aura_rt_mul"; break;
                        case lang::IROp::Div: rt_name = "aura_rt_div"; break;
                        default: throw std::runtime_error("bad binop");
                    }
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), rt_name,
                        void_ty, {ptr_ty, ptr_ty, ptr_ty, i64_ty});
                    builder_.CreateCall(fnc, {lhs, rhs, out, n_val});
                    vregs[inst.dst] = out;
                } else {
                    Value* lhs = vregs[inst.args[0]];
                    Value* rhs = vregs[inst.args[1]];
                    if      (inst.op == lang::IROp::Add) vregs[inst.dst] = builder_.CreateFAdd(lhs, rhs, "add");
                    else if (inst.op == lang::IROp::Sub) vregs[inst.dst] = builder_.CreateFSub(lhs, rhs, "sub");
                    else if (inst.op == lang::IROp::Mul) vregs[inst.dst] = builder_.CreateFMul(lhs, rhs, "mul");
                    else                                 vregs[inst.dst] = builder_.CreateFDiv(lhs, rhs, "div");
                }
                vreg_types[inst.dst] = inst.type;
                break;
            }

            // -------- Poređenja --------
            case lang::IROp::Lt: case lang::IROp::Le:
            case lang::IROp::Gt: case lang::IROp::Ge:
            case lang::IROp::Eq: case lang::IROp::Ne: {
                if (inst.type == lang::Type::SeriesBool) {
                    Value* lhs = ensure_series_f64(inst.args[0]);
                    Value* rhs = ensure_series_f64(inst.args[1]);
                    const char* rt_name = nullptr;
                    switch (inst.op) {
                        case lang::IROp::Lt: rt_name = "aura_rt_lt"; break;
                        case lang::IROp::Le: rt_name = "aura_rt_le"; break;
                        case lang::IROp::Gt: rt_name = "aura_rt_gt"; break;
                        case lang::IROp::Ge: rt_name = "aura_rt_ge"; break;
                        case lang::IROp::Eq: rt_name = "aura_rt_eq"; break;
                        case lang::IROp::Ne: rt_name = "aura_rt_ne"; break;
                        default: throw std::runtime_error("bad cmp");
                    }
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), rt_name,
                        void_ty, {ptr_ty, ptr_ty, ptr_ty, i64_ty});
                    builder_.CreateCall(fnc, {lhs, rhs, out, n_val});
                    vregs[inst.dst] = out;
                } else {
                    Value* lhs = vregs[inst.args[0]];
                    Value* rhs = vregs[inst.args[1]];
                    CmpInst::Predicate pred = CmpInst::FCMP_OEQ;
                    switch (inst.op) {
                        case lang::IROp::Lt: pred = CmpInst::FCMP_OLT; break;
                        case lang::IROp::Le: pred = CmpInst::FCMP_OLE; break;
                        case lang::IROp::Gt: pred = CmpInst::FCMP_OGT; break;
                        case lang::IROp::Ge: pred = CmpInst::FCMP_OGE; break;
                        case lang::IROp::Eq: pred = CmpInst::FCMP_OEQ; break;
                        case lang::IROp::Ne: pred = CmpInst::FCMP_UNE; break;
                        default: break;
                    }
                    vregs[inst.dst] = builder_.CreateFCmp(pred, lhs, rhs, "cmp");
                }
                vreg_types[inst.dst] = inst.type;
                break;
            }

            // -------- Boolean --------
            case lang::IROp::And:
            case lang::IROp::Or: {
                if (inst.type == lang::Type::SeriesBool) {
                    Value* lhs = ensure_series_u8(inst.args[0]);
                    Value* rhs = ensure_series_u8(inst.args[1]);
                    const char* rt = (inst.op == lang::IROp::And) ? "aura_rt_and" : "aura_rt_or";
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), rt,
                        void_ty, {ptr_ty, ptr_ty, ptr_ty, i64_ty});
                    builder_.CreateCall(fnc, {lhs, rhs, out, n_val});
                    vregs[inst.dst] = out;
                } else {
                    Value* lhs = vregs[inst.args[0]];
                    Value* rhs = vregs[inst.args[1]];
                    vregs[inst.dst] = (inst.op == lang::IROp::And)
                        ? builder_.CreateAnd(lhs, rhs, "and")
                        : builder_.CreateOr(lhs, rhs, "or");
                }
                vreg_types[inst.dst] = inst.type;
                break;
            }

            // -------- Call (indikatori) --------
            case lang::IROp::Call: {
                const std::string& fn_name = inst.name;
                if (fn_name == "sma" || fn_name == "ema" || fn_name == "stdev" ||
                    fn_name == "rsi" || fn_name == "highest" || fn_name == "lowest" ||
                    fn_name == "shift") {
                    Value* in = vregs[inst.args[0]];
                    Value* w  = vregs[inst.args[1]];
                    if (vreg_types[inst.args[1]] == lang::Type::Float) {
                        w = builder_.CreateFPToSI(w, i64_ty, "w_i");
                    }
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), "aura_rt_" + fn_name,
                        void_ty, {ptr_ty, i64_ty, i64_ty, ptr_ty});
                    builder_.CreateCall(fnc, {in, n_val, w, out});
                    vregs[inst.dst] = out;
                } else if (fn_name == "crossover" || fn_name == "crossunder") {
                    Value* a = vregs[inst.args[0]];
                    Value* b = vregs[inst.args[1]];
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), "aura_rt_" + fn_name,
                        void_ty, {ptr_ty, ptr_ty, ptr_ty, i64_ty});
                    builder_.CreateCall(fnc, {a, b, out, n_val});
                    vregs[inst.dst] = out;
                } else if (fn_name == "atr") {
                    Value* h = load_ctx_field(ctx, 1, "high", ptr_ty);
                    Value* l = load_ctx_field(ctx, 2, "low",  ptr_ty);
                    Value* c = load_ctx_field(ctx, 3, "close", ptr_ty);
                    Value* w = vregs[inst.args[0]];
                    if (vreg_types[inst.args[0]] == lang::Type::Float) {
                        w = builder_.CreateFPToSI(w, i64_ty, "w_i");
                    }
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), "aura_rt_atr",
                        void_ty, {ptr_ty, ptr_ty, ptr_ty, i64_ty, i64_ty, ptr_ty});
                    builder_.CreateCall(fnc, {h, l, c, n_val, w, out});
                    vregs[inst.dst] = out;
                } else if (fn_name == "where") {
                    Value* c = vregs[inst.args[0]];
                    Value* a = ensure_series_f64(inst.args[1]);
                    Value* b = ensure_series_f64(inst.args[2]);
                    Value* out = alloc_scratch(scratch_base, n_val, scratch_slot++);
                    auto fnc = get_runtime_fn(mod.get(), "aura_rt_where",
                        void_ty, {ptr_ty, ptr_ty, ptr_ty, ptr_ty, i64_ty});
                    builder_.CreateCall(fnc, {c, a, b, out, n_val});
                    vregs[inst.dst] = out;
                } else {
                    throw std::runtime_error("unknown function: " + fn_name);
                }
                vreg_types[inst.dst] = inst.type;
                break;
            }

            // -------- Signals --------
            case lang::IROp::EnterLong:
            case lang::IROp::ExitLong:
            case lang::IROp::EnterShort:
            case lang::IROp::ExitShort: {
                int idx = -1;
                if (inst.op == lang::IROp::EnterLong)  idx = 6;
                if (inst.op == lang::IROp::ExitLong)   idx = 7;
                if (inst.op == lang::IROp::EnterShort) idx = 8;
                if (inst.op == lang::IROp::ExitShort)  idx = 9;
                Value* dst = load_ctx_field(ctx, idx, "signal", ptr_ty);
                Value* src = ensure_series_u8(inst.args[0]);
                auto fnc = get_runtime_fn(mod.get(), "aura_rt_copy_u8",
                    void_ty, {ptr_ty, ptr_ty, i64_ty});
                builder_.CreateCall(fnc, {src, dst, n_val});
                break;
            }

            // -------- SetSize --------
            case lang::IROp::SetSize: {
                Value* dst = load_ctx_field(ctx, 10, "size", ptr_ty);
                Value* src = ensure_series_f64(inst.args[0]);
                auto fnc = get_runtime_fn(mod.get(), "aura_rt_copy_f64",
                    void_ty, {ptr_ty, ptr_ty, i64_ty});
                builder_.CreateCall(fnc, {src, dst, n_val});
                break;
            }

            default:
                break;
        }
    }

    builder_.CreateRetVoid();
    max_scratch_slots_ = scratch_slot;
    return mod;
}

void LLVMCodegen::print_ir(Module* mod, const std::string& path) {
    std::error_code EC;
    raw_fd_ostream dest(path, EC, sys::fs::OF_None);
    if (!EC) mod->print(dest, nullptr);
}

} // namespace aura::codegen