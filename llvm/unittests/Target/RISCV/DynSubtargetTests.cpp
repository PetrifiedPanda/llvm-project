#include "RISCVDynSubtargetInfo.h"

#include "gtest/gtest.h"

#ifndef RISCV_SOURCE_FILE_DIR
#error "Source file dir not defined"
#endif

using namespace llvm;

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
  const auto& Lst = LstVal.LstVal.Vals;
  ASSERT_EQ(Lst.size(), Size);

  for (size_t I = 0; I < Size; ++I) {
    const auto &Ref = Lst[I];
    ASSERT_EQ(Ref.Type.Kind, VarKind::Ref);
    ASSERT_EQ(Ref.Type.ObjTypeIdx, ObjTypeIdx);
    EXPECT_EQ(Ref.Key, Ids[I]);
    EXPECT_NE(Vals.find(Ref.Key), Vals.end());
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

    const auto &Unsupported = Rocket.find("UnsupportedFeatures")->second;
    const char *RefStrings[] = {
        "HasStdExtZbkb",    "HasStdExtZbkc",       "HasStdExtZbkx",
        "HasStdExtZknd",    "HasStdExtZkne",       "HasStdExtZknh",
        "HasStdExtZksed",   "HasStdExtZksh",       "HasStdExtZkr",
        "HasVInstructions", "HasVInstructionsI64",
    };
    checkRefLst(Unsupported, RefStrings, sizeof RefStrings / sizeof *RefStrings, Rocket, 7);

    const auto &RocketUnitALU = Rocket.find("RocketUnitALU")->second;
    ASSERT_EQ(RocketUnitALU.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitALU.ObjVal.Members, "NumUnits", 1);
    checkInt(RocketUnitALU.ObjVal.Members, "BufferSize", -1);

    const auto &RocketUnitIMul = Rocket.find("RocketUnitIMul")->second;
    ASSERT_EQ(RocketUnitIMul.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitIMul.ObjVal.Members, "NumUnits", 1);
    checkInt(RocketUnitIMul.ObjVal.Members, "BufferSize", 0);

    const auto &RocketUnitB = Rocket.find("RocketUnitB")->second;
    ASSERT_EQ(RocketUnitB.Type.Kind, VarKind::Obj);
    checkInt(RocketUnitB.ObjVal.Members, "NumUnits", 1);
    checkInt(RocketUnitB.ObjVal.Members, "BufferSize", 0);
    const auto &BAssocWrites =
        RocketUnitB.ObjVal.Members.find("AssociatedWrites")->second;
    ASSERT_EQ(BAssocWrites.Type.Kind, VarKind::List);
    ASSERT_EQ(BAssocWrites.Type.ListKind, VarKind::Obj);
    ASSERT_EQ(BAssocWrites.Type.ListObjTypeIdx, 0);

    ASSERT_EQ(BAssocWrites.LstVal.Vals.size(), 1);
    const auto &RBAssocWrite = BAssocWrites.LstVal.Vals[0];
    ASSERT_EQ(RBAssocWrite.Type.Kind, VarKind::Obj);
    ASSERT_EQ(RBAssocWrite.Type.ObjTypeIdx, 0);
    const auto &WriteRes = RBAssocWrite.ObjVal.Members.find("WriteRes")->second;
    // TODO: ResourceCycles
    checkInt(WriteRes.ObjVal.Members, "Latency", 1);
    checkInt(WriteRes.ObjVal.Members, "NumMicroOps", 1);
    const auto &Writes = RBAssocWrite.ObjVal.Members.find("Writes")->second;
    const char *WriteKeys[] = {
        "WriteJmp",
        "WriteJal",
        "WriteJalr",
        "WriteJmpReg",
    };
    checkRefLst(Writes, WriteKeys, sizeof WriteKeys / sizeof *WriteKeys, Rocket, 4);

    const auto &ReadJmp = Rocket.find("ReadJmp")->second;
    ASSERT_EQ(ReadJmp.Type.Kind, VarKind::Obj);
    const auto &ReadJmpRA = ReadJmp.ObjVal.Members.find("ReadAdvance")->second;
    ASSERT_EQ(ReadJmpRA.Type.Kind, VarKind::Obj);
    checkInt(ReadJmpRA.ObjVal.Members, "Cycles", 0);

    const auto &UnusedRA = Rocket.find("UnusedRA")->second;
    ASSERT_EQ(UnusedRA.Type.Kind, VarKind::Obj);
    checkInt(UnusedRA.ObjVal.Members, "Cycles", 1);
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
