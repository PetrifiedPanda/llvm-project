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
  inline static const ObjectVal DefaultWriteRes = {{
      {"ResourceCycles",
       VarVal{VarKind::Int, VarVal::InvalidTypeIdx, std::vector<VarVal>{}}},
      {"Latency", VarVal(static_cast<int64_t>(1))},
      {"NumMicroOps", VarVal(static_cast<int64_t>(1))},
  }};
  inline static const ObjectVal DefaultAssociatedWrite = {{
      {"WriteRes", VarVal(2, DefaultWriteRes)},
      {"Writes", VarVal{VarKind::Obj, 4, std::vector<VarVal>{}}},
  }};
  inline static const ObjectVal DefaultProcResource = {{
      {"NumUnits", VarVal{static_cast<int64_t>(1)}},
      {"Super", VarVal{1, ""}},
      {"BufferSize", VarVal{static_cast<int64_t>(-1)}},
      {"AssociatedWrites", VarVal{VarKind::Obj, 0, std::vector<VarVal>{}}},
  }};
  inline static const ObjectVal DefaultReadAdvance = {{
      {"Cycles", VarVal(VarVal::createUninitialized(
                     VarType{VarKind::Int}))}, // Uninitialized
      {"ValidWrites",
       VarVal{VarKind::Ref, 4, std::vector<VarVal>{}}}, // list<SchedWrite>
  }};

  inline static const ObjectVal DefaultProcResGroup = {{
      {"Resources", VarVal{VarKind::Ref, 1, std::vector<VarVal>{}}},
      {"AssociatedWrites", VarVal{VarKind::Obj, 0, std::vector<VarVal>{}}},
  }};

  inline static const ObjectVal DefaultSchedWrite = {{}};

  inline static const ObjectVal DefaultSchedRead = {{
      {"ReadAdvance", VarVal(3, DefaultReadAdvance)},
  }};

  inline static const ObjectVal DefaultRISCVFeature = {{}};
  // TODO: ProcResGroup
  inline static const StringMap<size_t> TypeMap = {
      {"AssociatedWrite", 0}, {"ProcResource", 1}, {"WriteRes", 2},
      {"ReadAdvance", 3},     {"SchedWrite", 4},   {"ProcResGroup", 5},
      {"SchedRead", 6},       {"RISCVFeature", 7},
  };

  inline static const Vec<ObjectVal> TypeDefaults = {
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
      {"UnsupportedFeatures", VarVal{VarKind::Obj, 7, std::vector<VarVal>{}}},
      {"NoProcResource",
       VarVal{1, DefaultProcResource}}, // TODO: not sure if this should be
                                        // DefaultProcResource
      // SchedReads TODO: check if all reads from RISCVSchedule.td are here
      {"ReadJmp", VarVal{6, DefaultSchedRead}},
      {"ReadJalr", VarVal{6, DefaultSchedRead}},
      {"ReadCSR", VarVal{6, DefaultSchedRead}},
      {"ReadStoreData", VarVal{6, DefaultSchedRead}},
      {"ReadMemBase", VarVal{6, DefaultSchedRead}},
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
      {"ReadFStoreData", VarVal{6, DefaultSchedRead}},
      {"ReadFMemBase", VarVal{6, DefaultSchedRead}},
      {"ReadFAdd32", VarVal{6, DefaultSchedRead}},
      {"ReadFAdd64", VarVal{6, DefaultSchedRead}},
      {"ReadFMul32", VarVal{6, DefaultSchedRead}},
      {"ReadFMul64", VarVal{6, DefaultSchedRead}},
      {"ReadFMA32", VarVal{6, DefaultSchedRead}},
      {"ReadFMA64", VarVal{6, DefaultSchedRead}},
      {"ReadFDiv32", VarVal{6, DefaultSchedRead}},
      {"ReadFDiv64", VarVal{6, DefaultSchedRead}},
      {"ReadFSqrt32", VarVal{6, DefaultSchedRead}},
      {"ReadFSqrt64", VarVal{6, DefaultSchedRead}},
      {"ReadFCmp32", VarVal{6, DefaultSchedRead}},
      {"ReadFCmp64", VarVal{6, DefaultSchedRead}},
      {"ReadFSGNJ32", VarVal{6, DefaultSchedRead}},
      {"ReadFSGNJ64", VarVal{6, DefaultSchedRead}},
      {"ReadFMinMax32", VarVal{6, DefaultSchedRead}},
      {"ReadFMinMax64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF32ToI32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF32ToI64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF64ToI32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF64ToI64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI32ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI32ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI64ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtI64ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF32ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFCvtF64ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFMovF32ToI32", VarVal{6, DefaultSchedRead}},
      {"ReadFMovI32ToF32", VarVal{6, DefaultSchedRead}},
      {"ReadFMovF64ToI64", VarVal{6, DefaultSchedRead}},
      {"ReadFMovI64ToF64", VarVal{6, DefaultSchedRead}},
      {"ReadFClass32", VarVal{6, DefaultSchedRead}},
      {"ReadFClass64", VarVal{6, DefaultSchedRead}},
      {"ReadSFB", VarVal{6, DefaultSchedRead}},

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
  // Stores the size of VarStack before this overwrite
  Vec<size_t> OverwriteStack;
  std::string CurrentDefID = "";
  int64_t CurrentArchIdx = -1;

public:
  Vec<DSLArchData> getArchData() {
    Vec<DSLArchData> Res;
    const size_t Size = CurrentArchIdx + 1;
    Res.reserve(Size);
    for (size_t I = 0; I < Size; ++I) {
      Res.push_back(
          DSLArchData{std::move(ArchNames[I]), std::move(PrevArchVals[I])});
    }
    return Res;
  }

private:
  void
  enterTranslationUnit(DSLGrammarParser::TranslationUnitContext *TL) override;
  void
  exitTranslationUnit(DSLGrammarParser::TranslationUnitContext *TL) override;

  void enterArchitectureDefinition(
      DSLGrammarParser::ArchitectureDefinitionContext *AD) override;
  void exitArchitectureDefinition(
      DSLGrammarParser::ArchitectureDefinitionContext *AD) override;

  void enterDefinition(DSLGrammarParser::DefinitionContext *Def) override;
  void exitDefinition(DSLGrammarParser::DefinitionContext *Def) override;

  void enterOverwrite(DSLGrammarParser::OverwriteContext *OW) override;
  void exitOverwrite(DSLGrammarParser::OverwriteContext *OW) override;

  void enterMemberAccess(DSLGrammarParser::MemberAccessContext *MA) override;
  void exitMemberAccess(DSLGrammarParser::MemberAccessContext *MA) override;

  void enterType(DSLGrammarParser::TypeContext *T) override;
  void exitType(DSLGrammarParser::TypeContext *T) override;

  void enterListType(DSLGrammarParser::ListTypeContext *LT) override;
  void exitListType(DSLGrammarParser::ListTypeContext *LT) override;

  void enterExpr(DSLGrammarParser::ExprContext *Expr) override;
  void exitExpr(DSLGrammarParser::ExprContext *Expr) override;

  void enterList(DSLGrammarParser::ListContext *List) override;
  void exitList(DSLGrammarParser::ListContext *List) override;

  void enterObj(DSLGrammarParser::ObjContext *Obj) override;
  void exitObj(DSLGrammarParser::ObjContext *Obj) override;

  void enterEveryRule(antlr4::ParserRuleContext *Rule) override;
  void exitEveryRule(antlr4::ParserRuleContext *Rule) override;

  void visitTerminal(antlr4::tree::TerminalNode *Term) override;
  void visitErrorNode(antlr4::tree::ErrorNode *Err) override;

  const VarVal &getVarVal(const std::string &Spell) const;

  int64_t getArchIndex(const std::string &Name) const;

  VarType getCurrVarType() const;
  const std::string *getCurrVarSpell() const;
  VarVal getDefaultValue(VarKind Kind, size_t ObjTypeIdx) const;
};

void DSLListener::enterTranslationUnit(
    DSLGrammarParser::TranslationUnitContext *TL) {
  // TODO:
}
void DSLListener::exitTranslationUnit(
    DSLGrammarParser::TranslationUnitContext *TL) {
  // TODO:
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
  // TODO:
  CurrentDefID = Def->ID()->toString();
}
void DSLListener::exitDefinition(DSLGrammarParser::DefinitionContext *Def) {
  // TODO:
  std::cout << "Popping " << VarStack[VarStack.size() - 1].Id << '\n';
  VarStack.pop_back();
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
  const auto &ToSearch = Curr.ObjVal.Members;
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
  return {VarKind::Invalid, VarVal::InvalidTypeIdx};
}

const std::string *DSLListener::getCurrVarSpell() const {
  if (VarStack.size() == 0) {
    return nullptr;
  }
  return &VarStack[VarStack.size() - 1].Id;
}

void DSLListener::enterOverwrite(DSLGrammarParser::OverwriteContext *OW) {
  OverwriteStack.push_back(VarStack.size());
  // TODO:
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
    std::cout << "Popping " << Curr.Id
              << " with Kind: " << static_cast<size_t>(Curr.Val.Type.Kind)
              << '\n';
    if (VarStack.empty()) {
      auto It = CurrentArchVals.find(Curr.Id);
      assert(It != CurrentArchVals.end());
      It->second = std::move(Curr.Val);
    } else {
      VarInfo &Prev = VarStack.back();
      assert(Prev.Val.Type.Kind == VarKind::Obj);
      Prev.Val.ObjVal.writeMember(Curr.Id, std::move(Curr.Val));
    }
  }
}

void DSLListener::enterMemberAccess(DSLGrammarParser::MemberAccessContext *MA) {
  for (auto *Id : MA->ID()) {
    std::string Spell = Id->toString();
    const VarVal &Val = getVarVal(Spell);
    assert(Val.Type.Kind != VarKind::Invalid);
    std::cout << "Pushing " << Spell << " with Kind "
              << static_cast<size_t>(Val.Type.Kind) << '\n';
    VarStack.push_back(VarInfo{Spell, Val});
  }
}
void DSLListener::exitMemberAccess(DSLGrammarParser::MemberAccessContext *MA) {}

// This function assumes that this is always called after enterDefinition has
// set CurrentDefID
void DSLListener::enterType(DSLGrammarParser::TypeContext *T) {
  assert(CurrentDefID != "");
  // TODO:
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
      CurrentDefVal = VarVal::createUninitialized(
          VarType{VarKind::Int, VarVal::InvalidTypeIdx});
    } else if (Type == "int") {
      CurrentDefVal = VarVal::createUninitialized(
          VarType{VarKind::Int, VarVal::InvalidTypeIdx});
    } else {
      llvm_unreachable("This code should be unreachable");
    }
  }
  assert(CurrentDefVal.Type.Kind != VarKind::Invalid);
  VarInfo Info{std::move(CurrentDefID), std::move(CurrentDefVal)};
  CurrentDefID = "";
  CurrentArchVals.insert(std::make_pair(Info.Id, Info.Val));
  std::cout << "Pushing " << Info.Id << " with kind "
            << static_cast<size_t>(CurrentArchVals[Info.Id].Type.Kind) << '\n';
  VarStack.push_back(std::move(Info));
}
void DSLListener::exitType(DSLGrammarParser::TypeContext *T) {
  // TODO:
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
    assert(ObjTypeIdx != VarVal::InvalidTypeIdx);
    return VarVal{ObjTypeIdx, TypeDefaults[ObjTypeIdx]};
  case VarKind::List:
    llvm_unreachable("List of lists not implemented");
  case VarKind::Ref:
    return VarVal{ObjTypeIdx, ""};
  }

  assert(false);
}

void DSLListener::enterExpr(DSLGrammarParser::ExprContext *Expr) {
  // TODO:
  VarVal &Curr = VarStack.back().Val;
  if (Curr.Type.Kind == VarKind::List) {
    VarStack.push_back(VarInfo{
        "", getDefaultValue(Curr.Type.ListKind, Curr.Type.ListObjTypeIdx)});
  }
}
void DSLListener::exitExpr(DSLGrammarParser::ExprContext *Expr) {
  // TODO:
  // TODO: Maybe for every object that is in a list create an Object with empty
  // ID? Every empty ID would be assumed to be part of a list (Assert that the
  // Object on the stack before it is a list)
  VarInfo &CurrInfo = VarStack.back();
  VarVal &Curr = CurrInfo.Val;
  if (auto *Int = Expr->INT()) {
    if (Curr.Type.Kind != VarKind::Int) {
      throw std::runtime_error{CurrInfo.Id + " is not an int"};
    }
    Curr = VarVal{static_cast<int64_t>(stoi(Int->toString()))};
    std::cout << "INT: " << Int->toString() << '\n';
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
    std::cout << "BOOL: " << Bool->toString() << '\n';
  } else if (auto *Id = Expr->ID()) {
    std::string Spell = Id->toString();
    auto It = CurrentArchVals.find(Spell);
    if (It == CurrentArchVals.end()) {
      throw std::runtime_error{"Undefined reference to " + Spell};
    }
    
    const VarVal& GotVal = It->second;
    const VarType &GotType = GotVal.Type;

    if (Curr.Type.Kind == VarKind::Ref) {
      if (GotType.Kind != VarKind::Obj) {
        throw std::runtime_error{
            "Expected an object type but got " +
            std::to_string(static_cast<size_t>(GotType.Kind))};
      }
      if (GotType.ObjTypeIdx != Curr.Type.ObjTypeIdx) {
        throw std::runtime_error{"Expected Object Type " +
                                 std::to_string(Curr.Type.ObjTypeIdx) +
                                 " but got " + std::to_string(GotType.ObjTypeIdx)};
      }

      Curr = VarVal{GotType.ObjTypeIdx, std::move(Spell)};
    } else {
      if (GotType != Curr.Type) {
        throw std::runtime_error{"Expected type " + std::to_string(static_cast<size_t>(Curr.Type.Kind)) + " but got " + std::to_string(static_cast<size_t>(GotType.Kind)) + "THIS MAY ALSO HAVE DIFFERENT OBJECT TYPES"};
      }
      Curr = GotVal;
    }
    std::cout << "ID: " << Id->toString() << '\n';
  } else if (auto *Lst = Expr->list()) {
    std::cout << "List: " << Lst->toString() << '\n';
  } else if (auto *Obj = Expr->obj()) {
    std::cout << "Object: " << Obj->toString() << '\n';
  }

  if (VarStack.back().Id == "") {
    assert(VarStack.size() > 1);
    VarVal &List = VarStack[VarStack.size() - 2].Val;
    assert(List.Type.Kind == VarKind::List);
    List.LstVal.Vals.push_back(std::move(VarStack.back().Val));
    VarStack.pop_back();
  }
}

void DSLListener::enterList(DSLGrammarParser::ListContext *List) {
  // TODO:
}
void DSLListener::exitList(DSLGrammarParser::ListContext *List) {
  // TODO:
}

void DSLListener::enterObj(DSLGrammarParser::ObjContext *Obj) {
  // TODO: error if current variable is not an Object
}
void DSLListener::exitObj(DSLGrammarParser::ObjContext *Obj) {
  // TODO:
}

void DSLListener::enterEveryRule(antlr4::ParserRuleContext *Rule) {
  // TODO:
}
void DSLListener::exitEveryRule(antlr4::ParserRuleContext *Rule) {
  // TODO:
}

void DSLListener::visitTerminal(antlr4::tree::TerminalNode *Term) {
  // TODO:
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

RISCVDynSubtargetData getDynSubtargetData(const char *Filename) {
  std::ifstream Stream{Filename};
  antlr4::ANTLRInputStream In{Stream};
  DSLGrammarLexer Lexer{&In};
  antlr4::CommonTokenStream Toks{&Lexer};
  DSLGrammarParser Parser{&Toks};
  antlr4::tree::ParseTree *Tree = Parser.translationUnit();
  DSLListener Listener;
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&Listener, Tree);
  // TODO: convert data from Listener to RISCVDynSubtargetData
  return RISCVDynSubtargetData{};
}

static MCSchedModel getDynSubtargetSchedModel(const RISCVDynSubtargetData &SD,
                                              size_t ArchIdx) {
  static constexpr unsigned int UINT_PLACEHOLDER = -1;
  static constexpr bool BOOL_PLACEHOLDER = false;

  const SmallVector<MCProcResourceDesc> &ProcResourceTable =
      SD.ProcResourceTables[ArchIdx];
  const SmallVector<MCSchedClassDesc> &SchedClassTable =
      SD.SchedClassTables[ArchIdx];
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
