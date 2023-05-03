add_llvm_component_group(RISCV)

set(LLVM_TARGET_DEFINITIONS RISCV.td)

tablegen(LLVM RISCVGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM RISCVGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM RISCVGenCompressInstEmitter.inc -gen-compress-inst-emitter)
tablegen(LLVM RISCVGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM RISCVGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM RISCVGenGlobalISel.inc -gen-global-isel)
tablegen(LLVM RISCVGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM RISCVGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM RISCVGenMCPseudoLowering.inc -gen-pseudo-lowering)
tablegen(LLVM RISCVGenRegisterBank.inc -gen-register-bank)
tablegen(LLVM RISCVGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM RISCVGenSearchableTables.inc -gen-searchable-tables)
tablegen(LLVM RISCVGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(RISCVCommonTableGen)

# Source: https://github.com/antlr/antlr4/tree/master/runtime/Cpp/cmake 
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# required if linking to static library
add_definitions(-DANTLR4CPP_STATIC)

# using /MD flag for antlr4_runtime (for Visual C++ compilers only)
set(ANTLR4_WITH_STATIC_CRT OFF)

# Specify the version of the antlr4 library needed for this project.
# By default the latest version of antlr4 will be used.  You can specify a
# specific, stable version by setting a repository tag value or a link
# to a zip file containing the libary source.
#set(ANTLR4_TAG 4.11.1)
#set(ANTLR4_ZIP_REPOSITORY https://github.com/antlr/antlr4/archive/refs/tags/4.11.1.zip)

# add external build for antlrcpp
include(ExternalAntlr4Cpp)
# add antrl4cpp artifacts to project environment
include_directories(${ANTLR4_INCLUDE_DIRS})
# set variable pointing to the antlr tool that supports C++
# this is not required if the jar file can be found under PATH environment
set(ANTLR_EXECUTABLE /usr/local/lib/antlr-4.11.1-complete.jar)
# add macros to generate ANTLR Cpp code from grammar
find_package(ANTLR REQUIRED)

antlr_target(DSLLexParse DSLGrammar.g4 LEXER PARSER LISTENER PACKAGE antlr_dsl)

add_llvm_target(RISCVCodeGen
  RISCVAsmPrinter.cpp
  RISCVCodeGenPrepare.cpp
  RISCVMakeCompressible.cpp
  RISCVExpandAtomicPseudoInsts.cpp
  RISCVExpandPseudoInsts.cpp
  RISCVFrameLowering.cpp
  RISCVGatherScatterLowering.cpp
  RISCVInsertVSETVLI.cpp
  RISCVInstrInfo.cpp
  RISCVISelDAGToDAG.cpp
  RISCVISelLowering.cpp
  RISCVMachineFunctionInfo.cpp
  RISCVMacroFusion.cpp
  RISCVMCInstLower.cpp
  RISCVMergeBaseOffset.cpp
  RISCVOptWInstrs.cpp
  RISCVRedundantCopyElimination.cpp
  RISCVRegisterInfo.cpp
  RISCVRVVInitUndef.cpp
  RISCVSubtarget.cpp
  RISCVTargetMachine.cpp
  RISCVTargetObjectFile.cpp
  RISCVTargetTransformInfo.cpp
  RISCVDynSubtargetInfo.cpp
  DSL.cpp
  GISel/RISCVCallLowering.cpp
  GISel/RISCVInstructionSelector.cpp
  GISel/RISCVLegalizerInfo.cpp
  GISel/RISCVRegisterBankInfo.cpp
  ${ANTLR_DSLLexParse_CXX_OUTPUTS}
  
  ADDITIONAL_HEADER_DIRS
  ${ANTLR_DSLLexParse_OUTPUT_DIR}
  ${ANTLR4_INCLUDE_DIRS}

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  Core
  IPO
  CodeGen
  MC
  RISCVDesc
  RISCVInfo
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils
  GlobalISel
  LINK_LIBS
  antlr4_static
  ${ANTLR4_RUNTIME_LIBRARIES}

  ADD_TO_COMPONENT
  RISCV
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(MCA)
add_subdirectory(TargetInfo)
