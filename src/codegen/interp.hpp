#ifndef AURA_CODEGEN_INTERP_HPP
#define AURA_CODEGEN_INTERP_HPP

#include "lang/ir.hpp"
#include "codegen/context.hpp"

namespace aura::codegen {

// Execute IR directly (no JIT), calling the same aura_rt_* functions.
// Scratchpad must be pre-allocated in ctx.scratch with at least
// ctx.num_scratch * ctx.n double slots.
// Slot indices are assigned in the same order as in LLVMCodegen.
void interpret(const lang::IRModule& ir, KernelCtx& ctx);

} // namespace aura::codegen

#endif