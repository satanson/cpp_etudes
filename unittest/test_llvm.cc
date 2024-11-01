// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2022/11/12.
//
#include <glog/logging.h>
#include <gtest/gtest.h>
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

#include <llvm.hh>
#include <memory>
using namespace llvm::orc;
using namespace llvm;
namespace test {
class LLVMTest : public ::testing::Test {};
typedef struct OpaqueObj* ObjRef;
TEST_F(LLVMTest, testBasic) {
    std::string s = "abc";
    ObjRef s0 = reinterpret_cast<ObjRef>(&s);
    std::string* s2 = reinterpret_cast<std::string*>(s0);
    std::cout << *s2 << std::endl;
}

TEST_F(LLVMTest, testBackInserter) {
    std::vector<int> xs{0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<int> ys;
    ys.reserve(xs.size());
    auto has3 = false;
    std::copy_if(xs.begin(), xs.end(), std::back_inserter(ys), [&has3](int x) {
        has3 |= x == 3;
        return x % 2 == 1;
    });
    for (int i = 0; i < ys.size(); ++i) {
        std::cout << ys[i] << std::endl;
    }
    std::cout << "has3:" << has3 << std::endl;
}
struct Referee {
public:
    explicit Referee(const std::string& s) : _s(s) {}
    ~Referee() = default;
    void print() const { std::cout << _s << std::endl; }

private:
    std::string _s;
};
using RefereePtr = std::shared_ptr<Referee>;
using Referees = std::vector<RefereePtr>;
struct Reference {
public:
    Reference(const Referees& referees) : _referees(referees) {}
    ~Reference() {
        std::for_each(_referees.begin(), _referees.end(), [](const auto& ref) { ref->print(); });
    }

private:
    Referees _referees;
};
using ReferencePtr = std::shared_ptr<Reference>;
using RefereesAndReference = std::tuple<Referees, ReferencePtr>;
RefereesAndReference getRefereesAndRef() {
    Referees referees;
    referees.reserve(10);
    for (int i = 0; i < 10; ++i) {
        referees.push_back(std::make_shared<Referee>("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"));
    }
    return std::make_tuple(referees, std::make_shared<Reference>(referees));
}
TEST_F(LLVMTest, testTupleReference) {
    std::cout << "begin" << std::endl;
    {
        auto [referees, ref] = getRefereesAndRef();
        std::cout << referees.size() << std::endl;
    }
    std::cout << "end" << std::endl;
}

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
        auto Addr = (LLVMOrcJITTargetAddress)(&materializationUnitFn);
        LLVMJITSymbolFlags Flags = {LLVMJITSymbolGenericFlagsWeak, 0};
        LLVMJITEvaluatedSymbol Sym = {Addr, Flags};
        LLVMOrcRetainSymbolStringPoolEntry(Element.Name);
        LLVMOrcCSymbolMapPair Pair = {Element.Name, Sym};
        LLVMOrcCSymbolMapPair Pairs[] = {Pair};
        LLVMOrcMaterializationUnitRef MU = LLVMOrcAbsoluteSymbols(Pairs, 1);
        LLVMErrorRef Err = LLVMOrcJITDylibDefine(JD, MU);
        if (Err) return Err;
    }
    return LLVMErrorSuccess;
}

TEST_F(LLVMTest, testBasic2) {
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

    ref_gen = LLVMOrcCreateCustomCAPIDefinitionGenerator(definitionGeneratorFn, nullptr, nullptr);
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

TEST_F(LLVMTest, testBasic3) {
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    auto EPC = llvm::orc::SelfExecutorProcessControl::Create();
    if (!EPC) {
        errs() << EPC.takeError();
        return;
    }
    auto ES = std::make_unique<llvm::orc::ExecutionSession>(std::move(*EPC));
    llvm::orc::JITTargetMachineBuilder JTMB(ES->getExecutorProcessControl().getTargetTriple());
    auto DL = cantFail(JTMB.getDefaultDataLayoutForTarget());
    using llvm::orc::MangleAndInterner;
    using llvm::orc::IRCompileLayer;
    MangleAndInterner Mangle(*ES, DL);
    RTDyldObjectLinkingLayer ObjectLayer(*ES, []() { return std::make_unique<SectionMemoryManager>(); });
    IRCompileLayer CompileLayer(*ES, ObjectLayer, std::make_unique<ConcurrentIRCompiler>(JTMB));
    JITDylib& MainJD = ES->createBareJITDylib("<main>");
    MainJD.addGenerator(cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(DL.getGlobalPrefix())));

    if (JTMB.getTargetTriple().isOSBinFormatCOFF()) {
        ObjectLayer.setOverrideObjectFlagsWithResponsibilityFlags(true);
        ObjectLayer.setAutoClaimResponsibilityForObjectSymbols(true);
    }

    auto TheContext = std::make_unique<LLVMContext>();
    auto TheModule = std::make_unique<Module>("my cool jit", *TheContext);
    TheModule->setDataLayout(DL);
    TheModule->setTargetTriple(JTMB.getTargetTriple().getTriple());
    // Create a new builder for the module.
    auto Builder = std::make_unique<IRBuilder<>>(*TheContext);
    std::vector<llvm::Type*> funcParamTypes(2, llvm::Type::getInt32Ty(*TheContext));
    auto funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*TheContext), funcParamTypes, false);
    auto func = Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, "foobar", TheModule.get());
    auto block0 = BasicBlock::Create(*TheContext, "entry", func, nullptr);
    auto arg0 = func->getArg(0);
    auto arg1 = func->getArg(1);
    Builder->SetInsertPoint(block0);
    auto tmp = Builder->CreateAlloca(llvm::Type::getInt32Ty(*TheContext), nullptr, "alloc");
    auto addResult = Builder->CreateAdd(arg0, arg1, "add");
    Builder->CreateStore(addResult, tmp);
    auto c = Builder->CreateLoad(llvm::Type::getInt32Ty(*TheContext), tmp);
    auto d = ConstantInt::get(llvm::Type::getInt32Ty(*TheContext), 2, true);
    auto e = Builder->CreateMul(c, d, "mul");
    Builder->CreateRet(e);

    // Create a new pass manager attached to it.
    auto TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());
    TheFPM->doInitialization();
    TheModule->print(outs(), nullptr);
    TheFPM->run(*func);
    TheModule->print(outs(), nullptr);

    auto RT = MainJD.createResourceTracker();

    auto TSM = ThreadSafeModule(std::move(TheModule), std::move(TheContext));
    RT = MainJD.getDefaultResourceTracker();
    cantFail(CompileLayer.add(RT, std::move(TSM)));

    StringRef Name = "foobar";
    auto ExprSymbol = cantFail(ES->lookup({&MainJD}, Mangle(Name.str())));

    // Get the symbol's address and cast it to the right type (takes no
    // arguments, returns a double) so we can call it as a native function.
    auto f = (int (*)(int, int))static_cast<intptr_t>(ExprSymbol.getAddress().getValue());
    std::cout << "result=" << f(100, 20) << std::endl;

    // Delete the anonymous expression module from the JIT.
    ExitOnError ExitOnError;
    ExitOnError(RT->remove());
    if (auto Err = ES->endSession()) {
        ES->reportError(std::move(Err));
    }
}

TEST_F(LLVMTest, testBasic4) {
    LLVMInitialization();
    auto jit_result = EtudesJIT::create();
    ASSERT_TRUE(jit_result.ok());
    auto jit = std::move(jit_result.value());

    auto& context = jit->get_llvm_context();
    auto& module = jit->get_module();
    auto Builder = std::make_unique<IRBuilder<>>(*context);
    std::vector<llvm::Type*> funcParamTypes(2, llvm::Type::getInt32Ty(*context));
    auto funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), funcParamTypes, false);
    auto func = Function::Create(funcType, llvm::GlobalValue::ExternalLinkage, "foobar", module.get());
    auto block0 = BasicBlock::Create(*context, "entry", func, nullptr);
    auto arg0 = func->getArg(0);
    auto arg1 = func->getArg(1);
    Builder->SetInsertPoint(block0);
    auto tmp = Builder->CreateAlloca(llvm::Type::getInt32Ty(*context), nullptr, "alloc");
    auto addResult = Builder->CreateAdd(arg0, arg1, "add");
    Builder->CreateStore(addResult, tmp);
    auto c = Builder->CreateLoad(llvm::Type::getInt32Ty(*context), tmp);
    auto d = ConstantInt::get(llvm::Type::getInt32Ty(*context), 2, true);
    auto e = Builder->CreateMul(c, d, "mul");
    Builder->CreateRet(e);

    jit->opt(true);
    auto addr = jit->look_up("foobar");

    auto f = (int (*)(int, int))addr;
    std::cout << "result=" << f(100, 20) << std::endl;
}

TEST_F(LLVMTest, testRoundToPowerTwo) {
    ASSERT_EQ(RoundToPowerTwo<0b00001111>::value, 0b00010000);
    ASSERT_EQ(RoundToPowerTwo<0b01001111>::value, 0b10000000);
    ASSERT_EQ(RoundToPowerTwo<0b01000000>::value, 0b01000000);
}

} // namespace test
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}