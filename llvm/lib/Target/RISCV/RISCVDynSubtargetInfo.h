#ifndef RISCV_DYN_SUBTARGET_H
#define RISCV_DYN_SUBTARGET_H

#include <vector>

#include "llvm/MC/MCSchedule.h"

#include "llvm/CodeGen/TargetSubtargetInfo.h"

namespace llvm {

struct RISCVDynSubtargetData {
  // Data for SubtargetInfo
  std::vector<SubtargetFeatureKV> FeatureKV;
  std::vector<SubtargetSubTypeKV> SubTypeKV;

  std::vector<MCWriteProcResEntry> WriteProcResTable;
  std::vector<MCWriteLatencyEntry> WriteLatencyTable;
  std::vector<MCReadAdvanceEntry> ReadAdvanceTable;

  // Data for schedModel
  std::vector<MCProcResourceDesc> ProcResourceTable;
  std::vector<MCSchedClassDesc> SchedClassTable;

  RISCVDynSubtargetData() = delete;
  RISCVDynSubtargetData(const RISCVDynSubtargetData &) = delete;
  RISCVDynSubtargetData(RISCVDynSubtargetData &&) = default;
};

MCSchedModel getDynSubtargetSchedModel(const RISCVDynSubtargetData &SD);

class RISCVDynSubtargetInfo : public TargetSubtargetInfo {
  RISCVDynSubtargetInfo(const RISCVDynSubtargetData &SD, const Triple &TT,
                        StringRef CPU, StringRef TuneCPU, StringRef FS);
};

} // namespace llvm

#endif
