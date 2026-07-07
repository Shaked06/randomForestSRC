
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "polarity.h"
#include "factorOps.h"
#include "nrutil.h"
// #include "error.h"
char getDaughterPolarity(uint treeID, SplitInfo *info, uint indv, void *value, ...) {
  char (*getDaughterPolarityGeneric) (uint       treeID,
                                      SplitInfo *info,
                                      uint       indv,
                                      void      *value,
                                      ...);
  void *obsLocal;
  char daughterFlag;
    obsLocal = ((double **) value)[info -> randomVar[1]];
    if (info -> mwcpSizeAbs[1] > 0) {
      getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
    }
    else {
      getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
    }
  daughterFlag = getDaughterPolarityGeneric(0, info, indv, obsLocal);
  return daughterFlag;
}
char getDaughterPolaritySimpleFactor(uint treeID, SplitInfo *info, uint indv, void *value, ...) {
  char daughterFlag;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleFactor(%10d) ENTRY ...\n", treeID);
  //    }
  //  }
  //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //        if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //          RF_nativePrint("\nNon-Greedy Daughter Comparison (value, const):  (%10d, ", (uint) ((double *) value)[indv]);
  //          for (uint m = 1; m <= info -> mwcpSizeAbs[1]; m++) {
  //            RF_nativePrint(" %10x", ((uint*) info -> randomPts[1])[m]);
  //          }
  //          RF_nativePrint(")");
  //        }
  //      }
  daughterFlag = splitOnFactor((uint) ((double *) value)[indv], (uint*) info -> randomPts[1]);
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleFactor(%10d) EXIT ...\n", treeID);
  //    }
  //  }
  return daughterFlag;
}
char getDaughterPolaritySimpleNonFactor(uint treeID, SplitInfo *info, uint indv, void *value, ...) {
  char daughterFlag;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleNonFactor(%10d) ENTRY ...\n", treeID);
  //    }
  //  }
  //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //        if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //          RF_nativePrint("\nNon-Greedy Daughter Comparison (value, const):  (%10.4f, %10.4f)", ((double *) value)[indv], ((double*) info -> randomPts[1])[1]);
  //        }
  //      }
  daughterFlag =  (( ((double*) info -> randomPts[1])[1] - ((double *) value)[indv]) >= 0.0) ? LEFT : RIGHT;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleNonFactor(%10d) EXIT ...\n", treeID);
  //    }
  //  }
  return daughterFlag;
}
char getDaughterPolaritySimpleFactorSingle(uint treeID, SplitInfo *info, uint indv, void *value, ...) {
  char daughterFlag;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleFactorSingle(%10d) ENTRY ...\n", treeID);
  //    }
  //  }
  //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //        if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //          RF_nativePrint("\nNon-Greedy Daughter Comparison (value, const):  (%10d, ", *((uint *) ((double *) value)));
  //          for (uint m = 1; m <= info -> mwcpSizeAbs[1]; m++) {
  //            RF_nativePrint(" %10x", ((uint*) info -> randomPts[1])[m]);
  //          }
  //          RF_nativePrint(")");
  //        }
  //      }
  daughterFlag = splitOnFactor(*((uint *) ((double *) value)), (uint*) info -> randomPts[1]);
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleFactorSingle(%10d) EXIT ...\n", treeID);
  //    }
  //  }
  return daughterFlag;
}
char getDaughterPolaritySimpleNonFactorSingle(uint treeID, SplitInfo *info, uint indv, void *value, ...) {
  char daughterFlag;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleNonFactorSingle(%10d) ENTRY ...\n", treeID);
  //    }
  //  }
  //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //        if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //          RF_nativePrint("\nNon-Greedy Daughter Comparison (value, const):  (%10.4f, %10.4f)", *((double *) value), ((double*) info -> randomPts[1])[1]);
  //        }
  //      }
  daughterFlag =  (( ((double*) info -> randomPts[1])[1] - (*((double *) value))) >= 0.0) ? LEFT : RIGHT;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //      RF_nativePrint("\ngetDaughterPolaritySimpleNonFactorSingle(%10d) EXIT ...\n", treeID);
  //    }
  //  }
  return daughterFlag;
}
