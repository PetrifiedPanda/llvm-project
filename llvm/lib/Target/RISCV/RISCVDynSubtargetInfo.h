#ifndef RISCV_DYN_SUBTARGET_H
#define RISCV_DYN_SUBTARGET_H

#include "llvm/MC/MCSchedule.h"

#include "llvm/CodeGen/TargetSubtargetInfo.h"

namespace llvm {

struct RISCVDynSubtargetData {
  SmallVector<std::string> ArchNames;
  // Data for SubtargetInfo
  SmallVector<SubtargetFeatureKV> FeatureKV;
  SmallVector<SubtargetSubTypeKV> SubTypeKV;

  SmallVector<MCWriteProcResEntry> WriteProcResTable;
  SmallVector<MCWriteLatencyEntry> WriteLatencyTable;
  SmallVector<MCReadAdvanceEntry> ReadAdvanceTable;

  // Data for each schedModel
  SmallVector<SmallVector<MCProcResourceDesc>> ProcResourceTables;
  SmallVector<SmallVector<MCSchedClassDesc>> SchedClassTables;

  SmallVector<MCSchedModel> SchedModels;

  RISCVDynSubtargetData() = default;
  RISCVDynSubtargetData(const RISCVDynSubtargetData &) = delete;
  RISCVDynSubtargetData(RISCVDynSubtargetData &&) = default;
};

RISCVDynSubtargetData getDynSubtargetData(const char* Filename);

class RISCVDynSubtargetInfo : public TargetSubtargetInfo {
  RISCVDynSubtargetInfo(const RISCVDynSubtargetData &SD, const Triple &TT,
                        StringRef CPU, StringRef TuneCPU, StringRef FS);
};

} // namespace llvm

#endif
