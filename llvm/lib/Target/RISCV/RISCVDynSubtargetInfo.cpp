#include "RISCVDynSubtargetInfo.h"

#include "antlr4-common.h"
#include "antlr4-runtime.h"
#include "antlr4cpp_generated_src/DSLGrammar/DSLGrammarBaseListener.h"
#include "antlr4cpp_generated_src/DSLGrammar/DSLGrammarLexer.h"
#include "antlr4cpp_generated_src/DSLGrammar/DSLGrammarParser.h"

namespace llvm {

using namespace antlr_dsl;

// TODO: remove all unneeded overrides
class DSLListener final : public DSLGrammarBaseListener {
  RISCVDynSubtargetData &Res;
  enum class VarType {
    Invalid,
    Bool,
    Int,
    Obj,
    List,
    Ref,
  };
  struct ObjectVal;

  struct VarTypeData {
    VarType Type = VarType::Invalid;
    size_t ObjTypeIdx = VarVal::InvalidTypeIdx;
  };

  struct VarVal;

  struct ListVal {
    VarTypeData UnderlyingType;
    std::vector<VarVal> Vals;

    ListVal() : UnderlyingType{VarType::Invalid, 0}, Vals{} {}
    ListVal(VarTypeData Underlying, std::vector<VarVal> &&Vals)
        : UnderlyingType{Underlying}, Vals{std::move(Vals)} {}

    ListVal(const ListVal &Other)
        : UnderlyingType{Other.UnderlyingType}, Vals{Other.Vals} {}
    ListVal(ListVal &&Other)
        : UnderlyingType{Other.UnderlyingType}, Vals{std::move(Other.Vals)} {}

    ListVal &operator=(const ListVal &Other) {
      UnderlyingType = Other.UnderlyingType;
      Vals = Other.Vals;
      return *this;
    }

    ListVal &operator=(ListVal &&Other) {
      UnderlyingType = Other.UnderlyingType;
      Vals = std::move(Other.Vals);
      return *this;
    }
  };

  struct ObjectVal {
    using MemberMap = StringMap<VarVal>;
    MemberMap Members;

    ObjectVal(MemberMap &&Members) : Members{std::move(Members)} {}

    void writeMember(const std::string &Id, VarVal &&Val) {
      auto It = Members.find(Id);
      assert(It != Members.end());
      VarVal &ToWrite = It->second;
      if (ToWrite.Type.Type != Val.Type.Type) {
        // TODO error
        throw "error";
      } else if (ToWrite.Type.Type == VarType::Obj ||
                 ToWrite.Type.Type == VarType::Ref) {
        assert(Val.Type.Type == VarType::Obj || Val.Type.Type == VarType::Ref);
        if (ToWrite.Type.ObjTypeIdx != Val.Type.ObjTypeIdx) {
          // TODO: errror
          throw "error";
        }
      }
      ToWrite = std::move(Val);
    }
  };

  struct VarVal {
    VarTypeData Type;
    union {
      int64_t IntVal;
      bool BoolVal;
      ListVal LstVal;
      ObjectVal ObjVal;
      const VarVal *RefData;
    };
    bool Initialized;

    static constexpr size_t InvalidTypeIdx = static_cast<size_t>(-1);

    static VarVal createUninitialized(VarTypeData Type) {
      VarVal Res;
      Res.Type = Type;
      Res.Initialized = false;
      return Res;
    }

    VarVal() : Type{VarType::Invalid, InvalidTypeIdx}, Initialized{false} {}
    // TODO: normal int literal will be ambiguous, because it might be a bool
    VarVal(int64_t IntVal)
        : Type{VarType::Int, InvalidTypeIdx}, IntVal{IntVal},
          Initialized{true} {}
    VarVal(bool BoolVal)
        : Type{VarType::Bool, InvalidTypeIdx}, BoolVal{BoolVal},
          Initialized{true} {}
    VarVal(size_t TypeIdx, const ObjectVal &Val)
        : Type{VarType::Obj, TypeIdx}, ObjVal{Val}, Initialized{true} {
      assert(TypeIdx != InvalidTypeIdx);
    }
    VarVal(VarTypeData MemberType, std::vector<VarVal> &&Vals)
        : Type{VarType::List, InvalidTypeIdx},
          LstVal{MemberType, std::move(Vals)}, Initialized{true} {}
    VarVal(size_t RefTypeIdx, const VarVal *RefVal)
        : Type{VarType::Ref, RefTypeIdx}, RefData{RefVal} {
      assert(RefTypeIdx != InvalidTypeIdx);
      assert(RefVal == nullptr || RefTypeIdx == RefVal->Type.ObjTypeIdx);
    }
    VarVal(const VarVal &Other) : Type{Other.Type} {
      switch (Type.Type) {
      case VarType::Invalid:
        break;
      case VarType::Bool:
        BoolVal = Other.BoolVal;
        break;
      case VarType::Int:
        IntVal = Other.IntVal;
        break;
      case VarType::Obj:
        new (&ObjVal) ObjectVal(Other.ObjVal);
        break;
      case VarType::List:
        new (&LstVal) ListVal(Other.LstVal);
        break;
      case VarType::Ref:
        RefData = Other.RefData;
        break;
      }
      Initialized = Other.Initialized;
    }
    ~VarVal() {
      switch (Type.Type) {
      case VarType::Invalid:
      case VarType::Bool:
      case VarType::Int:
      case VarType::Ref:
        break;
      case VarType::Obj:
        ObjVal.~ObjectVal();
        break;
      case VarType::List:
        LstVal.~ListVal();
        break;
      }
    }

    VarVal &operator=(const VarVal &Other) {
      switch (Other.Type.Type) {
      case VarType::Invalid:
        break;
      case VarType::Bool:
        BoolVal = Other.BoolVal;
        break;
      case VarType::Int:
        IntVal = Other.IntVal;
        break;
      case VarType::Obj:
        ObjVal = Other.ObjVal;
        break;
      case VarType::List:
        LstVal = Other.LstVal;
        break;
      case VarType::Ref:
        RefData = Other.RefData;
        break;
      }
      Initialized = Other.Initialized;
      return *this;
    }

    // TODO: Make this work with self-assignment
    VarVal &operator=(VarVal &&Other) {
      if (Type.Type == VarType::Obj) {
        ObjVal.~ObjectVal();
      } else if (Type.Type == VarType::List) {
        LstVal.~ListVal();
      }
      switch (Other.Type.Type) {
      case VarType::Invalid:
        break;
      case VarType::Bool:
        BoolVal = Other.BoolVal;
        break;
      case VarType::Int:
        IntVal = Other.IntVal;
        break;
      case VarType::Obj:
        new (&ObjVal) ObjectVal(std::move(Other.ObjVal));
        break;
      case VarType::List:
        new (&LstVal) ListVal(std::move(Other.LstVal));
        break;
      case VarType::Ref:
        RefData = Other.RefData;
        break;
      }
      Initialized = Other.Initialized;
      return *this;
    }
  };

  // TOOD: this might not be necessary anymore
  struct VarInfo {
    std::string Id;
    VarVal Val;
  };

  inline static const ObjectVal DefaultWriteRes = {{
      {"ResourceCycles",
       VarVal(VarTypeData{VarType::Int, VarVal::InvalidTypeIdx},
              std::vector<VarVal>{})},
      {"Latency", VarVal(static_cast<int64_t>(1))},
      {"NumMicroOps", VarVal(static_cast<int64_t>(1))},
  }};
  inline static const ObjectVal DefaultAssociatedWrite = {{
      {"WriteRes", VarVal(2, DefaultWriteRes)},
      {"Writes", VarVal(VarTypeData{VarType::Obj, 4}, std::vector<VarVal>{})},
  }};
  inline static const ObjectVal DefaultReadAdvance = {{
      {"Cycles", VarVal(VarVal::createUninitialized(
                     {VarType::Int, VarVal::InvalidTypeIdx}))}, // Uninitialized
      {"ValidWrites", VarVal(VarTypeData{VarType::Ref, 4},
                             std::vector<VarVal>{})}, // list<SchedWrite>
  }};

  inline static const ObjectVal DefaultProcResGroup = {{
      {"Resources", VarVal{VarTypeData{VarType::Ref, 1}, std::vector<VarVal>{}}},
  }};
  // TODO: ProcResGroup
  inline static const StringMap<size_t> TypeMap = {
      {"AssociatedWrite", 0}, {"ProcResource", 1}, {"WriteRes", 2},
      {"ReadAdvance", 3},     {"SchedWrite", 4}, {"ProcResGroup", 5},
  };
  inline static const SmallVector<ObjectVal> TypeMembers = {
      DefaultAssociatedWrite,

      ObjectVal{{{"NumUnits", VarVal(static_cast<int64_t>(1))},
                 {"Super", VarVal(1, nullptr)},
                 {"AssociatedWrites", VarVal(0, DefaultAssociatedWrite)}}},

      DefaultWriteRes,

      DefaultReadAdvance,

      ObjectVal{{{"ReadAdvance", VarVal(3, DefaultReadAdvance)}}},
      DefaultProcResGroup,
  };
  inline static const StringMap<VarVal> DefaultArchVals = {
      // Values from MCSchedModel
      {"IssueWidth", VarVal{static_cast<int64_t>(1)}},
      {"LoadLatency", VarVal{static_cast<int64_t>(4)}},
      {"HighLatency", VarVal{static_cast<int64_t>(10)}},
      {"MispredictPenalty", VarVal{static_cast<int64_t>(10)}},
      {"CompleteModel", VarVal{false}},
      {"UnsupportedFeatures", VarVal{/* TODO*/}},
  };

  StringMap<VarVal> CurrentArchVals;
  std::vector<VarInfo> VarStack;
  // Stores the size of VarStack before this overwrite
  std::vector<size_t> OverwriteStack;
  VarVal CurrentExpr = VarVal{};
  std::string CurrentDefID = "";
  int64_t CurrentArchIdx = -1;

public:
  DSLListener(RISCVDynSubtargetData &Res) : Res{Res} {}

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

  VarTypeData getCurrVarType() const;
  const std::string *getCurrVarSpell() const;
};

void DSLListener::enterTranslationUnit(
    DSLGrammarParser::TranslationUnitContext *TL) {
  // TODO:
}
void DSLListener::exitTranslationUnit(
    DSLGrammarParser::TranslationUnitContext *TL) {
  // TODO:
}

static int getArchIndex(const std::string &Name,
                        const RISCVDynSubtargetData &Data) {
  for (int I = 0; static_cast<size_t>(I) < Data.ArchNames.size(); ++I) {
    if (Data.ArchNames[I] == Name) {
      return I;
    }
  }
  return -1;
}

void DSLListener::enterArchitectureDefinition(
    DSLGrammarParser::ArchitectureDefinitionContext *AD) {
  const auto &Ids = AD->ID();
  assert(Ids.size() == 1 || Ids.size() == 2);

  std::string ArchName = Ids[0]->toString();
  if (Ids.size() == 2) {
    std::string InheritName = Ids[1]->toString();
    const int64_t ArchIndex = getArchIndex(InheritName, Res);
    if (ArchIndex == -1) {
      // TODO: correct error
      throw "could not find arch def";
    }
    // Copy tables of inherited arch
    Res.ProcResourceTables.push_back(Res.ProcResourceTables[ArchIndex]);
    Res.SchedClassTables.push_back(Res.SchedClassTables[ArchIndex]);
  } else {
    Res.ProcResourceTables.emplace_back();
    Res.SchedClassTables.emplace_back();
  }
  Res.ArchNames.push_back(std::move(ArchName));
  // TODO: Just copying the default vals may be bad as just deleting all keys
  // not in the default vals and resetting them may preserve allocation size, as
  // the copy will probably just free the old map
  CurrentArchVals = DefaultArchVals;
  ++CurrentArchIdx;
}

void DSLListener::exitArchitectureDefinition(
    DSLGrammarParser::ArchitectureDefinitionContext *AD) {
  // TODO: Write Values read from file

  std::cout << "ValStack.size(): " << VarStack.size() << '\n';
}

void DSLListener::enterDefinition(DSLGrammarParser::DefinitionContext *Def) {
  // TODO:
  CurrentDefID = Def->ID()->toString();
}
void DSLListener::exitDefinition(DSLGrammarParser::DefinitionContext *Def) {
  // TODO:
  VarStack.pop_back();
}

const DSLListener::VarVal &
DSLListener::getVarVal(const std::string &Spell) const {
  if (VarStack.empty()) {
    auto Found = CurrentArchVals.find(Spell);
    if (Found == CurrentArchVals.end()) {
      // TODO: correcct error
      throw "not found";
    }
    return Found->getValue();
  }
  return VarStack[VarStack.size() - 1].Val;
}

DSLListener::VarTypeData DSLListener::getCurrVarType() const {
  if (VarStack.size() != 0) {
    return VarStack[VarStack.size() - 1].Val.Type;
  }
  return {VarType::Invalid, 0};
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
  VarTypeData Type = getCurrVarType();
  // assert(Type.Type != VarType::Invalid);
  // TODO: Check assignment types and write CurrentVal
  size_t NewSize = OverwriteStack.back();
  OverwriteStack.pop_back();
  while (VarStack.size() > NewSize) {
    // TODO: write value here
    VarStack.pop_back();
  }
}

void DSLListener::enterMemberAccess(DSLGrammarParser::MemberAccessContext *MA) {
  for (auto *Id : MA->ID()) {
    std::string Spell = Id->toString();
    const VarVal &Type = getVarVal(Spell);
    VarVal Val;
    Val.Type = Type.Type;
    VarStack.push_back(VarInfo{Spell, std::move(Val)});
  }
}
void DSLListener::exitMemberAccess(DSLGrammarParser::MemberAccessContext *MA) {
  // TODO: this should probably be moved to exitOverwrite
  /*
  const auto &Ids = MA->ID();
  for (auto It = Ids.rbegin(); It != Ids.rend(); ++It) {
    assert((*It)->toString() == *getCurrVarSpell());
    // TODO: might need to write the written info here
    VarStack.pop_back();
  }
  */
}

// This function assumes that this is always called after enterDefinition has set CurrentDefID
void DSLListener::enterType(DSLGrammarParser::TypeContext *T) {
  assert(CurrentDefID != "");
  // TODO:
  VarVal CurrentDefVal;
  if (T->ID() != nullptr) {
    auto It = TypeMap.find(T->ID()->toString());
    if (It == TypeMap.end()) {
        // TODO: proper error
        throw "unknown type";
    }
    size_t TypeIdx = It->getValue();
    CurrentDefVal = VarVal{TypeIdx, TypeMembers[TypeIdx]};
  } else if (T->listType() != nullptr) {
    // TODO:
  } else {
    std::string Type = T->toString();
    if (Type == "bool") {
      CurrentDefVal = VarVal::createUninitialized(VarTypeData{VarType::Int, VarVal::InvalidTypeIdx});
    } else if (Type == "int") {
      CurrentDefVal = VarVal::createUninitialized(VarTypeData{VarType::Int, VarVal::InvalidTypeIdx});
    } else {
      llvm_unreachable("This code should be unreachable");
    }
  }
  VarInfo Info{std::move(CurrentDefID), std::move(CurrentDefVal)};
  CurrentDefID = "";
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

void DSLListener::enterExpr(DSLGrammarParser::ExprContext *Expr) {
  // TODO:
  std::cout << "INT: " << Expr->INT() << ": " << (Expr->INT() != nullptr ? Expr->INT()->toString() : "") << "\n";
  std::cout << "BOOL: " << Expr->BOOL() << ": " << (Expr->BOOL() != nullptr ? Expr->BOOL()->toString() : "") << "\n";
  std::cout << "ID: " << Expr->ID() << ": " << (Expr->ID() != nullptr ? Expr->ID()->toString() : "") <<"\n";
  std::cout << "list: " << Expr->list() << ": " << (Expr->list() != nullptr ? Expr->list()->toString() : "") << "\n";
  std::cout << "obj: " << Expr->obj() << ": " << (Expr->obj() != nullptr ? Expr->obj()->toString() : "") << "\n";
}
void DSLListener::exitExpr(DSLGrammarParser::ExprContext *Expr) {
  // TODO:
}

void DSLListener::enterList(DSLGrammarParser::ListContext *List) {
  // TODO:
}
void DSLListener::exitList(DSLGrammarParser::ListContext *List) {
  // TODO:
}

void DSLListener::enterObj(DSLGrammarParser::ObjContext *Obj) {
  // TODO:
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

RISCVDynSubtargetData getDynSubtargetData(const char *Filename) {
  std::ifstream Stream{Filename};
  antlr4::ANTLRInputStream In{Stream};
  DSLGrammarLexer Lexer{&In};
  antlr4::CommonTokenStream Toks{&Lexer};
  DSLGrammarParser Parser{&Toks};
  antlr4::tree::ParseTree *Tree = Parser.translationUnit();
  RISCVDynSubtargetData Res;
  DSLListener Listener{Res};
  try {
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&Listener, Tree);
  } catch (const char* chr) {
    std::cout << "Exc:" << chr << "\n";
  }
  return Res;
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
