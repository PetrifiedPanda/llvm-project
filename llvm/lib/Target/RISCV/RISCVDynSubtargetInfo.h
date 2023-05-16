#ifndef RISCV_DYN_SUBTARGET_H
#define RISCV_DYN_SUBTARGET_H

#include "llvm/MC/MCSchedule.h"
#include "llvm/MC/MCInstrDesc.h"

#include "llvm/CodeGen/TargetSubtargetInfo.h"

#include "DSL.h"

namespace llvm {

struct RISCVDynSubtargetData {
  Vec<std::string> ArchNames;
  // Data for SubtargetInfo
  Vec<SubtargetFeatureKV> FeatureKV;
  Vec<SubtargetSubTypeKV> SubTypeKV;

  Vec<MCWriteProcResEntry> WriteProcResTable;
  Vec<MCWriteLatencyEntry> WriteLatencyTable;
  Vec<MCReadAdvanceEntry> ReadAdvanceTable;

  // Data for each schedModel

  // As the names are const char* in MCProcResourceDesc, they need to be allocated somewhere
  // This is a vector of vectors, so we can predetermine each vectors size (number of ProcResources) and no reallocation takes place, which might invalidate the char pointers if the strings are small
  Vec<Vec<std::string>> ProcResourceNames;
  // Holds the Subunit indices for the ProcResouces
  Vec<Vec<Vec<unsigned>>> ProcResourceSubUnits;
  Vec<Vec<MCProcResourceDesc>> ProcResourceTables;
  Vec<Vec<MCSchedClassDesc>> SchedClassTables;

  Vec<MCSchedModel> SchedModels;

  Vec<MCInstrDesc> InstrDescs;

  RISCVDynSubtargetData() = default;
  RISCVDynSubtargetData(const RISCVDynSubtargetData &) = delete;
  RISCVDynSubtargetData(RISCVDynSubtargetData &&) = default;
};

struct DSLArchData {
  std::string Name;
  IdentifierMap<VarVal> Vals;
};

Vec<DSLArchData> getDSLArchData(const char* Filename);

RISCVDynSubtargetData getDynSubtargetData(const char* Filename);

class RISCVDynSubtargetInfo : public TargetSubtargetInfo {
  RISCVDynSubtargetInfo(const RISCVDynSubtargetData &SD, const Triple &TT,
                        StringRef CPU, StringRef TuneCPU, StringRef FS);
};

} // namespace llvm

#endif
