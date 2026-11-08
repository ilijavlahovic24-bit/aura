#ifndef AURA_CODEGEN_LLVM_CODEGEN_HPP
#define AURA_CODEGEN_LLVM_CODEGEN_HPP

#include "lang/ir.hpp"
#include "lang/type.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace aura::codegen {

class LLVMCodegen {
public:
    LLVMCodegen();
    std::unique_ptr<llvm::Module> generate(const lang::IRModule& ir,const std::string& name = "aura_strategy");
    
    void print_ir(llvm::Module* mod, const std::string& path);

    
    int used_scratch_slots() const { return max_scratch_slots_; }

private:
    llvm::LLVMContext ctx_;
    llvm::IRBuilder<> builder_;
    llvm::StructType* kernel_ctx_ty_ = nullptr;

    int max_scratch_slots_ = 0; 

    void init_types();
    llvm::Type* to_llvm_type(lang::Type t);

    llvm::FunctionCallee get_runtime_fn(llvm::Module* mod,const std::string& name,llvm::Type* ret,llvm::ArrayRef<llvm::Type*> args);

    llvm::Value* alloc_scratch(llvm::Value* scratch_base,llvm::Value* n_val,int slot_idx);

    llvm::Value* load_ctx_field(llvm::Value* ctx, int idx,const std::string& name, llvm::Type* pointee);
};

} // namespace aura::codegen

#endif