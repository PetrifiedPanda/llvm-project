#include "RISCVDynSubtargetInfo.h"

#include "gtest/gtest.h"

#ifndef RISCV_SOURCE_FILE_DIR
#error "Source file dir not defined"
#endif

TEST(DynSubtargetTests, TestForTestingTests) {
    auto Test = llvm::getDynSubtargetData(RISCV_SOURCE_FILE_DIR "/test_arch_defs.txt");
    ASSERT_FALSE(true);
}
