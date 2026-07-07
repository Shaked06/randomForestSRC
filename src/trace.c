
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "trace.h"
#include "error.h"
unsigned int    RF_traceFlagDiagLevel;
unsigned int    RF_traceFlagIterValue;
size_t          RF_memor_maxMemoryAllocation;
size_t          RF_memor_minMemoryAllocation;
size_t          RF_memor_probeMemoryAllocation;
void setTraceFlag(unsigned int traceFlag, unsigned int tree) {
  RF_traceFlagDiagLevel = traceFlag;
  RF_traceFlagIterValue = tree;
}
unsigned int getTraceFlag(unsigned int tree) {
  unsigned int result;
  result = FALSE;
  if (RF_traceFlagIterValue == tree) {
    result = RF_traceFlagDiagLevel;
  }
  else {
    if (RF_traceFlagIterValue == 0) {
      result = RF_traceFlagDiagLevel;
    }
  }
  return result;
}
unsigned int updateTimeStamp(unsigned int before) {
  unsigned int stamp;
  double cpuTimeUsed;
  stamp = clock();
  cpuTimeUsed = ((double) (stamp - before)) / CLOCKS_PER_SEC;
  RF_nativePrint("\nRF-SRC:  CPU process time:  %20.3f \n", cpuTimeUsed);
  return stamp;
}
void memoryCheck(void) {
  //  if (getTraceFlag(0) & SUMM_DEF_TRACE) {
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\nMaximum stack usage:  usage = %20lld ", getMaxMemoryAllocation());
  //    RF_nativePrint("\nExiting stack usage:  usage = %20lld ", getMinMemoryAllocation());
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\n");
  //  }
  //  if (getMinMemoryAllocation() != 0) {
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\nProbe (informal) stack usage:  usage = %20lld ", getProbeMemoryAllocation());
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\n");
  //  }
  //  if (getMinMemoryAllocation() != 0) {
  //    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
  //    RF_nativeError("\nRF-SRC:  Stack imbalance, exiting usage is not zero (0):  %20d  ", getMinMemoryAllocation());
  //    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\n");
  //    RF_nativeExit();
  //  }
}
void setMaxMemoryAllocation(size_t value) {
  RF_memor_maxMemoryAllocation = value;
}
void setMinMemoryAllocation(size_t value) {
  RF_memor_minMemoryAllocation = value;
}
void setProbeMemoryAllocation(size_t value) {
  RF_memor_probeMemoryAllocation = value;
}
size_t getMaxMemoryAllocation(void) {
  return (RF_memor_maxMemoryAllocation);
}
size_t getMinMemoryAllocation(void) {
  return (RF_memor_minMemoryAllocation);
}
size_t getProbeMemoryAllocation(void) {
  return (RF_memor_probeMemoryAllocation);
}
void increaseMemoryAllocation(size_t amount) {
  RF_memor_minMemoryAllocation += amount;
  if (RF_memor_minMemoryAllocation > RF_memor_maxMemoryAllocation) {
    RF_memor_maxMemoryAllocation = RF_memor_minMemoryAllocation;
  }
}
void increaseProbeMemoryAllocation(size_t amount) {
  RF_memor_probeMemoryAllocation += amount;
}
void decreaseMemoryAllocation(size_t amount) {
    RF_memor_minMemoryAllocation -= amount;
}
void decreaseProbeMemoryAllocation(size_t amount) {
    RF_memor_probeMemoryAllocation -= amount;
}
