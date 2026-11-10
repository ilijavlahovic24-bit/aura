#include "codegen/interp.hpp"
#include <unordered_map>
#include <cstring>
#include <cmath>
#include <stdexcept>

extern "C" {
    void aura_rt_add(const double*, const double*, double*, int64_t);
    void aura_rt_sub(const double*, const double*, double*, int64_t);
    void aura_rt_mul(const double*, const double*, double*, int64_t);
    void aura_rt_div(const double*, const double*, double*, int64_t);
    void aura_rt_neg(const double*, double*, int64_t);
    void aura_rt_not(const uint8_t*, uint8_t*, int64_t);
    void aura_rt_lt(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_le(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_gt(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_ge(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_eq(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_ne(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_and(const uint8_t*, const uint8_t*, uint8_t*, int64_t);
    void aura_rt_or (const uint8_t*, const uint8_t*, uint8_t*, int64_t);
    void aura_rt_sma(const double*, int64_t, int64_t, double*);
    void aura_rt_ema(const double*, int64_t, int64_t, double*);
    void aura_rt_stdev(const double*, int64_t, int64_t, double*);
    void aura_rt_rsi(const double*, int64_t, int64_t, double*);
    void aura_rt_highest(const double*, int64_t, int64_t, double*);
    void aura_rt_lowest(const double*, int64_t, int64_t, double*);
    void aura_rt_shift(const double*, int64_t, int64_t, double*);
    void aura_rt_crossover(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_crossunder(const double*, const double*, uint8_t*, int64_t);
    void aura_rt_atr(const double*, const double*, const double*, int64_t, int64_t, double*);
    void aura_rt_where(const uint8_t*, const double*, const double*, double*, int64_t);
    void aura_rt_broadcast_f64(double, double*, int64_t);
    void aura_rt_broadcast_u8(uint8_t, uint8_t*, int64_t);
    void aura_rt_copy_u8(const uint8_t*, uint8_t*, int64_t);
    void aura_rt_copy_f64(const double*, double*, int64_t);
    void aura_rt_mod(const double*, const double*, double*, int64_t);
}

namespace aura::codegen {

struct Val {
    enum Kind { F64, I64, Bool, SeriesF64, SeriesU8 } kind;
    double f = 0;
    int64_t i = 0;
    bool b = false;
    double* sf64 = nullptr;
    uint8_t* su8 = nullptr;
};

void interpret(const lang::IRModule& ir, KernelCtx& ctx) {
    std::unordered_map<int, Val> regs;
    int64_t scratch_slot = 0;
    int param_idx = 0;
    int64_t n = ctx.n;

    // One slot = n doubles. For u8 series we use the same slot 
    // (interpreted as n bytes), to make the slot indices the same as in LLVMCodegen.
    auto alloc_f64 = [&]() -> double* {
        double* p = ctx.scratch + scratch_slot * n;
        scratch_slot++;
        return p;
    };
    auto alloc_u8 = [&]() -> uint8_t* {
        uint8_t* p = reinterpret_cast<uint8_t*>(ctx.scratch + scratch_slot * n);
        scratch_slot++;
        return p;
    };

    auto ensure_f64 = [&](int r) -> double* {
        auto& v = regs[r];
        if (v.kind == Val::SeriesF64) return v.sf64;
        double val = (v.kind == Val::I64) ? (double)v.i : v.f;
        double* out = alloc_f64();
        aura_rt_broadcast_f64(val, out, n);
        return out;
    };
    auto ensure_u8 = [&](int r) -> uint8_t* {
        auto& v = regs[r];
        if (v.kind == Val::SeriesU8) return v.su8;
        uint8_t val = v.b ? 1 : 0;
        uint8_t* out = alloc_u8();
        aura_rt_broadcast_u8(val, out, n);
        return out;
    };

    for (const auto& inst : ir.insts) {
        switch (inst.op) {
            case lang::IROp::ConstInt:
                regs[inst.dst] = {Val::I64}; regs[inst.dst].i = inst.i_val; break;
            case lang::IROp::ConstFloat:
                regs[inst.dst] = {Val::F64}; regs[inst.dst].f = inst.f_val; break;
            case lang::IROp::ConstBool:
                regs[inst.dst] = {Val::Bool}; regs[inst.dst].b = inst.b_val; break;

            case lang::IROp::LoadSeries: {
                double* p = nullptr;
                if      (inst.name == "open")   p = const_cast<double*>(ctx.open);
                else if (inst.name == "high")   p = const_cast<double*>(ctx.high);
                else if (inst.name == "low")    p = const_cast<double*>(ctx.low);
                else if (inst.name == "close")  p = const_cast<double*>(ctx.close);
                else if (inst.name == "volume") p = const_cast<double*>(ctx.volume);
                else throw std::runtime_error("interp: unknown series: " + inst.name);
                regs[inst.dst] = {Val::SeriesF64}; regs[inst.dst].sf64 = p;
                break;
            }
            case lang::IROp::LoadParam: {
                double v = ctx.params[param_idx++];
                if (inst.type == lang::Type::Int) {
                    regs[inst.dst] = {Val::I64}; regs[inst.dst].i = (int64_t)v;
                } else {
                    regs[inst.dst] = {Val::F64}; regs[inst.dst].f = v;
                }
                break;
            }
            case lang::IROp::Neg: {
                auto& a = regs[inst.args[0]];
                if (inst.type == lang::Type::SeriesFloat) {
                    double* out = alloc_f64();
                    aura_rt_neg(a.sf64, out, n);
                    regs[inst.dst] = {Val::SeriesF64}; regs[inst.dst].sf64 = out;
                } else if (a.kind == Val::I64) {
                    regs[inst.dst] = {Val::I64}; regs[inst.dst].i = -a.i;
                } else {
                    regs[inst.dst] = {Val::F64}; regs[inst.dst].f = -a.f;
                }
                break;
            }
            case lang::IROp::Not: {
                if (inst.type == lang::Type::SeriesBool) {
                    uint8_t* src = ensure_u8(inst.args[0]);
                    uint8_t* out = alloc_u8();
                    aura_rt_not(src, out, n);
                    regs[inst.dst] = {Val::SeriesU8}; regs[inst.dst].su8 = out;
                } else {
                    regs[inst.dst] = {Val::Bool}; regs[inst.dst].b = !regs[inst.args[0]].b;
                }
                break;
            }

            case lang::IROp::Add: case lang::IROp::Sub:
            case lang::IROp::Mul: case lang::IROp::Div:
            case lang::IROp::Mod: {                                          // <-- DODATO
                if (inst.type == lang::Type::SeriesFloat) {
                    double* lhs = ensure_f64(inst.args[0]);
                    double* rhs = ensure_f64(inst.args[1]);
                    double* out = alloc_f64();
                    switch (inst.op) {
                        case lang::IROp::Add: aura_rt_add(lhs, rhs, out, n); break;
                        case lang::IROp::Sub: aura_rt_sub(lhs, rhs, out, n); break;
                        case lang::IROp::Mul: aura_rt_mul(lhs, rhs, out, n); break;
                        case lang::IROp::Div: aura_rt_div(lhs, rhs, out, n); break;
                        case lang::IROp::Mod: aura_rt_mod(lhs, rhs, out, n); break;   // <-- DODATO
                        default: break;
                    }
                    regs[inst.dst] = {Val::SeriesF64}; regs[inst.dst].sf64 = out;
                } else {
                    double l = regs[inst.args[0]].f;
                    double r = regs[inst.args[1]].f;
                    double v = 0;
                    switch (inst.op) {
                        case lang::IROp::Add: v = l + r; break;
                        case lang::IROp::Sub: v = l - r; break;
                        case lang::IROp::Mul: v = l * r; break;
                        case lang::IROp::Div: v = l / r; break;
                        case lang::IROp::Mod: v = std::fmod(l, r); break;             // <-- DODATO
                        default: break;
                    }
                    regs[inst.dst] = {Val::F64}; regs[inst.dst].f = v;
                }
                break;
            }

            case lang::IROp::Lt: case lang::IROp::Le:
            case lang::IROp::Gt: case lang::IROp::Ge:
            case lang::IROp::Eq: case lang::IROp::Ne: {
                if (inst.type == lang::Type::SeriesBool) {
                    double* lhs = ensure_f64(inst.args[0]);
                    double* rhs = ensure_f64(inst.args[1]);
                    uint8_t* out = alloc_u8();
                    switch (inst.op) {
                        case lang::IROp::Lt: aura_rt_lt(lhs, rhs, out, n); break;
                        case lang::IROp::Le: aura_rt_le(lhs, rhs, out, n); break;
                        case lang::IROp::Gt: aura_rt_gt(lhs, rhs, out, n); break;
                        case lang::IROp::Ge: aura_rt_ge(lhs, rhs, out, n); break;
                        case lang::IROp::Eq: aura_rt_eq(lhs, rhs, out, n); break;
                        case lang::IROp::Ne: aura_rt_ne(lhs, rhs, out, n); break;
                        default: break;
                    }
                    regs[inst.dst] = {Val::SeriesU8}; regs[inst.dst].su8 = out;
                } else {
                    double l = regs[inst.args[0]].f;
                    double r = regs[inst.args[1]].f;
                    bool v = false;
                    switch (inst.op) {
                        case lang::IROp::Lt: v = l < r; break;
                        case lang::IROp::Le: v = l <= r; break;
                        case lang::IROp::Gt: v = l > r; break;
                        case lang::IROp::Ge: v = l >= r; break;
                        case lang::IROp::Eq: v = l == r; break;
                        case lang::IROp::Ne: v = l != r; break;
                        default: break;
                    }
                    regs[inst.dst] = {Val::Bool}; regs[inst.dst].b = v;
                }
                break;
            }

            case lang::IROp::And: case lang::IROp::Or: {
                if (inst.type == lang::Type::SeriesBool) {
                    uint8_t* lhs = ensure_u8(inst.args[0]);
                    uint8_t* rhs = ensure_u8(inst.args[1]);
                    uint8_t* out = alloc_u8();
                    if (inst.op == lang::IROp::And) aura_rt_and(lhs, rhs, out, n);
                    else                            aura_rt_or (lhs, rhs, out, n);
                    regs[inst.dst] = {Val::SeriesU8}; regs[inst.dst].su8 = out;
                } else {
                    bool l = regs[inst.args[0]].b;
                    bool r = regs[inst.args[1]].b;
                    regs[inst.dst] = {Val::Bool};
                    regs[inst.dst].b = (inst.op == lang::IROp::And) ? (l && r) : (l || r);
                }
                break;
            }

            case lang::IROp::Call: {
                const std::string& fn = inst.name;
                if (fn == "sma" || fn == "ema" || fn == "stdev" ||
                    fn == "rsi" || fn == "highest" || fn == "lowest" || fn == "shift") {
                    double* in = regs[inst.args[0]].sf64;
                    auto& wv = regs[inst.args[1]];
                    int64_t w = (wv.kind == Val::I64) ? wv.i : (int64_t)wv.f;
                    double* out = alloc_f64();
                    if      (fn == "sma")     aura_rt_sma(in, n, w, out);
                    else if (fn == "ema")     aura_rt_ema(in, n, w, out);
                    else if (fn == "stdev")   aura_rt_stdev(in, n, w, out);
                    else if (fn == "rsi")     aura_rt_rsi(in, n, w, out);
                    else if (fn == "highest") aura_rt_highest(in, n, w, out);
                    else if (fn == "lowest")  aura_rt_lowest(in, n, w, out);
                    else if (fn == "shift")   aura_rt_shift(in, n, w, out);
                    regs[inst.dst] = {Val::SeriesF64}; regs[inst.dst].sf64 = out;
                } else if (fn == "crossover" || fn == "crossunder") {
                    double* a = regs[inst.args[0]].sf64;
                    double* b = regs[inst.args[1]].sf64;
                    uint8_t* out = alloc_u8();
                    if (fn == "crossover") aura_rt_crossover(a, b, out, n);
                    else                   aura_rt_crossunder(a, b, out, n);
                    regs[inst.dst] = {Val::SeriesU8}; regs[inst.dst].su8 = out;
                } else if (fn == "atr") {
                    auto& wv = regs[inst.args[0]];
                    int64_t w = (wv.kind == Val::I64) ? wv.i : (int64_t)wv.f;
                    double* out = alloc_f64();
                    aura_rt_atr(const_cast<double*>(ctx.high), const_cast<double*>(ctx.low),
                                const_cast<double*>(ctx.close), n, w, out);
                    regs[inst.dst] = {Val::SeriesF64}; regs[inst.dst].sf64 = out;
                } else if (fn == "where") {
                    uint8_t* c = ensure_u8(inst.args[0]);
                    double* a = ensure_f64(inst.args[1]);
                    double* b = ensure_f64(inst.args[2]);
                    double* out = alloc_f64();
                    aura_rt_where(c, a, b, out, n);
                    regs[inst.dst] = {Val::SeriesF64}; regs[inst.dst].sf64 = out;
                } else {
                    throw std::runtime_error("interp: unknown function: " + fn);
                }
                break;
            }

            case lang::IROp::EnterLong:
            case lang::IROp::ExitLong:
            case lang::IROp::EnterShort:
            case lang::IROp::ExitShort: {
                uint8_t* src = ensure_u8(inst.args[0]);
                uint8_t* dst = nullptr;
                if (inst.op == lang::IROp::EnterLong)  dst = ctx.enter_long;
                if (inst.op == lang::IROp::ExitLong)   dst = ctx.exit_long;
                if (inst.op == lang::IROp::EnterShort) dst = ctx.enter_short;
                if (inst.op == lang::IROp::ExitShort)  dst = ctx.exit_short;
                aura_rt_copy_u8(src, dst, n);
                break;
            }
            case lang::IROp::SetSize: {
                double* src = ensure_f64(inst.args[0]);
                aura_rt_copy_f64(src, ctx.size, n);
                break;
            }
            
        }
    }
}

} // namespace aura::codegen