//
// Created by grakra on 23-3-20.
//

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/Orc.h>
#include <llvm-c/OrcEE.h>
#include <llvm-c/TargetMachine.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

static LLVMOrcObjectLayerRef llvm_create_object_layer(void* Ctx, LLVMOrcExecutionSessionRef ES, const char* Triple) {
    LLVMOrcObjectLayerRef objlayer = LLVMOrcCreateRTDyldObjectLinkingLayerWithSectionMemoryManager(ES);

#if defined(HAVE_DECL_LLVMCREATEGDBREGISTRATIONLISTENER) && HAVE_DECL_LLVMCREATEGDBREGISTRATIONLISTENER
    if (jit_debugging_support) {
        LLVMJITEventListenerRef l = LLVMCreateGDBRegistrationListener();

        LLVMOrcRTDyldObjectLinkingLayerRegisterJITEventListener(objlayer, l);
    }
#endif

#if defined(HAVE_DECL_LLVMCREATEPERFJITEVENTLISTENER) && HAVE_DECL_LLVMCREATEPERFJITEVENTLISTENER
    if (jit_profiling_support) {
        LLVMJITEventListenerRef l = LLVMCreatePerfJITEventListener();

        LLVMOrcRTDyldObjectLinkingLayerRegisterJITEventListener(objlayer, l);
    }
#endif

    return objlayer;
}

static void llvm_log_jit_error(void* ctx, LLVMErrorRef error) {
    std::cerr << "Error: " << LLVMGetErrorMessage(error) << std::endl;
}

static void materializationUnitFn() {}
static LLVMErrorRef definitionGeneratorFn(LLVMOrcDefinitionGeneratorRef G, void* Ctx, LLVMOrcLookupStateRef* LS,
                                          LLVMOrcLookupKind K, LLVMOrcJITDylibRef JD, LLVMOrcJITDylibLookupFlags F,
                                          LLVMOrcCLookupSet Names, size_t NamesCount) {
    for (size_t I = 0; I < NamesCount; I++) {
        LLVMOrcCLookupSetElement Element = Names[I];
        LLVMOrcJITTargetAddress Addr = (LLVMOrcJITTargetAddress)(&materializationUnitFn);
        LLVMJITSymbolFlags Flags = {LLVMJITSymbolGenericFlagsWeak, 0};
        LLVMJITEvaluatedSymbol Sym = {Addr, Flags};
        LLVMOrcRetainSymbolStringPoolEntry(Element.Name);
        LLVMJITCSymbolMapPair Pair = {Element.Name, Sym};
        LLVMJITCSymbolMapPair Pairs[] = {Pair};
        LLVMOrcMaterializationUnitRef MU = LLVMOrcAbsoluteSymbols(Pairs, 1);
        LLVMErrorRef Err = LLVMOrcJITDylibDefine(JD, MU);
        if (Err) return Err;
    }
    return LLVMErrorSuccess;
}

void demo() {
    auto llvmCtx = LLVMContextCreate();
    ASSERT_TRUE(LLVMInitializeNativeTarget() == 0);
    auto module = LLVMModuleCreateWithName("m0");
    std::string triple = LLVMNormalizeTargetTriple(LLVMGetDefaultTargetTriple());
    auto target = LLVMGetFirstTarget();
    ASSERT_TRUE(target != nullptr);
    std::string hostCPU = LLVMGetHostCPUName();
    std::string hostCPUFeatures = LLVMGetHostCPUFeatures();
    auto targetMachine = LLVMCreateTargetMachine(target, triple.c_str(), hostCPU.c_str(), hostCPUFeatures.c_str(),
                                                 LLVMCodeGenOptLevel::LLVMCodeGenLevelDefault,
                                                 LLVMRelocMode::LLVMRelocPIC, LLVMCodeModel::LLVMCodeModelDefault);
    auto targetDataLayout = LLVMCreateTargetDataLayout(targetMachine);
    auto dataLayout = LLVMCopyStringRepOfTargetData(targetDataLayout);
    LLVMSetTarget(module, triple.c_str());
    LLVMSetDataLayout(module, dataLayout);
    auto builder = LLVMCreateBuilder();
    std::vector<LLVMTypeRef> funcParams(2, LLVMInt32Type());
    auto funcType = LLVMFunctionType(LLVMInt32Type(), funcParams.data(), funcParams.size(), false);
    auto func = LLVMAddFunction(module, "foobar", funcType);
    LLVMSetLinkage(func, LLVMExternalLinkage);
    LLVMSetVisibility(func, LLVMDefaultVisibility);
    auto block0 = LLVMAppendBasicBlock(func, "entry");
    auto param0 = LLVMGetParam(func, 0);
    auto param1 = LLVMGetParam(func, 1);
    LLVMPositionBuilderAtEnd(builder, block0);
    auto tmp = LLVMBuildAlloca(builder, LLVMInt32Type(), "alloc");
    auto addResult = LLVMBuildAdd(builder, param0, param1, "add");
    LLVMBuildStore(builder, addResult, tmp);
    auto c = LLVMBuildLoad2(builder, LLVMInt32Type(), tmp, "");
    auto d = LLVMConstInt(LLVMInt32Type(), 2, 0);
    auto e = LLVMBuildMul(builder, c, d, "");
    LLVMBuildRet(builder, e);
    LLVMDisposeBuilder(builder);
    //auto buffer = LLVMWriteBitcodeToMemoryBuffer(module);
    //const auto* start = LLVMGetBufferStart(buffer);
    //const auto size = LLVMGetBufferSize(buffer);
    //std::string_view s(start, size);
    //std::cout<<s<<std::endl;
    std::cout << LLVMPrintModuleToString(module) << std::endl;
    auto orcTsCtx = LLVMOrcCreateNewThreadSafeContext();

    LLVMOrcLLJITRef lljit;
    auto orcJitBuilder = LLVMOrcCreateLLJITBuilder();
    auto orcJitTargetMachineBuilder = LLVMOrcJITTargetMachineBuilderCreateFromTargetMachine(targetMachine);
    LLVMOrcLLJITBuilderSetJITTargetMachineBuilder(orcJitBuilder, orcJitTargetMachineBuilder);
    LLVMOrcLLJITBuilderSetObjectLinkingLayerCreator(orcJitBuilder, llvm_create_object_layer, NULL);
    auto err = LLVMOrcCreateLLJIT(&lljit, orcJitBuilder);
    if (err) {
        LOG(ERROR) << "error: " << LLVMGetErrorMessage(err);
    }

    LLVMOrcExecutionSessionSetErrorReporter(LLVMOrcLLJITGetExecutionSession(lljit), llvm_log_jit_error, NULL);

    LLVMOrcDefinitionGeneratorRef main_gen, ref_gen;
    err = LLVMOrcCreateDynamicLibrarySearchGeneratorForProcess(&main_gen, LLVMOrcLLJITGetGlobalPrefix(lljit), 0, NULL);
    if (err) {
        std::cout << "error" << LLVMGetErrorMessage(err) << std::endl;
    }
    LLVMOrcJITDylibAddGenerator(LLVMOrcLLJITGetMainJITDylib(lljit), main_gen);

    ref_gen = LLVMOrcCreateCustomCAPIDefinitionGenerator(definitionGeneratorFn, NULL);
    LLVMOrcJITDylibAddGenerator(LLVMOrcLLJITGetMainJITDylib(lljit), ref_gen);

    auto orcTsModule = LLVMOrcCreateNewThreadSafeModule(module, orcTsCtx);
    LLVMOrcJITDylibRef jd = LLVMOrcLLJITGetMainJITDylib(lljit);
    auto rt = LLVMOrcJITDylibCreateResourceTracker(jd);

    err = LLVMOrcLLJITAddLLVMIRModuleWithRT(lljit, rt, orcTsModule);
    if (err) {
        LOG(ERROR) << "error: " << LLVMGetErrorMessage(err);
    }
    LLVMOrcExecutorAddress addr;
    err = LLVMOrcLLJITLookup(lljit, &addr, "foobar");
    if (err) {
        LOG(ERROR) << "error: " << LLVMGetErrorMessage(err);
    }
    auto f = reinterpret_cast<int (*)(int, int)>(reinterpret_cast<uintptr_t>(addr));
    std::cout << f(10, 100) << std::endl;
}

int main(int argc, char** argv) {
    demo();
    return 0;
}