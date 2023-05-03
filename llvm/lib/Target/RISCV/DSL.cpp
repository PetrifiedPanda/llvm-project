#include "DSL.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace llvm {

VarType::VarType(VarKind Kind) : Kind{Kind} {
  assert(Kind != VarKind::Obj);
  assert(Kind != VarKind::Invalid);
  assert(Kind != VarKind::List);
  assert(Kind != VarKind::Ref);
}

VarType::VarType(VarKind Kind, size_t ObjTypeIdx)
    : Kind{Kind}, ObjTypeIdx{ObjTypeIdx} {
  assert(Kind == VarKind::Obj || Kind == VarKind::Ref);
}

VarType::VarType(VarKind Kind, size_t ObjTypeIdx, VarKind ListKind,
                 size_t ListObjTypeIdx)
    : Kind{Kind}, ObjTypeIdx{ObjTypeIdx}, ListKind{ListKind},
      ListObjTypeIdx{ListObjTypeIdx} {}

bool VarType::operator==(const VarType &Other) const {
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

bool VarType::operator!=(const VarType &Other) const {
  return !(*this == Other);
}

ObjectVal::ObjectVal(MemberMap &&Members) : Members{std::move(Members)} {}

void ObjectVal::writeMember(const std::string &Id, VarVal &&Val) {
  auto It = Members.find(Id);
  assert(It != Members.end());
  VarVal &ToWrite = It->second;
  if (ToWrite.Type.Kind != Val.Type.Kind) {
    throw std::runtime_error{
        "Type Mismatch: Expected " +
        std::to_string(static_cast<size_t>(ToWrite.Type.Kind)) + " but got " +
        std::to_string(static_cast<size_t>(Val.Type.Kind))};
  }

  if (ToWrite.Type.Kind == VarKind::Obj || ToWrite.Type.Kind == VarKind::Ref) {
    assert(Val.Type.Kind == VarKind::Obj || Val.Type.Kind == VarKind::Ref);
    if (ToWrite.Type.ObjTypeIdx != Val.Type.ObjTypeIdx) {
      // TODO: errror
      throw std::runtime_error{"Type Mismatch(Object Type): Expected " +
                               std::to_string(ToWrite.Type.ObjTypeIdx) +
                               " but got " +
                               std::to_string(Val.Type.ObjTypeIdx)};
    }
  }
  ToWrite = std::move(Val);
}

VarVal VarVal::createUninitialized(VarType Type) {
  VarVal Res;
  memset(&Res, 0, sizeof Res);
  Res.Type = Type;
  Res.Initialized = false;
  return Res;
}

VarVal::VarVal(size_t TypeIdx, const ObjectVal &Val)
    : Type{VarKind::Obj, TypeIdx}, ObjVal{Val}, Initialized{true} {
  assert(TypeIdx != InvalidTypeIdx);
}

VarVal::VarVal(size_t RefTypeIdx, std::string&& Key)
    : Type{VarKind::Ref, RefTypeIdx}, Key{std::move(Key)} {
  assert(RefTypeIdx != InvalidTypeIdx);
}

VarVal::VarVal(const VarVal &Other) : Type{Other.Type} {
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
    new (&Key) std::string(Other.Key);
    break;
  }
  Initialized = Other.Initialized;
}

VarVal::~VarVal() {
  switch (Type.Kind) {
  case VarKind::Invalid:
  case VarKind::Bool:
  case VarKind::Int:
    break;
  case VarKind::Obj:
    ObjVal.~ObjectVal();
    break;
  case VarKind::List:
    LstVal.~ListVal();
    break;
  case VarKind::Ref:
    Key.~basic_string();
    break;
  }
}

// TODO: assigning to garbage memory error
VarVal &VarVal::operator=(const VarVal &Other) {
  if (this == &Other) {
    return *this;
  }
  if (Type.Kind == VarKind::Obj) {
    ObjVal.~ObjectVal();
  } else if (Type.Kind == VarKind::List) {
    LstVal.~ListVal();
  } else if (Type.Kind == VarKind::Ref) {
    Key.~basic_string();
  }
  Initialized = Other.Initialized;
  Type = Other.Type;
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
    new (&Key) std::string(Other.Key);
    break;
  }
  return *this;
}
// TODO: Make this work with self-assignment
VarVal &VarVal::operator=(VarVal &&Other) {
  if (this == &Other) {
    return *this;
  }
  if (Type.Kind == VarKind::Obj) {
    ObjVal.~ObjectVal();
  } else if (Type.Kind == VarKind::List) {
    LstVal.~ListVal();
  } else if (Type.Kind == VarKind::Ref) {
    Key.~basic_string();
  }
  Initialized = Other.Initialized;
  Type = Other.Type;
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
    new (&Key) std::string(std::move(Other.Key));
    break;
  }
  return *this;
}

} // namespace llvm
