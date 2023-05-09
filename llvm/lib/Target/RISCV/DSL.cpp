#include "DSL.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

#include <iostream>

#include "llvm/Support/ErrorHandling.h"

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

VarVal VarVal::createUninitialized(VarType Type) {
  VarVal Res;
  // TODO: memset might not be necessary
  memset(&Res, 0, sizeof Res);
  Res.Type = Type;
  Res.Initialized = false;
  return Res;
}

VarVal::VarVal(size_t TypeIdx, const IdentifierMap<VarVal> &Val)
    : Type{VarKind::Obj, TypeIdx}, ObjVal{Val}, Initialized{true} {
  assert(TypeIdx != VarType::InvalidTypeIdx);
}

VarVal::VarVal(size_t TypeIdx, IdentifierMap<VarVal> &&Val)
    : Type{VarKind::Obj, TypeIdx}, ObjVal{std::move(Val)}, Initialized{true} {
  assert(TypeIdx != VarType::InvalidTypeIdx);
}

VarVal::VarVal(size_t RefTypeIdx, std::string &&Key)
    : Type{VarKind::Ref, RefTypeIdx}, RefKey{std::move(Key)}, Initialized{true} {
  assert(RefTypeIdx != VarType::InvalidTypeIdx);
}

VarVal::VarVal(const VarVal &Other)
    : Type{Other.Type}, Initialized{Other.Initialized} {
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
    new (&ObjVal) IdentifierMap<VarVal>(Other.ObjVal);
    break;
  case VarKind::List:
    new (&LstVal) std::vector(Other.LstVal);
    break;
  case VarKind::Ref:
    new (&RefKey) std::string(Other.RefKey);
    break;
  }
}

VarVal::~VarVal() {
  switch (Type.Kind) {
  case VarKind::Invalid:
  case VarKind::Bool:
  case VarKind::Int:
    break;
  case VarKind::Obj:
    ObjVal.~IdentifierMap<VarVal>();
    break;
  case VarKind::List:
    LstVal.~vector();
    break;
  case VarKind::Ref:
    RefKey.~basic_string();
    break;
  }
}

// TODO: assigning to garbage memory error
VarVal &VarVal::operator=(const VarVal &Other) {
  if (this == &Other) {
    return *this;
  }
  if (Type.Kind == VarKind::Obj) {
    ObjVal.~IdentifierMap<VarVal>();
  } else if (Type.Kind == VarKind::List) {
    LstVal.~vector();
  } else if (Type.Kind == VarKind::Ref) {
    RefKey.~basic_string();
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
    new (&ObjVal) IdentifierMap<VarVal>(Other.ObjVal);
    break;
  case VarKind::List:
    new (&LstVal) std::vector(Other.LstVal);
    break;
  case VarKind::Ref:
    new (&RefKey) std::string(Other.RefKey);
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
    ObjVal.~IdentifierMap<VarVal>();
  } else if (Type.Kind == VarKind::List) {
    LstVal.~vector();
  } else if (Type.Kind == VarKind::Ref) {
    RefKey.~basic_string();
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
    new (&ObjVal) IdentifierMap<VarVal>(std::move(Other.ObjVal));
    break;
  case VarKind::List:
    new (&LstVal) std::vector(std::move(Other.LstVal));
    break;
  case VarKind::Ref:
    new (&RefKey) std::string(std::move(Other.RefKey));
    break;
  }
  return *this;
}

bool VarVal::operator==(const VarVal &Other) const {
  if (Type != Other.Type) {
    return false;
  }

  switch (Type.Kind) {
  case VarKind::Invalid:
    return true;
  case VarKind::Bool:
    return BoolVal == Other.BoolVal;
  case VarKind::Int:
    return IntVal == Other.IntVal;
  case VarKind::Obj:
    return ObjVal == Other.ObjVal;
  case VarKind::List:
    return LstVal == Other.LstVal;
  case VarKind::Ref:
    return RefKey == Other.RefKey;
  }
  llvm_unreachable("Unhandled VarKind");
}

bool VarVal::operator!=(const VarVal &Other) const { return !(*this == Other); }

void VarVal::writeMember(const std::string &Id, VarVal &&Val) {
  assert(Type.Kind == VarKind::Obj);
  auto It = ObjVal.find(Id);
  assert(It != ObjVal.end());
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

} // namespace llvm
