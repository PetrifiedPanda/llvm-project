#include "RISCVDynSubtargetInfo.h"

#include "gtest/gtest.h"

#ifndef RISCV_SOURCE_FILE_DIR
#error "Source file dir not defined"
#endif

using namespace llvm;

static void checkInt(const IdentifierMap<VarVal>& Vals, const char* Key, int64_t Value) {
    const auto& Val = Vals.find(Key)->second;
    ASSERT_EQ(Val.Type.Kind, VarKind::Int);
    ASSERT_EQ(Val.IntVal, Value);
}

static void checkBool(const IdentifierMap<VarVal>& Vals, const char* Key, bool B) {
    const auto& Val = Vals.find(Key)->second;
    ASSERT_EQ(Val.Type.Kind, VarKind::Bool);
    ASSERT_EQ(Val.BoolVal, B);
}

TEST(DynSubtargetTests, ReadFromFile) {
    auto Got = getDSLArchData(RISCV_SOURCE_FILE_DIR "/test_arch_defs.txt");
    static const char* ExpectedNames[] = {
        "Rocket",
        "RocketWithBadMuls",
        "SiFive7",
        "SyntacoreSCR1Model",
    };
    
    constexpr size_t NumArchs = sizeof ExpectedNames / sizeof *ExpectedNames;
    ASSERT_EQ(NumArchs, Got.size());
    for (size_t I = 0; I < NumArchs; ++I) {
        ASSERT_EQ(ExpectedNames[I], Got[I].Name);
    }

    const auto& Rocket = Got[0].Vals;
    checkInt(Rocket, "IssueWidth", 1);
    checkInt(Rocket, "LoadLatency", 3);
    checkInt(Rocket, "MispredictPenalty", 3);
    checkBool(Rocket, "CompleteModel", false);

    const auto& RocketUnitALU = Rocket.find("RocketUnitALU")->second;
    ASSERT_EQ(RocketUnitALU.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitALU.ObjVal.Members, "NumUnits", 1);
    checkInt(RocketUnitALU.ObjVal.Members, "BufferSize", -1);

    const auto& RocketUnitIMul = Rocket.find("RocketUnitIMul")->second;
    ASSERT_EQ(RocketUnitIMul.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitIMul.ObjVal.Members, "NumUnits", 1);
    checkInt(RocketUnitIMul.ObjVal.Members, "BufferSize", 0);

    const auto& RocketWithBadMuls = Got[1].Vals;
    checkInt(RocketWithBadMuls, "IssueWidth", 1);
    checkInt(RocketWithBadMuls, "LoadLatency", 3);
    checkInt(RocketWithBadMuls, "MispredictPenalty", 3);
    checkBool(RocketWithBadMuls, "CompleteModel", false);

    const auto& SiFive7 = Got[2].Vals;
    checkInt(SiFive7, "IssueWidth", 2);
    checkInt(SiFive7, "LoadLatency", 3);
    checkInt(SiFive7, "MispredictPenalty", 3);
    checkBool(SiFive7, "CompleteModel", false);
}
