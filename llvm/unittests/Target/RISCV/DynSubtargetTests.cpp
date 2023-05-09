#include "RISCVDynSubtargetInfo.h"

#include "gtest/gtest.h"

#ifndef RISCV_SOURCE_FILE_DIR
#error "Source file dir not defined"
#endif

using namespace llvm;

static const IdentifierMap<VarVal> ExpectedRocket = {
    {"IssueWidth", VarVal{static_cast<int64_t>(1)}},
    {"LoadLatency", VarVal{static_cast<int64_t>(3)}},
    {"MispredictPenalty", VarVal{static_cast<int64_t>(3)}},
    {"CompleteModel", VarVal{false}},
    {"UnsupportedFeatures", VarVal{VarKind::Ref, 7,
                                   std::vector<VarVal>{
                                       VarVal{7, "HasStdExtZbkb"},
                                       VarVal{7, "HasStdExtZbkc"},
                                       VarVal{7, "HasStdExtZbkx"},
                                       VarVal{7, "HasStdExtZknd"},
                                       VarVal{7, "HasStdExtZkne"},
                                       VarVal{7, "HasStdExtZknh"},
                                       VarVal{7, "HasStdExtZksed"},
                                       VarVal{7, "HasStdExtZksh"},
                                       VarVal{7, "HasStdExtZkr"},
                                       VarVal{7, "HasVInstructions"},
                                       VarVal{7, "HasVInstructionsI64"},
                                   }}},
};

static void checkInt(const IdentifierMap<VarVal> &Vals, const char *Key,
                     int64_t Value) {
  const auto &Val = Vals.find(Key)->second;
  ASSERT_EQ(Val.Type.Kind, VarKind::Int);
  ASSERT_EQ(Val.Initialized, true);
  ASSERT_EQ(Val.IntVal, Value);
}

static void checkBool(const IdentifierMap<VarVal> &Vals, const char *Key,
                      bool B) {
  const auto &Val = Vals.find(Key)->second;
  ASSERT_EQ(Val.Type.Kind, VarKind::Bool);
  ASSERT_EQ(Val.Initialized, true);
  ASSERT_EQ(Val.BoolVal, B);
}

static void checkRefLst(const VarVal &LstVal, const char **Ids, size_t Size,
                        const IdentifierMap<VarVal> &Vals, size_t ObjTypeIdx) {
  ASSERT_EQ(LstVal.Type.Kind, VarKind::List);
  ASSERT_EQ(LstVal.Type.ListKind, VarKind::Ref);
  ASSERT_EQ(LstVal.Type.ListObjTypeIdx, ObjTypeIdx);
  const auto &Lst = LstVal.LstVal;
  ASSERT_EQ(Lst.size(), Size);

  for (size_t I = 0; I < Size; ++I) {
    const auto &Ref = Lst[I];
    ASSERT_EQ(Ref.Type.Kind, VarKind::Ref);
    ASSERT_EQ(Ref.Type.ObjTypeIdx, ObjTypeIdx);
    EXPECT_EQ(Ref.RefKey, Ids[I]);
    EXPECT_NE(Vals.find(Ref.RefKey), Vals.end());
  }
}

TEST(DynSubtargetTests, ReadFromFile) {
  auto Got = getDSLArchData(RISCV_SOURCE_FILE_DIR "/test_arch_defs.txt");
  static const char *ExpectedNames[] = {
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

  {
    const auto &Rocket = Got[0].Vals;
    checkInt(Rocket, "IssueWidth", 1);
    checkInt(Rocket, "LoadLatency", 3);
    checkInt(Rocket, "MispredictPenalty", 3);
    checkBool(Rocket, "CompleteModel", false);

    const VarVal &Unsupported = Rocket.find("UnsupportedFeatures")->second;
    ASSERT_EQ(Unsupported, ExpectedRocket.find("UnsupportedFeatures")->second);
    const char *RefStrings[] = {
        "HasStdExtZbkb",    "HasStdExtZbkc",       "HasStdExtZbkx",
        "HasStdExtZknd",    "HasStdExtZkne",       "HasStdExtZknh",
        "HasStdExtZksed",   "HasStdExtZksh",       "HasStdExtZkr",
        "HasVInstructions", "HasVInstructionsI64",
    };
    checkRefLst(Unsupported, RefStrings, sizeof RefStrings / sizeof *RefStrings,
                Rocket, 7);

    const auto &RocketUnitALU = Rocket.find("RocketUnitALU")->second;
    ASSERT_EQ(RocketUnitALU.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitALU.ObjVal, "NumUnits", 1);
    checkInt(RocketUnitALU.ObjVal, "BufferSize", -1);

    const auto &RocketUnitIMul = Rocket.find("RocketUnitIMul")->second;
    ASSERT_EQ(RocketUnitIMul.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitIMul.ObjVal, "NumUnits", 1);
    checkInt(RocketUnitIMul.ObjVal, "BufferSize", 0);

    const auto &RocketUnitB = Rocket.find("RocketUnitB")->second;
    ASSERT_EQ(RocketUnitB.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitB.ObjVal, "NumUnits", 1);
    checkInt(RocketUnitB.ObjVal, "BufferSize", 0);
    const auto &BAssocWrites =
        RocketUnitB.ObjVal.find("AssociatedWrites")->second;
    ASSERT_EQ(BAssocWrites.Type.Kind, VarKind::List);
    ASSERT_EQ(BAssocWrites.Type.ListKind, VarKind::Obj);
    ASSERT_EQ(BAssocWrites.Type.ListObjTypeIdx, 0u);

    ASSERT_EQ(BAssocWrites.LstVal.size(), 1u);
    const auto &RBAssocWrite = BAssocWrites.LstVal[0];
    ASSERT_EQ(RBAssocWrite.Type.Kind, VarKind::Obj);
    ASSERT_EQ(RBAssocWrite.Type.ObjTypeIdx, 0u);
    const auto &WriteRes = RBAssocWrite.ObjVal.find("WriteRes")->second;
    // TODO: ResourceCycles
    checkInt(WriteRes.ObjVal, "Latency", 1);
    checkInt(WriteRes.ObjVal, "NumMicroOps", 1);
    const auto &Writes = RBAssocWrite.ObjVal.find("Writes")->second;
    const char *WriteKeys[] = {
        "WriteJmp",
        "WriteJal",
        "WriteJalr",
        "WriteJmpReg",
    };
    checkRefLst(Writes, WriteKeys, sizeof WriteKeys / sizeof *WriteKeys, Rocket,
                4);

    const auto &ReadJmp = Rocket.find("ReadJmp")->second;
    ASSERT_EQ(ReadJmp.Type.Kind, VarKind::Obj);
    const auto &ReadJmpRA = ReadJmp.ObjVal.find("ReadAdvance")->second;
    ASSERT_EQ(ReadJmpRA.Type.Kind, VarKind::Obj);
    checkInt(ReadJmpRA.ObjVal, "Cycles", 0);

    const auto &UnusedRA = Rocket.find("UnusedRA")->second;
    ASSERT_EQ(UnusedRA.Type.Kind, VarKind::Obj);
    checkInt(UnusedRA.ObjVal, "Cycles", 1);
    // TODO:
  }
  const auto &RocketWithBadMuls = Got[1].Vals;
  checkInt(RocketWithBadMuls, "IssueWidth", 1);
  checkInt(RocketWithBadMuls, "LoadLatency", 3);
  checkInt(RocketWithBadMuls, "MispredictPenalty", 3);
  checkBool(RocketWithBadMuls, "CompleteModel", false);

  const auto &SiFive7 = Got[2].Vals;
  checkInt(SiFive7, "IssueWidth", 2);
  checkInt(SiFive7, "LoadLatency", 3);
  checkInt(SiFive7, "MispredictPenalty", 3);
  checkBool(SiFive7, "CompleteModel", false);
}
