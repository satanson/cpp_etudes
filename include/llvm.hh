//
// Created by grakra on 23-7-26.
//

#pragma once
#include <absl/status/statusor.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/Orc.h>
#include <llvm-c/OrcEE.h>
#include <llvm-c/TargetMachine.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ExecutionEngine/Orc/ExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

using llvm::orc::SelfExecutorProcessControl;
using llvm::orc::ExecutionSession;
using llvm::orc::JITTargetMachineBuilder;
using llvm::DataLayout;
using llvm::orc::MangleAndInterner;
using llvm::orc::IRCompileLayer;
using llvm::orc::RTDyldObjectLinkingLayer;
using llvm::SectionMemoryManager;
using llvm::orc::JITDylib;
using llvm::orc::DynamicLibrarySearchGenerator;
using llvm::orc::ConcurrentIRCompiler;
using llvm::LLVMContext;
using llvm::Module;
using absl::StatusOr;
using absl::Status;
struct LLVMInitialization {
    LLVMInitialization() {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
    }
};
class EtudesJIT;
using EtudesJITPtr = std::unique_ptr<EtudesJIT>;
class EtudesJIT {
private:
    static Status llvm_error_to_status_error(const llvm::Error& error) {
        std::string s;
        llvm::raw_string_ostream rso(s);
        rso << error;
        return Status(absl::StatusCode::kInternal, rso.str());
    }

    std::unique_ptr<ExecutionSession> _es;
    JITTargetMachineBuilder _jit_tmb;
    llvm::DataLayout _data_layout;
    MangleAndInterner _mangling;
    std::unique_ptr<RTDyldObjectLinkingLayer> _object_layer;
    std::unique_ptr<IRCompileLayer> _compiler_layer;
    JITDylib& _main_jd;
    std::unique_ptr<LLVMContext> _context;
    std::unique_ptr<Module> _module;

    EtudesJIT(const EtudesJIT&) = delete;

public:
    EtudesJIT(std::unique_ptr<ExecutionSession>&& es, JITTargetMachineBuilder&& jit_tmb, llvm::DataLayout&& data_layout,
              std::unique_ptr<RTDyldObjectLinkingLayer>&& object_layer, std::unique_ptr<IRCompileLayer>&& compile_layer,
              JITDylib& main_jd)
            : _es(std::move(es)),
              _jit_tmb(std::move(jit_tmb)),
              _data_layout(std::move(data_layout)),
              _mangling(*_es, _data_layout),
              _object_layer(std::move(object_layer)),
              _compiler_layer(std::move(compile_layer)),
              _main_jd(main_jd),
              _context(std::make_unique<LLVMContext>()),
              _module(std::make_unique<Module>("jit", *_context)) {
        _module->setDataLayout(_data_layout);
        _module->setTargetTriple(_jit_tmb.getTargetTriple().getTriple());
    }

    ~EtudesJIT() { llvm::cantFail(_es->endSession()); }

    static StatusOr<EtudesJITPtr> create() {
        auto epc = SelfExecutorProcessControl::Create();
        if (!epc) {
            return llvm_error_to_status_error(epc.takeError());
        }
        auto es = std::make_unique<ExecutionSession>(std::move(*epc));

        JITTargetMachineBuilder jit_tmb(es->getExecutorProcessControl().getTargetTriple());
        auto data_layout = jit_tmb.getDefaultDataLayoutForTarget();
        if (!data_layout) {
            return llvm_error_to_status_error(data_layout.takeError());
        }

        std::unique_ptr<RTDyldObjectLinkingLayer> object_layer = std::make_unique<RTDyldObjectLinkingLayer>(
                *es, []() { return std::make_unique<SectionMemoryManager>(); });

        std::unique_ptr<IRCompileLayer> compiler_layer =
                std::make_unique<IRCompileLayer>(*es, *object_layer, std::make_unique<ConcurrentIRCompiler>(jit_tmb));
        JITDylib& main_jd = es->createBareJITDylib("<main>");
        auto search_generator = DynamicLibrarySearchGenerator::GetForCurrentProcess(data_layout->getGlobalPrefix());
        if (!search_generator) {
            return llvm_error_to_status_error(search_generator.takeError());
        }
        main_jd.addGenerator(std::move(*search_generator));
        if (jit_tmb.getTargetTriple().isOSBinFormatCOFF()) {
            object_layer->setOverrideObjectFlagsWithResponsibilityFlags(true);
            object_layer->setAutoClaimResponsibilityForObjectSymbols(true);
        }

        return std::make_unique<EtudesJIT>(std::move(es), std::move(jit_tmb), std::move(*data_layout),
                                           std::move(object_layer), std::move(compiler_layer), main_jd);
    }
    void opt(bool debug) {
        auto fn_pass_mgr = std::make_unique<llvm::legacy::FunctionPassManager>(_module.get());

        // Do simple "peephole" optimizations and bit-twiddling optzns.
        fn_pass_mgr->add(llvm::createInstructionCombiningPass());
        // Reassociate expressions.
        fn_pass_mgr->add(llvm::createReassociatePass());
        // Eliminate Common SubExpressions.
        fn_pass_mgr->add(llvm::createGVNPass());
        // Simplify the control flow graph (deleting unreachable blocks, etc).
        fn_pass_mgr->add(llvm::createCFGSimplificationPass());
        fn_pass_mgr->doInitialization();
        if (debug) {
            llvm::outs() << "Before optimization:\n";
            _module->print(llvm::outs(), nullptr);
        }
        for (auto& fn : _module->getFunctionList()) {
            fn_pass_mgr->run(fn);
        }
        if (debug) {
            llvm::outs() << "After optimization:\n";
            _module->print(llvm::outs(), nullptr);
        }
    }

    std::unique_ptr<LLVMContext>& get_llvm_context() { return _context; }
    std::unique_ptr<Module>& get_module() { return _module; };
    std::unique_ptr<llvm::orc::ThreadSafeModule> _ts_module;
    uintptr_t look_up(const std::string& fn_name) {
        auto res_tracker = _main_jd.getDefaultResourceTracker();
        if (!res_tracker) {
            res_tracker = _main_jd.createResourceTracker();
        }
        DCHECK(res_tracker);
        if (!_ts_module) {
            auto ts_module = llvm::orc::ThreadSafeModule(std::move(_module), std::move(_context));
            cantFail(_compiler_layer->add(res_tracker, std::move(ts_module)));
        }

        auto symbol = cantFail(_es->lookup({&_main_jd}, _mangling(fn_name)));
        // Get the symbol's address and cast it to the right type (takes no
        // arguments, returns a double) so we can call it as a native function.
        return reinterpret_cast<uintptr_t>(symbol.getAddress());
    }
};

template <int N, int M>
struct RoundToPowerTwoTemp {
    static constexpr int value = RoundToPowerTwoTemp<(N >> 1), (M + 1)>::value;
};

template <int M>
struct RoundToPowerTwoTemp<0, M> {
    static constexpr int value = 1 << (M);
};

template <int N>
struct RoundToPowerTwo {
    static constexpr int value = RoundToPowerTwoTemp<N, 0>::value;
};