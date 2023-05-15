#include "RISCVDynSubtargetInfo.h"

#include "antlr4-common.h"
#include "antlr4-runtime.h"
#include "antlr4cpp_generated_src/DSLGrammar/DSLGrammarBaseListener.h"
#include "antlr4cpp_generated_src/DSLGrammar/DSLGrammarLexer.h"
#include "antlr4cpp_generated_src/DSLGrammar/DSLGrammarParser.h"

#include "DSL.h"

namespace llvm {

using namespace antlr_dsl;

// TODO: remove all unneeded overrides
class DSLListener final : public DSLGrammarBaseListener {
  inline static const IdentifierMap<VarVal> DefaultWriteRes = {
      {"ResourceCycles",
       VarVal{VarKind::Int, VarType::InvalidTypeIdx, std::vector<VarVal>{}}},
      {"Latency", VarVal(static_cast<int64_t>(1))},
      {"NumMicroOps", VarVal(static_cast<int64_t>(1))},
  };
  inline static const IdentifierMap<VarVal> DefaultAssociatedWrite = {
      {"WriteRes", VarVal(2, DefaultWriteRes)},
      {"Writes", VarVal{VarKind::Ref, 4, std::vector<VarVal>{}}},
  };
  inline static const IdentifierMap<VarVal> DefaultProcResource = {
      {"NumUnits", VarVal{static_cast<int64_t>(1)}},
      {"Super", VarVal{1, ""}},
      {"BufferSize", VarVal{static_cast<int64_t>(-1)}},
      {"AssociatedWrites", VarVal{VarKind::Obj, 0, std::vector<VarVal>{}}},
  };
  inline static const IdentifierMap<VarVal> DefaultReadAdvance = {
      {"Cycles", VarVal::createUninitialized(VarType{VarKind::Int})},
      {"ValidWrites",
       VarVal{VarKind::Ref, 4, std::vector<VarVal>{}}}, // list<SchedWrite>
  };

  inline static const IdentifierMap<VarVal> DefaultProcResGroup = {
      {"Resources", VarVal{VarKind::Ref, 1, std::vector<VarVal>{}}},
      {"AssociatedWrites", VarVal{VarKind::Obj, 0, std::vector<VarVal>{}}},
  };

  inline static const IdentifierMap<VarVal> DefaultSchedWrite = {};

  inline static const IdentifierMap<VarVal> DefaultSchedRead = {
      {"ReadAdvance", VarVal(3, DefaultReadAdvance)},
  };

  inline static const IdentifierMap<VarVal> DefaultRISCVFeature = {};
  // TODO: maybe add constants for these indices
  inline static const StringMap<size_t> TypeMap = {
      {"AssociatedWrite", 0}, {"ProcResource", 1}, {"WriteRes", 2},
      {"ReadAdvance", 3},     {"SchedWrite", 4},   {"ProcResGroup", 5},
      {"SchedRead", 6},       {"RISCVFeature", 7},
  };

  inline static const Vec<IdentifierMap<VarVal>> TypeDefaults = {
      DefaultAssociatedWrite,

      DefaultProcResource,

      DefaultWriteRes,

      DefaultReadAdvance,

      DefaultSchedWrite,

      DefaultProcResGroup,

      DefaultSchedRead,

      DefaultRISCVFeature,
  };
  inline static const IdentifierMap<VarVal> DefaultArchVals = {
      // Values from MCSchedModel
      {"IssueWidth", VarVal{static_cast<int64_t>(1)}},
      {"LoadLatency", VarVal{static_cast<int64_t>(4)}},
      {"HighLatency", VarVal{static_cast<int64_t>(10)}},
      {"MispredictPenalty", VarVal{static_cast<int64_t>(10)}},
      {"CompleteModel", VarVal{false}},
      {"UnsupportedFeatures", VarVal{VarKind::Ref, 7, std::vector<VarVal>{}}},
      {"NoProcResource",
       VarVal{1, DefaultProcResource}}, // TODO: not sure if this should be
                                        // DefaultProcResource
      {"ReadSFB", VarVal{6, DefaultSchedRead}},
      {"ReadJmp", VarVal{6, DefaultSchedRead}},
      {"ReadJalr", VarVal{6, DefaultSchedRead}},
      {"ReadCSR", VarVal{6, DefaultSchedRead}},
      {"ReadMemBase", VarVal{6, DefaultSchedRead}},
      {"ReadFMemBase", VarVal{6, DefaultSchedRead}},
      {"ReadStoreData", VarVal{6, DefaultSchedRead}},
      {"ReadFStoreData", VarVal{6, DefaultSchedRead}},
      {"ReadIALU", VarVal{6, DefaultSchedRead}},
      {"ReadIALU32", VarVal{6, DefaultSchedRead}},
      {"ReadShiftImm", VarVal{6, DefaultSchedRead}},
      {"ReadShiftImm32", VarVal{6, DefaultSchedRead}},
      {"ReadShiftReg", VarVal{6, DefaultSchedRead}},
      {"ReadShiftReg32", VarVal{6, DefaultSchedRead}},
      {"ReadIDiv", VarVal{6, DefaultSchedRead}},
      {"ReadIDiv32", VarVal{6, DefaultSchedRead}},
      {"ReadIMul", VarVal{6, DefaultSchedRead}},
      {"ReadIMul32", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicWA", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicWD", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicDA", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicDD", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicLDW", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicLDD", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicSTW", VarVal{6, DefaultSchedRead}},
      {"ReadAtomicSTD", VarVal{6, DefaultSchedRead}},
      {"ReadFAdd16", VarVal{6, DefaultSchedRead}},
      {"ReadFAdd32", VarVal{6, DefaultSchedRead}},
      {"ReadFAdd64", VarVal{6, DefaultSchedRead}},
      {"ReadFMul16", VarVal{6, DefaultSchedRead}},
      {"ReadFMul32", VarVal{6, DefaultSchedRead}},
      {"ReadFMul64", VarVal{6, DefaultSchedRead}},
      {"ReadFMA16", VarVal{6, DefaultSchedRead}},
      {"ReadFMA32", VarVal{6, DefaultSchedRead}},
      {"ReadFMA64", VarVal{6, DefaultSchedRead}},
      {"ReadFDiv16", VarVal{6, DefaultSchedRead}},
      {"ReadFDiv32", VarVal{6, DefaultSchedRead}},
      {"ReadFDiv64", VarVal{6, DefaultSchedRead}},
      {"ReadFSqrt16", VarVal{6, DefaultSchedRead}},
      {"ReadFSqrt32", VarVal{6, DefaultSchedRead}},
      {"ReadFSqrt64", VarVal{6, DefaultSchedRead}},
      {"ReadFCmp16", VarVal{6, DefaultSchedRead}},
      {"ReadFCmp32", VarVal{6, DefaultSchedRead}},
      {"ReadFCmp64", VarVal{6, DefaultSchedRead}},
      {"ReadFSGNJ16", VarVal{6, DefaultSchedRead}},
      {"ReadFSGNJ32", VarVal{6, DefaultSchedRead}},
      {"ReadFSGNJ64", VarVal{6, DefaultSchedRead}},
      {"ReadFMinMax16", VarVal{6, DefaultSchedRead}},
      {"ReadFMinMax32", VarVal{6, DefaultSchedRead}},
      {"ReadFMinMax64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF16ToI32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF16ToI64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF32ToI32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF32ToI64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF64ToI32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF64ToI64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI32ToF16", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI32ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI32ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI64ToF16", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI64ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI64ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFMovF16ToI16", VarVal{6, DefaultSchedRead}},
      {"ReadFMovI16ToF16", VarVal{6, DefaultSchedRead}},
      {"ReadFMovF32ToI32", VarVal{6, DefaultSchedRead}},
      {"ReadFMovI32ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFMovF64ToI64", VarVal{6, DefaultSchedRead}},
      {"ReadFMovI64ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF32ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF64ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF16ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF32ToF16", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF16ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF64ToF16", VarVal{6, DefaultSchedRead}},
      {"ReadFClass16", VarVal{6, DefaultSchedRead}},
      {"ReadFClass32", VarVal{6, DefaultSchedRead}},
      {"ReadFClass64", VarVal{6, DefaultSchedRead}},

      // RISCV Features
      {"HasStdExtZbkb", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZbkc", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZbkx", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZknd", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZkne", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZknh", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZksed", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZksh", VarVal{7, DefaultRISCVFeature}},
      {"HasStdExtZkr", VarVal{7, DefaultRISCVFeature}},
      {"HasVInstructions", VarVal{7, DefaultRISCVFeature}},
      {"HasVInstructionsI64", VarVal{7, DefaultRISCVFeature}},
      // SchedWrites
      {"WriteIALU", VarVal{4, DefaultSchedWrite}},
      {"WriteIALU32", VarVal{4, DefaultSchedWrite}},
      {"WriteShiftImm", VarVal{4, DefaultSchedWrite}},
      {"WriteShiftImm32", VarVal{4, DefaultSchedWrite}},
      {"WriteShiftReg", VarVal{4, DefaultSchedWrite}},
      {"WriteShiftReg32", VarVal{4, DefaultSchedWrite}},
      {"WriteIDiv", VarVal{4, DefaultSchedWrite}},
      {"WriteIDiv32", VarVal{4, DefaultSchedWrite}},
      {"WriteIMul", VarVal{4, DefaultSchedWrite}},
      {"WriteIMul32", VarVal{4, DefaultSchedWrite}},
      {"WriteJmp", VarVal{4, DefaultSchedWrite}},
      {"WriteJal", VarVal{4, DefaultSchedWrite}},
      {"WriteJalr", VarVal{4, DefaultSchedWrite}},
      {"WriteJmpReg", VarVal{4, DefaultSchedWrite}},
      {"WriteNop", VarVal{4, DefaultSchedWrite}},
      {"WriteLDB", VarVal{4, DefaultSchedWrite}},
      {"WriteLDH", VarVal{4, DefaultSchedWrite}},
      {"WriteLDW", VarVal{4, DefaultSchedWrite}},
      {"WriteLDD", VarVal{4, DefaultSchedWrite}},
      {"WriteCSR", VarVal{4, DefaultSchedWrite}},
      {"WriteSTB", VarVal{4, DefaultSchedWrite}},
      {"WriteSTH", VarVal{4, DefaultSchedWrite}},
      {"WriteSTW", VarVal{4, DefaultSchedWrite}},
      {"WriteSTD", VarVal{4, DefaultSchedWrite}},
      {"WriteAtomicW", VarVal{4, DefaultSchedWrite}},
      {"WriteAtomicD", VarVal{4, DefaultSchedWrite}},
      {"WriteAtomicLDW", VarVal{4, DefaultSchedWrite}},
      {"WriteAtomicLDD", VarVal{4, DefaultSchedWrite}},
      {"WriteAtomicSTW", VarVal{4, DefaultSchedWrite}},
      {"WriteAtomicSTD", VarVal{4, DefaultSchedWrite}},
      {"WriteFAdd16", VarVal{4, DefaultSchedWrite}},
      {"WriteFAdd32", VarVal{4, DefaultSchedWrite}},
      {"WriteFAdd64", VarVal{4, DefaultSchedWrite}},
      {"WriteFMul16", VarVal{4, DefaultSchedWrite}},
      {"WriteFMul32", VarVal{4, DefaultSchedWrite}},
      {"WriteFMul64", VarVal{4, DefaultSchedWrite}},
      {"WriteFMA16", VarVal{4, DefaultSchedWrite}},
      {"WriteFMA32", VarVal{4, DefaultSchedWrite}},
      {"WriteFMA64", VarVal{4, DefaultSchedWrite}},
      {"WriteFDiv16", VarVal{4, DefaultSchedWrite}},
      {"WriteFDiv32", VarVal{4, DefaultSchedWrite}},
      {"WriteFDiv64", VarVal{4, DefaultSchedWrite}},
      {"WriteFSqrt16", VarVal{4, DefaultSchedWrite}},
      {"WriteFSqrt32", VarVal{4, DefaultSchedWrite}},
      {"WriteFSqrt64", VarVal{4, DefaultSchedWrite}},
      // Int to float
      {"WriteFCvtI32ToF16", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtI32ToF32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtI32ToF64", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtI64ToF16", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtI64ToF32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtI64ToF64", VarVal{4, DefaultSchedWrite}},
      // Float to Int
      {"WriteFCvtF16ToI32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF16ToI64", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF32ToI32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF32ToI64", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF64ToI32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF64ToI64", VarVal{4, DefaultSchedWrite}},
      // Float to Float
      {"WriteFCvtF32ToF64", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF64ToF32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF16ToF32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF32ToF16", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF16ToF64", VarVal{4, DefaultSchedWrite}},
      {"WriteFCvtF64ToF16", VarVal{4, DefaultSchedWrite}},

      {"WriteFClass16", VarVal{4, DefaultSchedWrite}},
      {"WriteFClass32", VarVal{4, DefaultSchedWrite}},
      {"WriteFClass64", VarVal{4, DefaultSchedWrite}},
      {"WriteFCmp16", VarVal{4, DefaultSchedWrite}},
      {"WriteFCmp32", VarVal{4, DefaultSchedWrite}},
      {"WriteFCmp64", VarVal{4, DefaultSchedWrite}},
      {"WriteFSGNJ16", VarVal{4, DefaultSchedWrite}},
      {"WriteFSGNJ32", VarVal{4, DefaultSchedWrite}},
      {"WriteFSGNJ64", VarVal{4, DefaultSchedWrite}},
      {"WriteFMinMax16", VarVal{4, DefaultSchedWrite}},
      {"WriteFMinMax32", VarVal{4, DefaultSchedWrite}},
      {"WriteFMinMax64", VarVal{4, DefaultSchedWrite}},

      {"WriteFMovF16ToI16", VarVal{4, DefaultSchedWrite}},
      {"WriteFMovI16ToF16", VarVal{4, DefaultSchedWrite}},
      {"WriteFMovF32ToI32", VarVal{4, DefaultSchedWrite}},
      {"WriteFMovI32ToF32", VarVal{4, DefaultSchedWrite}},
      {"WriteFMovF64ToI64", VarVal{4, DefaultSchedWrite}},
      {"WriteFMovI64ToF64", VarVal{4, DefaultSchedWrite}},

      {"WriteFLD16", VarVal{4, DefaultSchedWrite}},
      {"WriteFLD32", VarVal{4, DefaultSchedWrite}},
      {"WriteFLD64", VarVal{4, DefaultSchedWrite}},
      {"WriteFST16", VarVal{4, DefaultSchedWrite}},
      {"WriteFST32", VarVal{4, DefaultSchedWrite}},
      {"WriteFST64", VarVal{4, DefaultSchedWrite}},
      {"WriteSFB", VarVal{4, DefaultSchedWrite}},
  };

  struct VarInfo {
    std::string Id;
    VarVal Val;
  };
  Vec<std::string> ArchNames;
  Vec<IdentifierMap<VarVal>> PrevArchVals;
  IdentifierMap<VarVal> CurrentArchVals;
  Vec<VarInfo> VarStack;
  // Stores the size of VarStack before current overwrite
  Vec<size_t> OverwriteStack;
  std::string CurrentDefID = "";
  int64_t CurrentArchIdx = -1;

public:
  Vec<DSLArchData> getArchData();

private:
  void enterArchitectureDefinition(
      DSLGrammarParser::ArchitectureDefinitionContext *AD) override;
  void exitArchitectureDefinition(
      DSLGrammarParser::ArchitectureDefinitionContext *AD) override;

  void enterDefinition(DSLGrammarParser::DefinitionContext *Def) override;
  void exitDefinition(DSLGrammarParser::DefinitionContext *Def) override;

  void enterOverwrite(DSLGrammarParser::OverwriteContext *OW) override;
  void exitOverwrite(DSLGrammarParser::OverwriteContext *OW) override;

  void enterMemberAccess(DSLGrammarParser::MemberAccessContext *MA) override;

  void enterType(DSLGrammarParser::TypeContext *T) override;

  void enterListType(DSLGrammarParser::ListTypeContext *LT) override;
  void exitListType(DSLGrammarParser::ListTypeContext *LT) override;

  void enterExpr(DSLGrammarParser::ExprContext *Expr) override;
  void exitExpr(DSLGrammarParser::ExprContext *Expr) override;

  void enterObj(DSLGrammarParser::ObjContext *Obj) override;

  void visitErrorNode(antlr4::tree::ErrorNode *Err) override;

  const VarVal &getVarVal(const std::string &Spell) const;

  int64_t getArchIndex(const std::string &Name) const;

  VarType getCurrVarType() const;
  const std::string *getCurrVarSpell() const;
  VarVal getDefaultValue(VarKind Kind, size_t ObjTypeIdx) const;
  void writeVal(const std::string &Id, VarVal &&Val);
};

Vec<DSLArchData> DSLListener::getArchData() {
    Vec<DSLArchData> Res;
    const size_t Size = CurrentArchIdx + 1;
    Res.reserve(Size);
    for (size_t I = 0; I < Size; ++I) {
      Res.push_back(
          DSLArchData{std::move(ArchNames[I]), std::move(PrevArchVals[I])});
    }
    return Res;
  }

int64_t DSLListener::getArchIndex(const std::string &Name) const {
  for (int64_t I = 0; static_cast<size_t>(I) < ArchNames.size(); ++I) {
    if (ArchNames[I] == Name) {
      return I;
    }
  }
  return -1;
}

void DSLListener::enterArchitectureDefinition(
    DSLGrammarParser::ArchitectureDefinitionContext *AD) {
  const auto &Ids = AD->ID();
  assert(Ids.size() == 1 || Ids.size() == 2);

  if (Ids.size() == 2) {
    std::string InheritName = Ids[1]->toString();
    const int64_t ArchIndex = getArchIndex(InheritName);
    if (ArchIndex == -1) {
      // TODO: correct error
      throw "could not find arch def";
    }
    CurrentArchVals = PrevArchVals[ArchIndex];
  } else {
    CurrentArchVals = DefaultArchVals;
  }
  ++CurrentArchIdx;
}

void DSLListener::exitArchitectureDefinition(
    DSLGrammarParser::ArchitectureDefinitionContext *AD) {
  assert(VarStack.size() == 0);
  std::string ArchName = AD->ID()[0]->toString();
  PrevArchVals.push_back(std::move(CurrentArchVals));

  ArchNames.push_back(std::move(ArchName));
}

void DSLListener::enterDefinition(DSLGrammarParser::DefinitionContext *Def) {
  // Set CurrentDefID for enterType
  CurrentDefID = Def->ID()->toString();
}
void DSLListener::exitDefinition(DSLGrammarParser::DefinitionContext *Def) {
  assert(VarStack.size() == 1);
  VarInfo Curr = std::move(VarStack.back());
  VarStack.pop_back();
  writeVal(Curr.Id, std::move(Curr.Val));
}

const VarVal &DSLListener::getVarVal(const std::string &Spell) const {
  if (VarStack.empty()) {
    auto Found = CurrentArchVals.find(Spell);
    if (Found == CurrentArchVals.end()) {
      // TODO: correct error
      throw std::runtime_error{"Undefined reference to " + Spell};
    }
    return Found->second;
  }

  const VarVal &Curr = VarStack.back().Val;
  // TODO: refs allowed?
  if (Curr.Type.Kind != VarKind::Obj) {
    throw std::runtime_error{"Expected object type"};
  }
  const auto &ToSearch = Curr.ObjVal;
  auto Found = ToSearch.find(Spell);
  if (Found == ToSearch.end()) {
    // TODO: correct error
    throw std::runtime_error{"Undefined reference to " + Spell};
  }
  return Found->second;
}

VarType DSLListener::getCurrVarType() const {
  if (VarStack.size() != 0) {
    return VarStack[VarStack.size() - 1].Val.Type;
  }
  return {VarKind::Invalid, VarType::InvalidTypeIdx};
}

const std::string *DSLListener::getCurrVarSpell() const {
  if (VarStack.size() == 0) {
    return nullptr;
  }
  return &VarStack[VarStack.size() - 1].Id;
}

void DSLListener::enterOverwrite(DSLGrammarParser::OverwriteContext *OW) {
  OverwriteStack.push_back(VarStack.size());
}

void DSLListener::writeVal(const std::string &Id, VarVal &&Val) {
  auto It = CurrentArchVals.find(Id);
  assert(It != CurrentArchVals.end());
  // TODO: Type check
  It->second = std::move(Val);
}

void DSLListener::exitOverwrite(DSLGrammarParser::OverwriteContext *OW) {
  VarType Type = getCurrVarType();
  assert(Type.Kind != VarKind::Invalid);
  // TODO: Check assignment types and write CurrentVal
  size_t NewSize = OverwriteStack.back();
  OverwriteStack.pop_back();
  while (VarStack.size() > NewSize) {
    VarInfo Curr = std::move(VarStack.back());
    VarStack.pop_back();
    if (VarStack.empty()) {
      writeVal(Curr.Id, std::move(Curr.Val));
    } else {
      VarInfo &Prev = VarStack.back();
      assert(Prev.Val.Type.Kind == VarKind::Obj);
      Prev.Val.writeMember(Curr.Id, std::move(Curr.Val));
    }
  }
}

void DSLListener::enterMemberAccess(DSLGrammarParser::MemberAccessContext *MA) {
  for (auto *Id : MA->ID()) {
    std::string Spell = Id->toString();
    const VarVal &Val = getVarVal(Spell);
    assert(Val.Type.Kind != VarKind::Invalid);
    VarStack.push_back(VarInfo{Spell, Val});
  }
}

// This method assumes that this is always called after enterDefinition has
// set CurrentDefID
void DSLListener::enterType(DSLGrammarParser::TypeContext *T) {
  assert(CurrentDefID != "");
  VarVal CurrentDefVal;
  if (T->ID() != nullptr) {
    std::string TypeName = T->ID()->toString();
    auto It = TypeMap.find(TypeName);
    if (It == TypeMap.end()) {
      // TODO: proper error
      throw std::runtime_error{"Unknown type: " + TypeName};
    }
    size_t TypeIdx = It->getValue();
    assert(TypeIdx < TypeDefaults.size());
    CurrentDefVal = VarVal{TypeIdx, TypeDefaults[TypeIdx]};
  } else if (T->listType() != nullptr) {
    // TODO:
  } else {
    std::string Type = T->toString();
    if (Type == "bool") {
      CurrentDefVal = VarVal::createUninitialized(VarType{VarKind::Int});
    } else if (Type == "int") {
      CurrentDefVal = VarVal::createUninitialized(VarType{VarKind::Int});
    } else {
      llvm_unreachable("This code should be unreachable");
    }
  }
  assert(CurrentDefVal.Type.Kind != VarKind::Invalid);
  VarInfo Info{std::move(CurrentDefID), std::move(CurrentDefVal)};
  CurrentDefID = "";
  CurrentArchVals.insert(std::make_pair(Info.Id, Info.Val));
  VarStack.push_back(std::move(Info));
}

void DSLListener::enterListType(DSLGrammarParser::ListTypeContext *LT) {
  // TODO:
}
void DSLListener::exitListType(DSLGrammarParser::ListTypeContext *LT) {
  // TODO:
}

VarVal DSLListener::getDefaultValue(VarKind Kind, size_t ObjTypeIdx) const {
  switch (Kind) {
  case VarKind::Invalid:
    llvm_unreachable("");
  case VarKind::Bool:
  case VarKind::Int:
    return VarVal::createUninitialized(VarType{Kind});
  case VarKind::Obj:
    assert(ObjTypeIdx != VarType::InvalidTypeIdx);
    return VarVal{ObjTypeIdx, TypeDefaults[ObjTypeIdx]};
  case VarKind::List:
    llvm_unreachable("List of lists not implemented");
  case VarKind::Ref:
    return VarVal{ObjTypeIdx, ""};
  }

  assert(false);
}

void DSLListener::enterExpr(DSLGrammarParser::ExprContext *Expr) {
  VarVal &Curr = VarStack.back().Val;
  if (Curr.Type.Kind == VarKind::List) {
    VarStack.push_back(VarInfo{
        "", getDefaultValue(Curr.Type.ListKind, Curr.Type.ListObjTypeIdx)});
  }
}
void DSLListener::exitExpr(DSLGrammarParser::ExprContext *Expr) {
  VarInfo &CurrInfo = VarStack.back();
  VarVal &Curr = CurrInfo.Val;
  if (auto *Int = Expr->INT()) {
    if (Curr.Type.Kind != VarKind::Int) {
      throw std::runtime_error{CurrInfo.Id + " is not an int"};
    }
    Curr = VarVal{static_cast<int64_t>(stoi(Int->toString()))};
  } else if (auto *Bool = Expr->BOOL()) {
    if (Curr.Type.Kind != VarKind::Bool) {
      throw std::runtime_error{CurrInfo.Id + " is not a bool"};
    }

    std::string Spell = Bool->toString();
    bool B;
    if (Spell == "true") {
      B = true;
    } else {
      assert(Spell == "false");
      B = false;
    }
    Curr = VarVal{B};
  } else if (auto *Id = Expr->ID()) {
    std::string Spell = Id->toString();
    auto It = CurrentArchVals.find(Spell);
    if (It == CurrentArchVals.end()) {
      throw std::runtime_error{"Undefined reference to " + Spell};
    }

    const VarVal &GotVal = It->second;
    const VarType &GotType = GotVal.Type;

    if (Curr.Type.Kind == VarKind::Ref) {
      if (GotType.Kind != VarKind::Obj) {
        throw std::runtime_error{
            "Expected an object type but got " +
            std::to_string(static_cast<size_t>(GotType.Kind))};
      }
      if (GotType.ObjTypeIdx != Curr.Type.ObjTypeIdx) {
        throw std::runtime_error{
            "Expected Object Type " + std::to_string(Curr.Type.ObjTypeIdx) +
            " but got " + std::to_string(GotType.ObjTypeIdx)};
      }

      Curr = VarVal{GotType.ObjTypeIdx, std::move(Spell)};
    } else {
      if (GotType != Curr.Type) {
        throw std::runtime_error{
            "Expected type " +
            std::to_string(static_cast<size_t>(Curr.Type.Kind)) + " but got " +
            std::to_string(static_cast<size_t>(GotType.Kind)) +
            "THIS MAY ALSO HAVE DIFFERENT OBJECT TYPES"};
      }
      Curr = GotVal;
    }
  }
  
  // Empty name for list values
  if (VarStack.back().Id == "") {
    assert(VarStack.size() > 1);
    VarVal &List = VarStack[VarStack.size() - 2].Val;
    assert(List.Type.Kind == VarKind::List);
    List.LstVal.push_back(std::move(VarStack.back().Val));
    VarStack.pop_back();
  }
}

void DSLListener::enterObj(DSLGrammarParser::ObjContext *Obj) {
  // TODO: error if current variable is not an Object
}

void DSLListener::visitErrorNode(antlr4::tree::ErrorNode *Err) {
  // TODO:
}

Vec<DSLArchData> getDSLArchData(const char *Filename) {
  std::ifstream Stream{Filename};
  antlr4::ANTLRInputStream In{Stream};
  DSLGrammarLexer Lexer{&In};
  antlr4::CommonTokenStream Toks{&Lexer};
  DSLGrammarParser Parser{&Toks};
  antlr4::tree::ParseTree *Tree = Parser.translationUnit();
  DSLListener Listener;
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&Listener, Tree);
  return Listener.getArchData();
}

static Vec<std::pair<std::string, VarVal>>
extractObjsOfType(VarType Type, IdentifierMap<VarVal> &Ids) {
  Vec<std::pair<std::string, VarVal>> Res;
  for (auto It = Ids.begin(); It != Ids.end(); /*no incr*/) {
    if (It->second.Type == Type) {
      Res.push_back(
          std::make_pair(std::move(It->first), std::move(It->second)));
      It = Ids.erase(It);
    } else {
      ++It;
    }
  }
  return Res;
}

const MCProcResourceDesc* findProcResByName(const RISCVDynSubtargetData::Vec<MCProcResourceDesc>& Resources, const std::string& Name) {
  auto It = std::find_if(Resources.begin(), Resources.end(), [&Name](const MCProcResourceDesc& R) {
    return R.Name == Name;
  });
  return &Resources[It - Resources.begin()];
}

// Each time a WriteRes or ReadAdvance is processed, we need to either create a
// new MCSchedClass or find an equivalent one and then write the index in the
// InstrInfo Table
RISCVDynSubtargetData getDynSubtargetData(const char *Filename) {
  auto ArchData = getDSLArchData(Filename);
  // TODO: reserve all tables to a reasonable value (maybe just the size of the
  // tablegen tables)
  RISCVDynSubtargetData Res;
  Res.ArchNames.reserve(ArchData.size());
  Res.ProcResourceTables.reserve(ArchData.size());
  Res.SchedClassTables.reserve(ArchData.size());
  Res.ProcResourceNames.reserve(ArchData.size());
  Res.ProcResourceSubUnits.reserve(ArchData.size());

  for (auto &Data : ArchData) {
    Res.ArchNames.push_back(std::move(Data.Name));

    auto ProcResources = extractObjsOfType(VarType{VarKind::Obj, 1}, Data.Vals);
    // Generate earlier so we know how many MCProcResourceDescs are there
    auto ProcResGroups = extractObjsOfType(VarType{VarKind::Obj, 5}, Data.Vals);
    Res.ProcResourceTables.emplace_back();
    Res.ProcResourceNames.emplace_back();
    Res.ProcResourceSubUnits.emplace_back();

    auto &CurrProcResourceTable = Res.ProcResourceTables.back();
    auto &CurrProcResourceNames = Res.ProcResourceNames.back();
    auto &CurrProcResourceSubUnits = Res.ProcResourceSubUnits.back();

    // Reserve so push_back does not invalidate Small strings
    // -1 because NoProcResource does not count and string does not need to be
    // allocated for InvalidUnit
    CurrProcResourceNames.reserve(ProcResources.size() + ProcResGroups.size() -
                                  1);

    // Size including "InvalidUnit"
    CurrProcResourceTable.reserve(ProcResources.size() + ProcResGroups.size());
    CurrProcResourceTable.push_back(
        MCProcResourceDesc{"InvalidUnit", 0, 0, 0, 0});

    for (auto &[Name, Val] : ProcResources) {
      if (Name == "NoProcResource") {
        // TODO: Special handling (Only AssociatedWrites relevant)
        continue;
      }
      CurrProcResourceNames.emplace_back(std::move(Name));
      Name = "";
      assert(Val.Type.Kind == VarKind::Obj);
      assert(Val.Type.ObjTypeIdx == 1);
      auto &Map = Val.ObjVal;

      unsigned SuperIdx = 0;
      const std::string &RefKey = Map["Super"].RefKey;
      if (RefKey != "") {
        const MCProcResourceDesc* Super = findProcResByName(CurrProcResourceTable, RefKey);
        if (Super == &CurrProcResourceTable[CurrProcResourceTable.end() - CurrProcResourceTable.begin()]) {
          unsigned I = 0;
          while (ProcResources[I].first == "") {
            ++I;
          }
          
          if (I == CurrProcResourceTable.size()) {
            while (ProcResources[I].first != RefKey) {
              ++I;
            }
            SuperIdx = I;
          } else {
            assert(I == CurrProcResourceTable.size() - 1);
            bool GotNoProcResource = false;
            while (ProcResources[I].first != RefKey) {
              if (ProcResources[I].first == "NoProcResource") {
                GotNoProcResource = true;
              }
              ++I;
            }
            SuperIdx = GotNoProcResource ? I : I - 1;
          }
        } else {
          SuperIdx = Super - &CurrProcResourceTable[0];
        }
      }

      const VarVal &MapNumUnits = Map["NumUnits"];
      if (!MapNumUnits.Initialized) {
        // TODO: proper error
        throw std::runtime_error{"NumUnits not initialized"};
      }
      const unsigned NumUnits = MapNumUnits.IntVal;
      assert(static_cast<int64_t>(NumUnits) == MapNumUnits.IntVal);
      const VarVal &MapBufferSize = Map["BufferSize"];
      if (!MapBufferSize.Initialized) {
        // TODO: proper error
        throw std::runtime_error{"BufferSize not initialized"};
      }
      const int BufferSize = MapBufferSize.IntVal;
      assert(BufferSize == MapBufferSize.IntVal);

      CurrProcResourceTable.push_back(MCProcResourceDesc{
        CurrProcResourceNames.back().c_str(),
        NumUnits,
        SuperIdx,
        BufferSize,
        nullptr,
      });

      // TODO: AssociatedWrites
    }

    CurrProcResourceSubUnits.reserve(ProcResGroups.size());
    for (auto &[Name, Val] : ProcResGroups) {
      CurrProcResourceNames.push_back(std::move(Name));
      assert(Val.Type.Kind == VarKind::Obj);
      assert(Val.Type.ObjTypeIdx == 5);
      auto &Map = Val.ObjVal;

      // TODO: what happens with empty ProcResGroups
      const auto &Resources = Map["Resources"].LstVal;
      CurrProcResourceSubUnits.emplace_back();
      CurrProcResourceSubUnits.back().reserve(Resources.size());
      unsigned NumUnits = 0;
      for (const auto &Resource : Resources) {
        assert(Resource.Type.Kind == VarKind::Ref);
        const MCProcResourceDesc *It = findProcResByName(CurrProcResourceTable, Resource.RefKey);
        NumUnits += It->NumUnits;
        const unsigned Idx = It - &CurrProcResourceTable[0];
        CurrProcResourceSubUnits.back().push_back(Idx);
      }

      CurrProcResourceTable.push_back(MCProcResourceDesc{
          CurrProcResourceNames.back().c_str(),
          NumUnits,
          0,  // TODO: ProcResGroup of ProcResGroups?
          -1, // TODO: is bufferSize always -1?
          CurrProcResourceSubUnits.back().data(),
      });
      // TODO: AssociatedWrites
    }
  }
  return Res;
}

static MCSchedModel getDynSubtargetSchedModel(const RISCVDynSubtargetData &SD,
                                              size_t ArchIdx) {
  static constexpr unsigned int UINT_PLACEHOLDER = -1;
  static constexpr bool BOOL_PLACEHOLDER = false;
  const auto &ProcResourceTable = SD.ProcResourceTables[ArchIdx];
  const auto &SchedClassTable = SD.SchedClassTables[ArchIdx];
  const unsigned NumProcResourceKinds =
      static_cast<unsigned>(ProcResourceTable.size());
  const unsigned NumSchedClasses =
      static_cast<unsigned>(SchedClassTable.size());
  assert(NumProcResourceKinds == ProcResourceTable.size());
  assert(NumSchedClasses == SchedClassTable.size());

  MCSchedModel Res = {
      UINT_PLACEHOLDER, // IssueWidth
      UINT_PLACEHOLDER, // MicroOpBufferSize
      UINT_PLACEHOLDER, // LoopMicroOpBufferSize
      UINT_PLACEHOLDER, // LoadLatency
      UINT_PLACEHOLDER, // HighLatency
      UINT_PLACEHOLDER, // MispredictPenalty
      BOOL_PLACEHOLDER, // PostRAScheduler
      BOOL_PLACEHOLDER, // CompleteModel
      UINT_PLACEHOLDER, // Processor ID
      ProcResourceTable.data(),
      SchedClassTable.data(),
      NumProcResourceKinds,
      NumSchedClasses,
      nullptr,
      nullptr,
  };
  return Res;
}

RISCVDynSubtargetInfo::RISCVDynSubtargetInfo(const RISCVDynSubtargetData &SD,
                                             const Triple &TT, StringRef CPU,
                                             StringRef TuneCPU, StringRef FS)
    : TargetSubtargetInfo(
          TT, CPU, TuneCPU, FS,
          ArrayRef<SubtargetFeatureKV>(SD.FeatureKV.data(),
                                       SD.FeatureKV.size()),
          ArrayRef<SubtargetSubTypeKV>(SD.SubTypeKV.data(),
                                       SD.SubTypeKV.size()),
          SD.WriteProcResTable.data(), SD.WriteLatencyTable.data(),
          SD.ReadAdvanceTable.data(), nullptr, nullptr, nullptr) {}

} // namespace llvm
