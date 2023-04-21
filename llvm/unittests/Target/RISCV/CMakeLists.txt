include_directories(
  ${LLVM_MAIN_SRC_DIR}/lib/Target/RISCV
  ${LLVM_BINARY_DIR}/lib/Target/RISCV
  )

set(LLVM_LINK_COMPONENTS RISCVCodeGen)

add_llvm_target_unittest(DynSubtargetTests DynSubtargetTests.cpp)

# set_property(TARGET RISCVTests PROPERTY FOLDER "Tests/UnitTests/TargetTests")
