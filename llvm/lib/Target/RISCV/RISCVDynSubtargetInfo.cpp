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
  template <typename T> using Vec = std::vector<T>;

  RISCVDynSubtargetData &Res;
  enum class VarKind {
    Invalid,
    Bool,
    Int,
    Obj,
    List,
    Ref,
  };
  struct ObjectVal;

  struct VarType {
    VarKind Kind = VarKind::Invalid;
    size_t ObjTypeIdx = VarVal::InvalidTypeIdx;
    VarKind ListKind = VarKind::Invalid;
    size_t ListObjTypeIdx = VarVal::InvalidTypeIdx;

    VarType() {}
    explicit VarType(VarKind Kind) : Kind{Kind} {
      assert(Kind != VarKind::Obj);
      assert(Kind != VarKind::Invalid);
      assert(Kind != VarKind::List);
      assert(Kind != VarKind::Ref);
    }
    VarType(VarKind Kind, size_t ObjTypeIdx) : Kind{Kind}, ObjTypeIdx{ObjTypeIdx} {
      assert(Kind == VarKind::Obj || Kind == VarKind::Ref);
    }
    VarType(VarKind Kind, size_t ObjTypeIdx, VarKind ListKind, size_t ListObjTypeIdx) : Kind{Kind}, ObjTypeIdx{ObjTypeIdx}, ListKind{ListKind}, ListObjTypeIdx{ListObjTypeIdx} {}

    bool operator==(const VarType& Other) const {
      if (Kind != Other.Kind) {
        return false;
      }

      if (Kind == VarKind::Obj || Kind == VarKind::Ref) {
        return ObjTypeIdx == Other.ObjTypeIdx;
      }

      if (Kind == VarKind::List) {
        if (ListKind != Other.ListKind) {
          return false;
        }

        if (ListKind == VarKind::Obj || ListKind == VarKind::Ref) {
            return ListObjTypeIdx == Other.ListObjTypeIdx;
        }
      }
      return true;
    }
  };

  struct VarVal;

  struct ListVal {
    std::vector<VarVal> Vals;
  };

  struct ObjectVal {
    using MemberMap = std::map<std::string, VarVal>;
    MemberMap Members;

    ObjectVal(MemberMap &&Members) : Members{std::move(Members)} {}

    void writeMember(const std::string &Id, VarVal &&Val) {
      auto It = Members.find(Id);
      assert(It != Members.end());
      VarVal &ToWrite = It->second;
      if (ToWrite.Type.Kind != Val.Type.Kind) {
        std::cout << "Type Mismatch: " << static_cast<size_t>(ToWrite.Type.Kind) << " vs " << static_cast<size_t>(Val.Type.Kind) << "\n";
        // TODO error
        throw std::runtime_error{"error"};
      } else if (ToWrite.Type.Kind == VarKind::Obj ||
                 ToWrite.Type.Kind == VarKind::Ref) {
        assert(Val.Type.Kind == VarKind::Obj || Val.Type.Kind == VarKind::Ref);
        if (ToWrite.Type.ObjTypeIdx != Val.Type.ObjTypeIdx) {
          // TODO: errror
          throw std::runtime_error{"error"};
        }
      }
      ToWrite = std::move(Val);
    }
  };

  struct VarVal {
    VarType Type;
    union {
      int64_t IntVal;
      bool BoolVal;
      ListVal LstVal;
      ObjectVal ObjVal;
      // TODO: If the Map is resized the pointers will be invalidated
      const VarVal *RefData;
    };
    bool Initialized;

    static constexpr size_t InvalidTypeIdx = static_cast<size_t>(-1);

    static VarVal createUninitialized(VarType Type) {
      VarVal Res;
      memset(&Res, 0, sizeof Res);
      Res.Type = Type;
      Res.Initialized = false;
      return Res;
    }

    VarVal() : Type{}, Initialized{false} {}
    // TODO: normal int literal will be ambiguous, because it might be a bool
    VarVal(int64_t IntVal)
        : Type{VarKind::Int}, IntVal{IntVal},
          Initialized{true} {}
    VarVal(bool BoolVal)
        : Type{VarKind::Bool}, BoolVal{BoolVal},
          Initialized{true} {}
    VarVal(size_t TypeIdx, const ObjectVal &Val)
        : Type{VarKind::Obj, TypeIdx}, ObjVal{Val}, Initialized{true} {
      assert(TypeIdx != InvalidTypeIdx);
    }
    VarVal(VarKind Kind, size_t ObjTypeIdx, std::vector<VarVal> &&Vals) : Type{VarKind::List, InvalidTypeIdx, Kind, ObjTypeIdx}, LstVal{std::move(Vals)} {}
    VarVal(size_t RefTypeIdx, const VarVal *RefVal)
        : Type{VarKind::Ref, RefTypeIdx}, RefData{RefVal} {
      assert(RefTypeIdx != InvalidTypeIdx);
      assert(RefVal == nullptr || RefTypeIdx == RefVal->Type.ObjTypeIdx);
    }
    VarVal(const VarVal &Other) : Type{Other.Type} {
      switch (Type.Kind) {
      case VarKind::Invalid:
        break;
      case VarKind::Bool:
        BoolVal = Other.BoolVal;
        break;
      case VarKind::Int:
        IntVal = Other.IntVal;
        break;
      case VarKind::Obj:
        new (&ObjVal) ObjectVal(Other.ObjVal);
        break;
      case VarKind::List:
        new (&LstVal) ListVal(Other.LstVal);
        break;
      case VarKind::Ref:
        RefData = Other.RefData;
        break;
      }
      Initialized = Other.Initialized;
    }
    ~VarVal() {
      switch (Type.Kind) {
      case VarKind::Invalid:
      case VarKind::Bool:
      case VarKind::Int:
      case VarKind::Ref:
        break;
      case VarKind::Obj:
        ObjVal.~ObjectVal();
        break;
      case VarKind::List:
        LstVal.~ListVal();
        break;
      }
    }

    // TODO: assigning to garbage memory error
    VarVal &operator=(const VarVal &Other) {
      if (Type.Kind == VarKind::Obj) {
        ObjVal.~ObjectVal();
      } else if (Type.Kind == VarKind::List) {
        LstVal.~ListVal();
      }
      switch (Other.Type.Kind) {
      case VarKind::Invalid:
        break;
      case VarKind::Bool:
        BoolVal = Other.BoolVal;
        break;
      case VarKind::Int:
        IntVal = Other.IntVal;
        break;
      case VarKind::Obj:
        new (&ObjVal) ObjectVal(Other.ObjVal);
        break;
      case VarKind::List:
        new (&LstVal) ListVal(Other.LstVal);
        break;
      case VarKind::Ref:
        RefData = Other.RefData;
        break;
      }
      Initialized = Other.Initialized;
      Type = Other.Type;
      return *this;
    }

    // TODO: Make this work with self-assignment
    VarVal &operator=(VarVal &&Other) {
      if (Type.Kind == VarKind::Obj) {
        ObjVal.~ObjectVal();
      } else if (Type.Kind == VarKind::List) {
        LstVal.~ListVal();
      }
      switch (Other.Type.Kind) {
      case VarKind::Invalid:
        break;
      case VarKind::Bool:
        BoolVal = Other.BoolVal;
        break;
      case VarKind::Int:
        IntVal = Other.IntVal;
        break;
      case VarKind::Obj:
        new (&ObjVal) ObjectVal(std::move(Other.ObjVal));
        break;
      case VarKind::List:
        new (&LstVal) ListVal(std::move(Other.LstVal));
        break;
      case VarKind::Ref:
        RefData = Other.RefData;
        break;
      }
      Initialized = Other.Initialized;
      Type = Other.Type;
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
       VarVal{VarKind::Int, VarVal::InvalidTypeIdx,
              std::vector<VarVal>{}}},
      {"Latency", VarVal(static_cast<int64_t>(1))},
      {"NumMicroOps", VarVal(static_cast<int64_t>(1))},
  }};
  inline static const ObjectVal DefaultAssociatedWrite = {{
      {"WriteRes", VarVal(2, DefaultWriteRes)},
      {"Writes", VarVal{VarKind::Obj, 4, std::vector<VarVal>{}}},
  }};
  inline static const ObjectVal DefaultProcResource = {{
      {"NumUnits", VarVal{static_cast<int64_t>(1)}},
      {"Super", VarVal{1, nullptr}},
      {"AssociatedWrites", VarVal{0, DefaultAssociatedWrite}},
  }};
  inline static const ObjectVal DefaultReadAdvance = {{
      {"Cycles", VarVal(VarVal::createUninitialized(
                     VarType{VarKind::Int}))}, // Uninitialized
      {"ValidWrites", VarVal{VarKind::Ref, 4,
                             std::vector<VarVal>{}}}, // list<SchedWrite>
  }};

  inline static const ObjectVal DefaultProcResGroup = {{
      {"Resources",
       VarVal{VarKind::Ref, 1, std::vector<VarVal>{}}},
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
      {"SchedRead", 6}, {"RISCVFeature", 7},
  };

  inline static const Vec<ObjectVal> TypeMembers = {
      DefaultAssociatedWrite,

      DefaultProcResource,

      DefaultWriteRes,

      DefaultReadAdvance,

      DefaultSchedWrite,

      DefaultProcResGroup,

      DefaultSchedRead,

      DefaultRISCVFeature,
  };
  inline static const StringMap<VarVal> DefaultArchVals = {
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
      // SchedReads
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
  };

  Vec<StringMap<VarVal>> PrevArchVals;
  StringMap<VarVal> CurrentArchVals;
  Vec<VarInfo> VarStack;
  // Stores the size of VarStack before this overwrite
  Vec<size_t> OverwriteStack;
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

  VarType getCurrVarType() const;
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
  if (CurrentArchIdx != -1) {
    PrevArchVals.push_back(std::move(CurrentArchVals));
  }
  if (Ids.size() == 2) {
    std::string InheritName = Ids[1]->toString();
    const int64_t ArchIndex = getArchIndex(InheritName, Res);
    if (ArchIndex == -1) {
      // TODO: correct error
      throw "could not find arch def";
    }
    // Copy values of inherited arch (Copying tables may not be necessary)
    Res.ProcResourceTables.push_back(Res.ProcResourceTables[ArchIndex]);
    Res.SchedClassTables.push_back(Res.SchedClassTables[ArchIndex]);

    CurrentArchVals = PrevArchVals[ArchIndex];
  } else {
    Res.ProcResourceTables.emplace_back();
    Res.SchedClassTables.emplace_back();

    CurrentArchVals = DefaultArchVals;
  }
  Res.ArchNames.push_back(std::move(ArchName));
  ++CurrentArchIdx;
}

void DSLListener::exitArchitectureDefinition(
    DSLGrammarParser::ArchitectureDefinitionContext *AD) {
  // TODO: Write Values read from file

  assert(VarStack.size() == 0);
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
      // TODO: correct error
      throw std::runtime_error{"Undefined reference to " + Spell};
    }
    return Found->getValue();
  }
  return VarStack[VarStack.size() - 1].Val;
}

DSLListener::VarType DSLListener::getCurrVarType() const {
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
  if (Type.Kind == VarKind::Invalid) {
    std::cout << "Invalid Type on " << VarStack[VarStack.size() - 1].Id << "\n";
  }
  assert(Type.Kind != VarKind::Invalid);
  // TODO: Check assignment types and write CurrentVal
  size_t NewSize = OverwriteStack.back();
  OverwriteStack.pop_back();
  while (VarStack.size() > NewSize) {
    // TODO: write value here
    VarInfo Curr = std::move(VarStack.back());
    VarStack.pop_back();
    /* TODO:
    if (VarStack.empty()) {
      auto It = CurrentArchVals.find(Curr.Id);
      assert(It != CurrentArchVals.end());
      It->getValue() = std::move(Curr.Val);
    } else {
      VarInfo &Prev = VarStack.back();
      assert(Prev.Val.Type.Type == VarKind::Obj);
      Prev.Val.ObjVal.writeMember(Curr.Id, std::move(Curr.Val));
    }
     */
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
    assert(TypeIdx < TypeMembers.size());
    CurrentDefVal = VarVal{TypeIdx, TypeMembers[TypeIdx]};
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
  // TODO: Maybe for every object that is in a list create an Object with empty ID? Every empty ID would be assumed to be part of a list (Assert that the Object on the stack before it is a list)
  if (auto *Int = Expr->INT()) {
    std::cout << "INT: " << Int->toString() << '\n';
  } else if (auto *Bool = Expr->BOOL()) {
    std::cout << "BOOL: " << Bool->toString() << '\n';
  } else if (auto *Id = Expr->ID()) {
    // TODO: reference or value
    std::cout << "ID: " << Id->toString() << '\n';
  } else if (auto *Lst = Expr->list()) {
    std::cout << "List: " << Lst->toString() << '\n';
  } else if (auto *Obj = Expr->obj()) {
    std::cout << "Object: " << Obj->toString() << '\n';
  }
}
void DSLListener::exitExpr(DSLGrammarParser::ExprContext *Expr) {
  // TODO:
}

void DSLListener::enterList(DSLGrammarParser::ListContext *List) {
  // TODO: error if current variable is not a list
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

RISCVDynSubtargetData getDynSubtargetData(const char *Filename) {
  std::ifstream Stream{Filename};
  antlr4::ANTLRInputStream In{Stream};
  DSLGrammarLexer Lexer{&In};
  antlr4::CommonTokenStream Toks{&Lexer};
  DSLGrammarParser Parser{&Toks};
  antlr4::tree::ParseTree *Tree = Parser.translationUnit();
  RISCVDynSubtargetData Res;
  DSLListener Listener{Res};
  antlr4::tree::ParseTreeWalker::DEFAULT.walk(&Listener, Tree);
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
