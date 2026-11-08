#ifndef AURA_CODEGEN_JIT_HPP
#define AURA_CODEGEN_JIT_HPP

#include "codegen/context.hpp"
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>
#include <memory>

namespace aura::codegen {

class JITEngine {
public:
    JITEngine();
    ~JITEngine();

    JITEngine(const JITEngine&) = delete;
    JITEngine& operator=(const JITEngine&) = delete;

    KernelFn compile(std::unique_ptr<llvm::Module> mod);

private:
    std::unique_ptr<llvm::orc::LLJIT> jit_;
};

} // namespace aura::codegen

#endif