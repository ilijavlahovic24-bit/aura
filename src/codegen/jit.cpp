#include "codegen/jit.hpp"
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <stdexcept>

using namespace llvm;
using namespace llvm::orc;

namespace aura::codegen {

static void check(Error err, const std::string& ctx) {
    if (err) {
        std::string msg;
        raw_string_ostream os(msg);
        os << ctx << ": " << err;
        throw std::runtime_error(os.str());
    }
}

JITEngine::JITEngine() {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();

    auto jit_or_err = LLJITBuilder().create();
    check(jit_or_err.takeError(), "failed to create LLJIT");
    jit_ = std::move(*jit_or_err);

    auto& JD = jit_->getMainJITDylib();
    auto gen = DynamicLibrarySearchGenerator::GetForCurrentProcess(
        jit_->getDataLayout().getGlobalPrefix());
    check(gen.takeError(), "failed to get symbol generator");
    JD.addGenerator(std::move(*gen));
}

JITEngine::~JITEngine() = default;

KernelFn JITEngine::compile(std::unique_ptr<Module> mod) {
    // Prepiši DataLayout modula JIT-ovim (native target layout).
    // LLVM 17 na x86-64 ne uključuje i128:128, a naš hardkodovani layout ga ima.
    mod->setDataLayout(jit_->getDataLayout());

    auto tsm = ThreadSafeModule(std::move(mod),
                                ThreadSafeContext(std::make_unique<LLVMContext>()));

    auto err = jit_->addIRModule(std::move(tsm));
    check(std::move(err), "failed to add module");

    auto sym = jit_->lookup("kernel");
    check(sym.takeError(), "failed to lookup 'kernel'");

    void* addr = reinterpret_cast<void*>(sym->getValue());
    return reinterpret_cast<KernelFn>(addr);
}

} // namespace aura::codegen