#include "RISCVDynSubtargetInfo.h"

#include "gtest/gtest.h"

TEST(DynSubtargetTests, TestForTestingTests) {
    // TODO: make this not need to use absolute path
    auto Test = llvm::getDynSubtargetData("/home/benedikt/projects/llvm-project/llvm/unittests/Target/RISCV/test_arch_defs.txt");
    ASSERT_FALSE(true);
}
