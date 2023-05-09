#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace llvm {

template <typename T> using IdentifierMap = std::map<std::string, T>;

template <typename T> using Vec = std::vector<T>;

enum class VarKind {
  Invalid,
  Bool,
  Int,
  Obj,
  List,
  Ref,
};

struct VarType {
  static constexpr size_t InvalidTypeIdx = static_cast<size_t>(-1);

  VarKind Kind = VarKind::Invalid;
  size_t ObjTypeIdx = InvalidTypeIdx;
  VarKind ListKind = VarKind::Invalid;
  size_t ListObjTypeIdx = InvalidTypeIdx;

  VarType() {}
  explicit VarType(VarKind Kind);
  VarType(VarKind Kind, size_t ObjTypeIdx);
  VarType(VarKind Kind, size_t ObjTypeIdx, VarKind ListKind,
          size_t ListObjTypeIdx);

  bool operator==(const VarType &Other) const;
  bool operator!=(const VarType &Other) const;
};

struct VarVal {
  VarType Type;
  union {
    int64_t IntVal;
    bool BoolVal;
    // Must be std::vector because SmallVector does not work with incomplete type
    std::vector<VarVal> LstVal;
    IdentifierMap<VarVal> ObjVal;
    std::string RefKey;
  };
  bool Initialized;

  static VarVal createUninitialized(VarType Type);

  VarVal() : Type{}, Initialized{false} {}
  // TODO: normal int literal will be ambiguous, because it might be a bool
  VarVal(int64_t IntVal)
      : Type{VarKind::Int}, IntVal{IntVal}, Initialized{true} {}
  VarVal(bool BoolVal)
      : Type{VarKind::Bool}, BoolVal{BoolVal}, Initialized{true} {}
  VarVal(size_t TypeIdx, const IdentifierMap<VarVal> &Val);
  VarVal(size_t TypeIdx, IdentifierMap<VarVal> &&Val);
  VarVal(VarKind Kind, size_t ObjTypeIdx, std::vector<VarVal> &&Vals)
      : Type{VarKind::List, VarType::InvalidTypeIdx, Kind, ObjTypeIdx},
        LstVal{std::move(Vals)} {}
  VarVal(size_t RefTypeIdx, std::string&& Key);
  VarVal(const VarVal &Other);
  ~VarVal();

  VarVal &operator=(const VarVal &Other);
  VarVal &operator=(VarVal &&Other);

  bool operator==(const VarVal &Other) const;
  bool operator!=(const VarVal &Other) const;

  void writeMember(const std::string &Id, VarVal &&Val);
};

} // namespace llvm
