set (LLVM_PATHS "/usr/local/lib/llvm")
foreach(llvm_v 11 10 9 8)
    if (NOT LLVM_FOUND)
        find_package (LLVM ${llvm_v} CONFIG PATHS ${LLVM_PATHS})
    endif ()
endforeach ()

if (LLVM_FOUND)
    # Remove dynamically-linked zlib and libedit from LLVM's dependencies:
    set_target_properties(LLVMSupport PROPERTIES INTERFACE_LINK_LIBRARIES "-lpthread;LLVMDemangle;${ZLIB_LIBRARIES}")
    set_target_properties(LLVMLineEditor PROPERTIES INTERFACE_LINK_LIBRARIES "LLVMSupport")
    option(LLVM_HAS_RTTI "Enable if LLVM was build with RTTI enabled" ON)
else()
    message (FATAL_ERROR "Can't find system LLVM")
endif()

if (LLVM_FOUND)
    message(STATUS "LLVM include Directory: ${LLVM_INCLUDE_DIRS}")
    message(STATUS "LLVM library Directory: ${LLVM_LIBRARY_DIRS}")
    message(STATUS "LLVM C++ compiler flags: ${LLVM_CXXFLAGS}")
else()
    message (STATUS "Can't enable LLVM")
endif()

# This list was generated by listing all LLVM libraries, compiling the binary and removing all libraries while it still compiles.
set (REQUIRED_LLVM_LIBRARIES
LLVMOrcJIT
LLVMExecutionEngine
LLVMRuntimeDyld
LLVMX86CodeGen
LLVMX86Desc
LLVMX86Info
LLVMX86Utils
LLVMAsmPrinter
LLVMDebugInfoDWARF
LLVMGlobalISel
LLVMSelectionDAG
LLVMMCDisassembler
LLVMPasses
LLVMCodeGen
LLVMipo
LLVMBitWriter
LLVMInstrumentation
LLVMScalarOpts
LLVMAggressiveInstCombine
LLVMInstCombine
LLVMVectorize
LLVMTransformUtils
LLVMTarget
LLVMAnalysis
LLVMProfileData
LLVMObject
LLVMBitReader
LLVMCore
LLVMRemarks
LLVMBitstreamReader
LLVMMCParser
LLVMMC
LLVMBinaryFormat
LLVMDebugInfoCodeView
LLVMSupport
LLVMDemangle
)

