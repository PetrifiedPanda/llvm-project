#include "RISCVDynSubtargetInfo.h"

namespace llvm {

MCSchedModel getDynSubtargetSchedModel(const RISCVDynSubtargetData &SD) {
  static constexpr unsigned int UINT_PLACEHOLDER = -1;
  static constexpr bool BOOL_PLACEHOLDER = false;

  const unsigned NumProcResourceKinds =
      static_cast<unsigned>(SD.ProcResourceTable.size());
  const unsigned NumSchedClasses =
      static_cast<unsigned>(SD.SchedClassTable.size());
  assert(NumProcResourceKinds == SD.ProcResourceTable.size());
  assert(NumSchedClasses == SD.SchedClassTable.size());

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
      SD.ProcResourceTable.data(),
      SD.SchedClassTable.data(),
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
