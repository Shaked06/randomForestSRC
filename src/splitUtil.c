
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "splitUtil.h"
#include "factor.h"
#include "factorOps.h"
#include "nrutil.h"
#include "error.h"
char getPreSplitResultGeneric (uint      treeID,
                               Node     *parent,
                               char      multImpFlag,
                               char      multVarFlag) {
  uint i, r;
  char mResponseFlag;
  char result;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\ngetPreSplitResultGeneric(%10d) ENTRY ...\n", treeID);
  //  }
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\n  called with   rep size:  %10d", parent -> repMembrSize);
  //  }
  result = TRUE;
  if (result) {
    if (parent -> repMembrSize >= (2 * RF_nodeSize)) {
      result = TRUE;
    }
    else {
      result = FALSE;
      //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
      //      RF_nativePrint("\nLess than twice the minimum number of replicates encountered.  ");
      //      RF_nativePrint("\nNode will not be split.  \n");
      //    }
    }
  }
  if (result) {
    if (RF_nodeDepth < 0) {
      result = TRUE;
    }
    else {
      if (parent -> depth < (uint) RF_nodeDepth) {
        result = TRUE;
      }
      else {
        result = FALSE;
        //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
        //      RF_nativePrint("\nMaximum node depth encountered.  ");
        //      RF_nativePrint("\nNode will not be split.  \n");
        //    }
      }
    }
  }
  if (result) {
    if ((RF_mRecordSize == 0) || multImpFlag || !(RF_optHigh & OPT_MISS_SKIP) || multVarFlag) {
      parent -> nonMissMembrSizeStatic = parent -> repMembrSize;
      parent -> nonMissMembrIndxStatic = RF_identityMembershipIndex;
      //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
      //      RF_nativePrint("\nNon-miss membership index assuming identity mapping.  ");
      //    }
    }
    else {
      parent -> nonMissMembrIndxStatic = uivector(1, parent -> repMembrSize);
      parent -> nonMissMembrSizeStatic = 0;
      for (i = 1; i <= parent -> repMembrSize; i++) {
        mResponseFlag = FALSE;
        if (RF_mRecordMap[parent -> repMembrIndx[i]] > 0) {
          for (r = 1; r <= RF_ySize; r++) {
            if (RF_mpSign[r][RF_mRecordMap[parent -> repMembrIndx[i]]] == 1) {
              mResponseFlag = TRUE;
            }
          }
        }
        if (!mResponseFlag) {
          (parent -> nonMissMembrSizeStatic) ++;
          (parent -> nonMissMembrIndxStatic)[parent -> nonMissMembrSizeStatic] = i;
        }
      }  
    }  
    if (!multVarFlag) {
      if ((RF_timeIndex > 0) && (RF_statusIndex > 0)) {
        uint q,k,m;
        uint *evntProp = uivector(1, RF_eventTypeSize + 1);
        for (q = 1; q <= RF_eventTypeSize + 1; q++) {
          evntProp[q] = 0;
        }
        for (i = 1; i <= parent -> nonMissMembrSizeStatic; i++) {
          m = (uint) RF_status[treeID][(parent -> repMembrIndx)[parent -> nonMissMembrIndxStatic[i]]];
          if (m > 0) {
            evntProp[RF_eventTypeIndex[m]] ++;
          }
          else {
            evntProp[RF_eventTypeSize + 1] ++;
          }
        }
        k = 0;
        for (q = 1; q <= RF_eventTypeSize + 1; q++) {
          if(evntProp[q] > 0) {
            k ++;
          }
        }
        if (k == 0) {
          result = FALSE;
        }
        else {
          if (k == 1) {
            if (evntProp[RF_eventTypeSize + 1] > 0) {
              result = FALSE;
            }
            else {
              result = getVariance(parent -> repMembrSize,
                                   parent -> repMembrIndx,
                                   parent -> nonMissMembrSizeStatic,
                                   parent -> nonMissMembrIndxStatic,
                                   RF_time[treeID],
                                   & (parent -> mean),
                                   NULL);
            }
          }
        }
        //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
        //      if (!result) {
        //        RF_nativePrint("\nNode purity encountered.  ");
        //        RF_nativePrint("\nNode will not be split.  \n");
        //      }
        //    }
        free_uivector(evntProp, 1, RF_eventTypeSize + 1);
      }
      else {
        result = getVariance(parent -> repMembrSize,
                             parent -> repMembrIndx,
                             parent -> nonMissMembrSizeStatic,
                             parent -> nonMissMembrIndxStatic,
                             RF_response[treeID][1],
                             & (parent -> mean),
                             NULL);
      }
    }
    if (!result) {
      parent -> nonMissMembrSizeStatic = 0;
      if (!((RF_mRecordSize == 0) || multImpFlag || !(RF_optHigh & OPT_MISS_SKIP) || multVarFlag)) {
        free_uivector(parent -> nonMissMembrIndxStatic, 1, parent -> repMembrSize);
        parent -> nonMissMembrIndxStatic = NULL;
        parent -> nonMissMembrIndx       = NULL;
      }
    }
  }
  parent -> nonMissMembrIndx = parent -> nonMissMembrIndxStatic;
  parent -> nonMissMembrSize  = parent -> nonMissMembrSizeStatic;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\ngetPreSplitResultGeneric(%10d) EXIT ...\n", treeID);
  //  }
  return result;
}
char getPreSplitResultNoMiss (uint      treeID,
                              Node     *parent,
                              char      multImpFlag,
                              char      multVarFlag) {
  uint i;
  char result;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\ngetPreSplitResultNoMiss(%10d) ENTRY ...\n", treeID);
  //  }
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\n  called with   rep size:  %10d", parent -> repMembrSize);
  //  }
  result = TRUE;
  if (result) {
    if (parent -> repMembrSize >= (2 * RF_nodeSize)) {
      result = TRUE;
    }
    else {
      result = FALSE;
      //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
      //      RF_nativePrint("\nLess than twice the minimum number of replicates encountered.  ");
      //      RF_nativePrint("\nNode will not be split.  \n");
      //    }
    }
  }
  if (result) {
    if (RF_nodeDepth < 0) {
      result = TRUE;
    }
    else {
      if (parent -> depth < (uint) RF_nodeDepth) {
        result = TRUE;
      }
      else {
        result = FALSE;
        //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
        //      RF_nativePrint("\nMaximum node depth encountered.  ");
        //      RF_nativePrint("\nNode will not be split.  \n");
        //    }
      }
    }
  }
  if (result) {
    parent -> nonMissMembrSizeStatic = parent -> repMembrSize;
    parent -> nonMissMembrIndxStatic = RF_identityMembershipIndex;
    if (!multVarFlag) {
      if ((RF_timeIndex > 0) && (RF_statusIndex > 0)) {
        uint q,k,m;
        uint *evntProp = uivector(1, RF_eventTypeSize + 1);
        for (q = 1; q <= RF_eventTypeSize + 1; q++) {
          evntProp[q] = 0;
        }
        for (i = 1; i <= parent -> nonMissMembrSizeStatic; i++) {
          m = (uint) RF_status[treeID][(parent -> repMembrIndx)[parent -> nonMissMembrIndxStatic[i]]];
          if (m > 0) {
            evntProp[RF_eventTypeIndex[m]] ++;
          }
          else {
            evntProp[RF_eventTypeSize + 1] ++;
          }
        }
        k = 0;
        for (q = 1; q <= RF_eventTypeSize + 1; q++) {
          if(evntProp[q] > 0) {
            k ++;
          }
        }
        if (k == 0) {
          result = FALSE;
        }
        else {
          if (k == 1) {
            if (evntProp[RF_eventTypeSize + 1] > 0) {
              result = FALSE;
            }
            else {
              result = getVariance(parent -> repMembrSize,
                                   parent -> repMembrIndx,
                                   parent -> nonMissMembrSizeStatic,
                                   parent -> nonMissMembrIndxStatic,
                                   RF_time[treeID],
                                   & (parent -> mean),
                                   NULL);
            }
          }
        }
        //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
        //      if (!result) {
        //        RF_nativePrint("\nNode purity encountered.  ");
        //        RF_nativePrint("\nNode will not be split.  \n");
        //      }
        //    }
        free_uivector(evntProp, 1, RF_eventTypeSize + 1);
      }
      else {
        result = getVariance(parent -> repMembrSize,
                             parent -> repMembrIndx,
                             parent -> nonMissMembrSizeStatic,
                             parent -> nonMissMembrIndxStatic,
                             RF_response[treeID][1],
                             & (parent -> mean),
                             NULL);
      }
    }
    if (!result) {
      parent -> nonMissMembrSizeStatic = 0;
    }
  }
  parent -> nonMissMembrIndx = parent -> nonMissMembrIndxStatic;
  parent -> nonMissMembrSize  = parent -> nonMissMembrSizeStatic;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\ngetPreSplitResultNoMiss(%10d) EXIT ...\n", treeID);
  //  }
  return result;
}
void unstackPreSplit (char      preliminaryResult,
                      Node     *parent,
                      char      multImpFlag,
                      char      multVarFlag) {
  //  if (getTraceFlag(0) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nunstackPreSplit() ENTRY ...\n");
  //  }
  if (preliminaryResult) {
    if (!((RF_mRecordSize == 0) || multImpFlag || !(RF_optHigh & OPT_MISS_SKIP) || multVarFlag)) {
      free_uivector(parent -> nonMissMembrIndxStatic, 1, parent -> repMembrSize);
    }
  }
  else {
  }
  //  if (getTraceFlag(0) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nunstackPreSplit() EXIT ...\n");
  //  }
}
void stackSplitPreliminary(uint     nodeSize,
                           char   **localSplitIndicator,
                           double **splitVector) {
  //  if (getTraceFlag(0) & SPLT_MED_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\nstackSplitPreliminary() ENTRY ...\n");
  //    }
  //  }
  *localSplitIndicator = cvector(1, nodeSize);
  *splitVector = dvector(1, nodeSize);
  //  if (getTraceFlag(0) & SPLT_MED_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\nstackSplitPreliminary() EXIT ...\n");
  //    }
  //  }
}
void unstackSplitPreliminary(uint    nodeSize,
                             char   *localSplitIndicator,
                             double *splitVector) {
  //  if (getTraceFlag(0) & SPLT_MED_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\nunstackSplitPreliminary() ENTRY ...\n");
  //    }
  //  }
  free_cvector(localSplitIndicator, 1, nodeSize);
  free_dvector(splitVector, 1, nodeSize);
  //  if (getTraceFlag(0) & SPLT_MED_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\nunstackSplitPreliminary() EXIT ...\n");
  //    }
  //  }
}
DistributionObj *stackRandomCovariatesGeneric(uint treeID, Node *parent) {
  uint actualWeightType;
  uint *augmentationSize;
  char *permissible;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nstackRandomCovariatesGeneric(%10d) ENTRY ...\n", treeID);
  //  }
  DistributionObj *obj = makeDistributionObjRaw();
  actualWeightType = RF_xWeightType;
  augmentationSize    = NULL;
  permissible    = parent -> permissible;
  obj -> permissibleIndex    = NULL;
  obj -> permissible         = permissible;
  obj -> permissibleSize     = parent -> xSize;
  obj -> augmentationSize    = augmentationSize;
  obj -> weightType          = actualWeightType;
  obj -> weight              = RF_xWeight;
  obj -> weightSorted        = RF_xWeightSorted;
  obj -> densityAllocSize    = RF_xWeightDensitySize;
  initializeCDFNew(treeID, obj);
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nstackRandomCovariatesGeneric(%10d) EXIT ...\n", treeID);
  //  }
  return obj;
}
void unstackRandomCovariatesGeneric(uint treeID, DistributionObj *obj) {
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nunstackRandomCovariatesGeneric(%10d) ENTRY ...\n", treeID);
  //  }
  if (obj -> augmentationSize != NULL) {
    free_uivector(obj -> augmentationSize, 1, 2);
    obj -> augmentationSize = NULL;
  }
  discardCDFNew(treeID, obj);
  freeDistributionObjRaw(obj);
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nunstackRandomCovariatesGeneric(%10d) EXIT ...\n", treeID);
  //  }
}
char selectRandomCovariatesGeneric(uint     treeID,
                                   Node     *parent,
                                   DistributionObj *distributionObj,
                                   char     *factorFlag,
                                   uint     *covariate,
                                   uint     *covariateCount) {
  char xVarFound;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nselectRandomCovariatesGeneric(%10d) ENTRY ...\n", treeID);
  //  }
  (*covariate) = UINT_MAX;
  xVarFound = FALSE;
  *factorFlag = FALSE;
  while ( ((*covariateCount) < RF_mtry) && ((*covariate) != 0) && (xVarFound == FALSE)) {
    (*covariateCount) ++;
    //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
    //        RF_nativePrint("\nCovariate counter:  %10d \n", (*covariateCount));
    //      }
    *covariate = sampleFromCDFNew(ran1B, treeID, distributionObj);
    if (*covariate != 0) {
      updateCDFNew(treeID, distributionObj);
      //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
      //        RF_nativePrint("\nCovariate:  %10d \n", *covariate);
      //      }
      xVarFound = TRUE;
        if (RF_xType[*covariate] == 'C') {
          *factorFlag = TRUE;
          //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
          //        RF_nativePrint("\nCandidate covariate %10d is a factor.", *covariate);
          //      }
        }
    }  
  }  
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nselectRandomCovariatesGeneric(%10d) EXIT ...\n", treeID);
  //  }
  return xVarFound;
}
uint stackAndConstructSplitVectorGenericPhase1 (uint     treeID,
                                                Node    *parent,
                                                uint     covariate,
                                                ...) {
  uint offset;
  double *nonMissSplit;
  char mPredictorFlag;
  uint vectorSize;
  uint i, ii;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nstackAndConstructSplitVectorGenericPhase1(%10d) ENTRY ...\n", treeID);
  //  }
  uint  *repMembrIndx = parent -> repMembrIndx;
  uint   repMembrSize = parent -> repMembrSize;
  uint  *nonMissMembrIndxStatic = parent -> nonMissMembrIndxStatic;
  uint   nonMissMembrSizeStatic = parent -> nonMissMembrSizeStatic;
  uint **nonMissMembrIndx = & (parent -> nonMissMembrIndx);
  uint  *nonMissMembrSize = & (parent -> nonMissMembrSize);
  va_list list;
  va_start(list, covariate);
  double *splitVector = va_arg(list, double*);
  uint **indxx = (uint**) va_arg(list, uint**);
  char   multImpFlag = (char) va_arg(list, uint);
  nonMissSplit = dvector(1, repMembrSize);
  if ((RF_mRecordSize == 0) || (multImpFlag) || (!(RF_optHigh & OPT_MISS_SKIP))) {
    *nonMissMembrSize = nonMissMembrSizeStatic;
    *nonMissMembrIndx = nonMissMembrIndxStatic;
  }
  else {
    *nonMissMembrSize = 0;
    *nonMissMembrIndx = uivector(1, nonMissMembrSizeStatic);
  }
  vectorSize = 0;
  if ((RF_mRecordSize == 0) || (multImpFlag) || (!(RF_optHigh & OPT_MISS_SKIP))) {
      for (i = 1; i <= repMembrSize; i++) {
        nonMissSplit[i] = RF_observation[treeID][covariate][repMembrIndx[i]];
      }
      //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
      //          RF_nativePrint("\nNote that all responses are inculded below, and not just the ytry targets:");
      //        RF_nativePrint("\nReplicate:           indx    targ indx        covar       responses ->");
      //        for (i = 1; i <= repMembrSize; i++) {
      //          RF_nativePrint("\n               %10d %12d %12.4f",
      //                    i,
      //                    repMembrIndx[i],
      //                    RF_observation[treeID][covariate][repMembrIndx[i]]);
      //          for (uint r = 1; r<= RF_ySize; r++) {
      //            RF_nativePrint(" %12.4f", RF_response[treeID][r][repMembrIndx[i]]);
      //          }
      //        }
      //      }
  }
  else {
    offset = RF_ySize + covariate;
    (*nonMissMembrSize) = 0;
    for (i = 1; i <= nonMissMembrSizeStatic; i++) {
      ii = nonMissMembrIndxStatic[i];
      mPredictorFlag = FALSE;
      if (RF_mRecordMap[repMembrIndx[ii]] > 0) {
        if (RF_mpSign[offset][RF_mRecordMap[repMembrIndx[ii]]] == 1) {
          mPredictorFlag = TRUE;
        }
      }
      if (!mPredictorFlag) {
        (*nonMissMembrSize) ++;
        (*nonMissMembrIndx)[*nonMissMembrSize] = ii;
        nonMissSplit[*nonMissMembrSize] = RF_observation[treeID][covariate][repMembrIndx[(*nonMissMembrIndx)[*nonMissMembrSize]]];
      }
    }  
    //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
    //          RF_nativePrint("\nNote that all responses are inculded below, and not just the ytry targets:");
    //          RF_nativePrint("\nReplicate:           indx    targ indx        covar         responses ->");
    //          for (i = 1; i <= (*nonMissMembrSize); i++) {
    //            RF_nativePrint("\n               %10d %12d %12.4f",
    //                    i,
    //                    repMembrIndx[(*nonMissMembrIndx)[i]],
    //                    RF_observation[treeID][covariate][repMembrIndx[(*nonMissMembrIndx)[i]]]);
    //            for (uint r = 1; r<= RF_ySize; r++) {
    //              RF_nativePrint(" %12.4f", RF_response[treeID][r][repMembrIndx[(*nonMissMembrIndx)[i]]]);
    //            }
    //          }
    //        }
  }  
  if ((*nonMissMembrSize) >= 2) {
    (*indxx) = uivector(1, repMembrSize);
    indexx((*nonMissMembrSize),
           nonMissSplit,
           (*indxx));
    splitVector[1] = nonMissSplit[(*indxx)[1]];
    vectorSize = 1;
    for (i = 2; i <= (*nonMissMembrSize); i++) {
      if (nonMissSplit[(*indxx)[i]] > splitVector[vectorSize]) {
        vectorSize ++;
        splitVector[vectorSize] = nonMissSplit[(*indxx)[i]];
      }
    }
    //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
    //        RF_nativePrint("\n\nNon-miss member size:        %10d ", (*nonMissMembrSize));
    //        RF_nativePrint("\nPermissible split size:      %10d ", vectorSize);
    //      }
    if(vectorSize >= 2) {
      //        if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
      //          RF_nativePrint("\nCovariate accepted:          %10d \n", covariate);
      //          RF_nativePrint("\nsplit points:       index        value");
      //          for (i = 1; i <= vectorSize; i++) {
      //            RF_nativePrint("\n               %10d %12.4f", i, splitVector[i]);
      //          }
      //          RF_nativePrint("\n");
      //          RF_nativePrint("\nNon-miss Repl:       indx        indxx  unord value    ord value");
      //          for (i = 1; i <= (*nonMissMembrSize); i++) {
      //            RF_nativePrint("\n               %10d %12d %12.4f %12.4f ",
      //                    i,
      //                    (*indxx)[i],
      //                    nonMissSplit[i],
      //                    nonMissSplit[(*indxx)[i]]
      //                   );
      //          }
      //        }
    }
    else {
      vectorSize = 0;
      if (covariate <= RF_xSize) {
        (parent -> permissible)[covariate] = FALSE;
        parent -> permissibleReIndxFlag = TRUE;
      }
      free_uivector(*indxx, 1, repMembrSize);
      if ((RF_mRecordSize == 0) || (multImpFlag) || (!(RF_optHigh & OPT_MISS_SKIP))) {
        *nonMissMembrSize = 0;
        *nonMissMembrIndx = NULL;
      }
      else {
        free_uivector(*nonMissMembrIndx, 1, nonMissMembrSizeStatic);
        *nonMissMembrSize = 0;
        *nonMissMembrIndx = NULL;
      }
    //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
    //        RF_nativePrint("\nCovariate rejected due to non-miss split being less than two (2):  %10d ", covariate);
    //      }
    }
  }
  else {
    vectorSize = 0;
    if (covariate <= RF_xSize) {
      (parent -> permissible)[covariate] = FALSE;
      parent -> permissibleReIndxFlag = TRUE;
    }
    //        if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
    //          RF_nativePrint("\nCovariate rejected:  %10d \n", covariate);
    //        }
    if ((RF_mRecordSize == 0) || (multImpFlag) || (!(RF_optHigh & OPT_MISS_SKIP))) {
      *nonMissMembrSize = 0;
      *nonMissMembrIndx = NULL;
    }
    else {
      free_uivector(*nonMissMembrIndx, 1, nonMissMembrSizeStatic);
      *nonMissMembrSize = 0;
      *nonMissMembrIndx = NULL;
    }
  }
  free_dvector(nonMissSplit, 1, repMembrSize);
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nstackAndConstructSplitVectorGenericPhase1(%10d) EXIT ...\n", treeID);
  //  }
  return vectorSize;
}
uint stackAndConstructSplitVectorGenericPhase2 (uint     treeID,
                                                Node    *parent,
                                                uint     covariate,
                                                double  *splitVector,
                                                uint     vectorSize,
                                                char    *factorFlag,
                                                char    *deterministicSplitFlag,
                                                uint    *mwcpSizeAbsolute,
                                                void   **splitVectorPtr) {
  uint repMembrSize;
  uint  sworIndex;
  uint *sworVector;
  uint  sworVectorSize;
  uint j, j2, k2;
  uint factorSizeAbsolute;
  uint offset;
  uint splitLength;
  uint relativePair;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nstackAndConstructSplitVectorGenericPhase2(%10d) ENTRY ...\n", treeID);
  //  }
  repMembrSize = parent -> repMembrSize;
  splitLength = 0;  
  (*splitVectorPtr) = NULL;  
  if (vectorSize < 2) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Split vector is of insufficient size in Stack Phase II allocation:  %10d", vectorSize);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\n(TreeID %10d):  Constructing split vector for (parameter, of size):  (%10d, %10d) \n", treeID, covariate, vectorSize);
  //  }
  if (*factorFlag) {
    if(RF_factorList[treeID][vectorSize] == NULL) {
      RF_factorList[treeID][vectorSize] = makeFactor(vectorSize, FALSE);
    }
    factorSizeAbsolute = RF_xFactorSize[RF_xFactorMap[covariate]];
    *mwcpSizeAbsolute = RF_factorList[treeID][factorSizeAbsolute] -> mwcpSize;
    //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
    //    RF_nativePrint("\n(%10d):  (Absolute Factor Size, Absolute MWCP Size):  (%10d, %10d)", treeID, factorSizeAbsolute, *mwcpSizeAbsolute);
    //    }
    if (RF_splitRule == RAND_SPLIT) {
      splitLength = 1 + 1;
      *deterministicSplitFlag = FALSE;
    }
    else {
      if (RF_nsplit == 0) {
        *deterministicSplitFlag = TRUE;
        if ((RF_factorList[treeID][vectorSize] -> r) > MAX_EXACT_LEVEL) {
          *deterministicSplitFlag = FALSE;
        }
        else {
          if ( *((uint *) RF_factorList[treeID][vectorSize] -> complementaryPairCount) >= repMembrSize ) {
            *deterministicSplitFlag = FALSE;
          }
        }
        if (*deterministicSplitFlag == FALSE) {
          splitLength = repMembrSize + 1;
          //          if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
          //            if (vectorSize <= MAX_EXACT_LEVEL) {
          //              RF_nativePrint("\n(%10d):  Factor override to random (pSplit, nsplit, ndSize):  (%10d, %10d, %10d) \n",
          //                      treeID,
          //                      *((uint*) RF_factorList[treeID][vectorSize] -> complementaryPairCount),
          //                      RF_nsplit,
          //                      repMembrSize);
          //            }
          //            else {
          //              RF_nativePrint("\n(%10d):  Factor override to random (pSplit, nsplit, ndSize):  (%24.0f, %10d, %10d) \n",
          //                      treeID,
          //                      *((double*) RF_factorList[treeID][vectorSize] -> complementaryPairCount),
          //                      RF_nsplit,
          //                      repMembrSize);
          //            }
          //          }
        }
        else {
          splitLength = *((uint*) RF_factorList[treeID][vectorSize] -> complementaryPairCount) + 1;
        }
      }
      else {
        *deterministicSplitFlag = FALSE;
        if ((RF_factorList[treeID][vectorSize] -> r) <= MAX_EXACT_LEVEL) {
          if (*((uint*) RF_factorList[treeID][vectorSize] -> complementaryPairCount) <= ((RF_nsplit <= repMembrSize) ? RF_nsplit : repMembrSize)) {
            splitLength = *((uint*) RF_factorList[treeID][vectorSize] -> complementaryPairCount) + 1;
            *deterministicSplitFlag = TRUE;
            //            if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
            //              RF_nativePrint("\nFactor override to determ (pSplit, nsplit, ndSize):  (%10d, %10d, %10d) \n",
            //                      *((uint*) RF_factorList[treeID][vectorSize] -> complementaryPairCount),
            //                      RF_nsplit,
            //                      repMembrSize);
            //            }
          }
        }
        if (*deterministicSplitFlag == FALSE) {
          splitLength = 1 + ((RF_nsplit <= repMembrSize) ? RF_nsplit : repMembrSize);
        }
      }  
    }  
    (*splitVectorPtr) = uivector(1, splitLength * (*mwcpSizeAbsolute));
    for (offset = 1; offset <= *mwcpSizeAbsolute; offset++) {
      ((uint*) (*splitVectorPtr) + ((splitLength - 1) * (*mwcpSizeAbsolute)))[offset] = 0;
    }
    if (*deterministicSplitFlag) {
      bookFactor(RF_factorList[treeID][vectorSize]);
      j2 = 0;
      for (j = 1; j <= RF_factorList[treeID][vectorSize] -> cardinalGroupCount; j++) {
        for (k2 = 1; k2 <= ((uint*) RF_factorList[treeID][vectorSize] -> cardinalGroupSize)[j]; k2++) {
          ++j2;
          relativePair = (RF_factorList[treeID][vectorSize] -> cardinalGroupBinary)[j][k2];
          convertRelToAbsBinaryPair(treeID,
                                    vectorSize,
                                    factorSizeAbsolute,
                                    relativePair,
                                    splitVector,
                                    (uint*) (*splitVectorPtr) + ((j2 - 1) * (*mwcpSizeAbsolute)));
        }
      }
    }  
    else {
      for (j = 1; j < splitLength; j++) {
        getRandomPair(treeID, vectorSize, factorSizeAbsolute, splitVector, (uint*) (*splitVectorPtr) + ((j - 1) * (*mwcpSizeAbsolute)));
      }
    }
  }  
  else {
    if (RF_splitRule == RAND_SPLIT) {
      splitLength = 1 + 1;
      *deterministicSplitFlag = FALSE;
    }
    else {
      if(RF_nsplit == 0) {
        splitLength = vectorSize;
        (*splitVectorPtr) = splitVector;
        *deterministicSplitFlag = TRUE;
      }
      else {
        if (vectorSize <= RF_nsplit + 1) {
          splitLength = vectorSize;
          (*splitVectorPtr) = splitVector;
          *deterministicSplitFlag = TRUE;
          //          if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
          //            RF_nativePrint("\nContinuous override to determ (pSplit, nsplit, ndSize):  (%10d, %10d, %10d) \n",
          //                    vectorSize,
          //                    RF_nsplit,
          //                    repMembrSize);
          //          }
        }
        else {
          splitLength = RF_nsplit + 1;
          *deterministicSplitFlag = FALSE;
        }
      }  
    }  
    if (*deterministicSplitFlag == FALSE) {
      (*splitVectorPtr) = dvector(1, splitLength);
      ((double*) (*splitVectorPtr))[splitLength] = 0;
      if (RF_splitRule == RAND_SPLIT) {
        ((double*) (*splitVectorPtr))[1]  = splitVector[(uint) ceil(ran1B(treeID) * ((vectorSize - 1) * 1.0))];
      }
      else {
        //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
        //    RF_nativePrint("\nConstructing SWOR of (populated) length:  %10d - 1 \n", splitLength);
        //  }
        sworVector = uivector(1, vectorSize);
        sworVectorSize = vectorSize - 1;
        for (j = 1; j <= sworVectorSize; j++) {
          sworVector[j] = j;
        }
        for (j = 1; j < splitLength; j++) {
          sworIndex = (uint) ceil(ran1B(treeID) * (sworVectorSize * 1.0));
          ((double*) (*splitVectorPtr))[j]  = splitVector[sworVector[sworIndex]];
          sworVector[sworIndex] = sworVector[sworVectorSize];
          sworVectorSize --;
        }
        free_uivector (sworVector, 1, vectorSize);
        qksort(((double*) (*splitVectorPtr)), splitLength-1);
      }
    }
  }  
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nstackAndConstructSplitVectorGenericPhase2(%10d) length:  %10d", treeID, splitLength);
  //  }
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nstackAndConstructSplitVectorGenericPhase2(%10d) EXIT ...\n", treeID);
  //  }
  return splitLength;
}
void unstackSplitVectorGeneric(uint   treeID,
                               Node  *parent,
                               uint   splitLength,
                               char   factorFlag,
                               uint   splitVectorSize,
                               uint   mwcpSizeAbsolute,
                               char   deterministicSplitFlag,
                               void  *splitVectorPtr,
                               char   multImpFlag,
                               uint  *indxx) {
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nunstackSplitVectorGeneric(%10d) ENTRY ...\n", treeID);
  //  }
  if (splitLength > 0) {
    if (factorFlag == TRUE) {
      free_uivector(splitVectorPtr, 1, splitLength * mwcpSizeAbsolute);
      if (deterministicSplitFlag == FALSE) {
        if (splitVectorSize > SAFE_FACTOR_SIZE) {
          unbookFactor(RF_factorList[treeID][splitVectorSize]);
        }
      }
    }
    else {
      if (deterministicSplitFlag == FALSE) {
        free_dvector(splitVectorPtr, 1, splitLength);
      }
    }
    if (indxx != NULL) {
      free_uivector((indxx), 1, parent -> repMembrSize);
    }
    if (!((RF_mRecordSize == 0) || (multImpFlag) || (!(RF_optHigh & OPT_MISS_SKIP)))) {
      free_uivector(parent -> nonMissMembrIndx, 1, parent -> nonMissMembrSizeStatic);
    }
  }
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nunstackSplitVectorGeneric(%10d) EXIT ...\n", treeID);
  //  }
}
uint virtuallySplitNodeGeneric(uint  treeID,
                               Node *parent,
                               char  factorFlag,
                               uint  mwcpSizeAbsolute,
                               double *observation,
                               uint *indxx,
                               void *splitVectorPtr,
                               uint  offset,
                               char *localSplitIndicator,
                               uint *leftSize,
                               uint  priorMembrIter,
                               uint *currentMembrIter) {
  uint *repMembrIndx, *nonMissMembrIndx;
  uint nonMissMembrSize;
  char daughterFlag;
  char iterFlag;
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //  RF_nativePrint("\nvirtuallySplitNodeGeneric(%10d) ENTRY ...\n", treeID);
  //  }
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    if (factorFlag == TRUE) {
  //      RF_nativePrint("\nVirtually splitting on vector element with factor (index, binary words (hex)):  ");
  //      RF_nativePrint("( %10d, ", offset);
  //      for (uint k = mwcpSizeAbsolute; k >= 1; k--) {
  //        RF_nativePrint("%8x", ((uint*) splitVectorPtr + ((offset - 1) * mwcpSizeAbsolute))[k]);
  //      }
  //      RF_nativePrint(")");
  //    }
  //    else {
  //      RF_nativePrint("\nVirtually splitting on vector element with real (index, value):  ( %10d, %12.4f) \n", offset, ((double*) splitVectorPtr)[offset]);
  //    }
  //  }
  iterFlag = TRUE;
  repMembrIndx = parent -> repMembrIndx;
  nonMissMembrIndx = parent -> nonMissMembrIndx;
  nonMissMembrSize = parent -> nonMissMembrSize;
  *currentMembrIter = priorMembrIter;
  while (iterFlag) {
    (*currentMembrIter) ++;
    //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
    //        if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //          RF_nativePrint("\nCurrent   (indx):  %10d ", *currentMembrIter);
    //          if (factorFlag != TRUE) {
    //            RF_nativePrint("\nCurrent  (indxx):  %10d ", indxx[*currentMembrIter]);
    //            RF_nativePrint("\nCurrent (nmmIdx):  %10d ", nonMissMembrIndx[indxx[*currentMembrIter]]);
    //          }
    //          else {    
    //            RF_nativePrint("\nCurrent (nmmIdx):  %10d ", nonMissMembrIndx[*currentMembrIter]);
    //          }
    //        }
    //      }
    if (factorFlag == TRUE) {
      daughterFlag = splitOnFactor((uint)  observation[    repMembrIndx[nonMissMembrIndx[*currentMembrIter]]     ],
                                   (uint*) splitVectorPtr + ((offset - 1) * mwcpSizeAbsolute));
      localSplitIndicator[     nonMissMembrIndx[*currentMembrIter]   ] = daughterFlag;
      if ((*currentMembrIter) == nonMissMembrSize) {
        iterFlag = FALSE;
      }
      //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
      //        if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
      //          if (daughterFlag == LEFT) {
      //            RF_nativePrint("\nMember of LEFT Daughter (index):  %10d %10d %10d %12.4f", *currentMembrIter, nonMissMembrIndx[*currentMembrIter], repMembrIndx[nonMissMembrIndx[*currentMembrIter]], observation[ repMembrIndx[nonMissMembrIndx[*currentMembrIter]] ]);
      //          }
      //          else {
      //            RF_nativePrint("\nMember of RGHT Daughter (index):  %10d %10d %10d %12.4f", *currentMembrIter, nonMissMembrIndx[*currentMembrIter], repMembrIndx[nonMissMembrIndx[*currentMembrIter]], observation[ repMembrIndx[nonMissMembrIndx[*currentMembrIter]] ]);
      //          }
      //        }
      //      }
    }
    else {
      if ((((double*) splitVectorPtr)[offset] - observation[   repMembrIndx[nonMissMembrIndx[indxx[*currentMembrIter]]]    ]) >= 0.0) {
        daughterFlag = LEFT;
      }
      else {
        daughterFlag = RIGHT;
        iterFlag = FALSE;
      }
      localSplitIndicator[     nonMissMembrIndx[indxx[*currentMembrIter]]   ] = daughterFlag;
      //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
      //        if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
      //          if (daughterFlag == LEFT) {
      //            RF_nativePrint("\nMember of LEFT Daughter (index):  %10d %10d %10d %10d %12.4f", *currentMembrIter, indxx[*currentMembrIter], nonMissMembrIndx[indxx[*currentMembrIter]], repMembrIndx[nonMissMembrIndx[indxx[*currentMembrIter]]], observation[ repMembrIndx[nonMissMembrIndx[indxx[*currentMembrIter]]] ]);
      //          }
      //          else {
      //            RF_nativePrint("\nMember of RGHT Daughter (index):  %10d %10d %10d %10d %12.4f", *currentMembrIter, indxx[*currentMembrIter], nonMissMembrIndx[indxx[*currentMembrIter]], repMembrIndx[nonMissMembrIndx[indxx[*currentMembrIter]]], observation[ repMembrIndx[nonMissMembrIndx[indxx[*currentMembrIter]]] ]);
      //          }
      //        }
      //      }
    }
    if (daughterFlag == LEFT) {
      (*leftSize) ++;
    }
  }  
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nLeft Size:  %10d, Right Size:  %10d", *leftSize, nonMissMembrSize - *leftSize);
  //  }
  if ((*leftSize == 0) || (*leftSize == nonMissMembrSize)) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Left or Right Daughter of size zero:  (%10d, %10d)", *leftSize, nonMissMembrSize);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  //  if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
  //    RF_nativePrint("\nvirtuallySplitNodeGeneric(%10d) EXIT ...\n", treeID);
  //  }
  return (*leftSize);
}
char summarizeSplitResult(SplitInfoMax *splitInfoMax) {
  char result;
  //  uint k;
  //  if (getTraceFlag(0) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nsummarizeSplitResult() ENTRY ...\n");
  //  }
  if (!RF_nativeIsNaN(splitInfoMax -> deltaMax)) {
    splitInfoMax -> splitStatistic = splitInfoMax -> deltaMax;
    result = TRUE;
    //    if (getTraceFlag(0) & SPLT_MED_TRACE) {
    //      RF_nativePrint("\nBest Split Statistics: \n");
    //      RF_nativePrint("  SplitParm        Delta \n");
    //      RF_nativePrint(" %10d %12.4f \n", splitInfoMax -> splitParameterMax, splitInfoMax -> deltaMax);
    //      if (RF_xType[splitInfoMax -> splitParameterMax] == 'C') {
    //        RF_nativePrint(" at MWCPsize= %2d, mwcp= ", splitInfoMax -> splitValueMaxFactSize);
    //        for (k = splitInfoMax -> splitValueMaxFactSize; k >= 1; k--) {
    //          RF_nativePrint("%8x ", splitInfoMax -> splitValueMaxFactPtr[k]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //      else {
    //        RF_nativePrint(" at %12.4f \n", splitInfoMax -> splitValueMaxCont);
    //      }
    //    }
  }
  else {
    result = FALSE;
  }
  //  if (getTraceFlag(0) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nsummarizeSplitResult(%1d) EXIT ...\n", result);
  //  }
  return result;
}
char updateMaximumSplitGeneric(uint    treeID,
                               Node   *parent,
                               double  delta,
                               uint    covariate,
                               uint    index,
                               char    factorFlag,
                               uint    mwcpSizeAbsolute,
                               uint    repMembrSize,
                               char  **polarity,
                               void   *splitVectorPtr,
                               SplitInfoMax *splitInfoMax) {
  char flag;
  uint k;
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nupdateMaximumSplitGeneric() ENTRY ...\n");
  //  }
  if(RF_nativeIsNaN(delta)) {
    flag = FALSE;
  }
  else {
    delta = delta * RF_xWeightStat[covariate];
    if(RF_nativeIsNaN(splitInfoMax -> deltaMax)) {
      flag = TRUE;
    }
    else {
      if ((delta - (splitInfoMax -> deltaMax)) > EPSILON) {
        flag = TRUE;
      }
      else {
        flag = FALSE;
      }
    }
  }
  if (flag) {
    splitInfoMax -> deltaMax = delta;
    splitInfoMax -> splitParameterMax = covariate;
    //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
    //      RF_nativePrint("\n\nRunning Split Statistics Updated: \n");
    //      RF_nativePrint("  SplitParm  SplitValIdx        Delta \n");
    //      RF_nativePrint(" %10d %12d %12.4f \n", covariate, index, splitInfoMax -> deltaMax);
    //    }
      //      if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
      //        RF_nativePrint(" with nominal x-var");
      //      }
    if (factorFlag == TRUE) {
      if (splitInfoMax -> splitValueMaxFactSize > 0) {
        if (splitInfoMax -> splitValueMaxFactSize != mwcpSizeAbsolute) {
          free_uivector(splitInfoMax -> splitValueMaxFactPtr, 1, splitInfoMax -> splitValueMaxFactSize);
          splitInfoMax -> splitValueMaxFactSize = mwcpSizeAbsolute;
          splitInfoMax -> splitValueMaxFactPtr = uivector(1, splitInfoMax -> splitValueMaxFactSize);
        }
      }
      else {
        splitInfoMax -> splitValueMaxFactSize = mwcpSizeAbsolute;
        splitInfoMax -> splitValueMaxFactPtr = uivector(1, splitInfoMax -> splitValueMaxFactSize);
      }
      splitInfoMax -> splitValueMaxCont = RF_nativeNaN;
      for (k=1; k <= splitInfoMax -> splitValueMaxFactSize; k++) {
        (splitInfoMax -> splitValueMaxFactPtr)[k] =
          ((uint*) splitVectorPtr + ((index - 1) * (splitInfoMax -> splitValueMaxFactSize)))[k];
      }
      //      if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
      //        RF_nativePrint(" at MWCPsize= %2d, mwcp= ", splitInfoMax -> splitValueMaxFactSize);
      //        for (k = splitInfoMax -> splitValueMaxFactSize; k >= 1; k--) {
      //          RF_nativePrint("%8x ", (splitInfoMax -> splitValueMaxFactPtr)[k]);
      //        }
      //        RF_nativePrint("\n");
      //      }
    }
    else {
      if (splitInfoMax -> splitValueMaxFactSize > 0) {
        free_uivector(splitInfoMax -> splitValueMaxFactPtr, 1, splitInfoMax -> splitValueMaxFactSize);
        splitInfoMax -> splitValueMaxFactSize = 0;
        splitInfoMax -> splitValueMaxFactPtr = NULL;
      }
      else {
      }
      splitInfoMax -> splitValueMaxCont = ((double*) splitVectorPtr)[index];
      //      if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
      //        RF_nativePrint(" at continuous point %12.4f \n", splitInfoMax -> splitValueMaxCont);
      //      }
    }
  }
  else {
    //    if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
    //      RF_nativePrint("\n\nRunning Split Statistics NOT Updated: \n");
    //      RF_nativePrint("  SplitParm  SplitValIdx        Delta     DeltaMax\n");
    //      RF_nativePrint(" %10d %12d %12.4f %12.4f\n", covariate, index, delta, splitInfoMax -> deltaMax);
    //    }
  }
  //  if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
  //    RF_nativePrint("\nupdateMaximumSplitGeneric() EXIT ...\n");
  //  }
  return flag;
}
void getReweightedRandomPair (uint    treeID,
                              uint    relativeFactorSize,
                              uint    absoluteFactorSize,
                              double *absoluteLevel,
                              uint   *result) {
  uint randomGroupIndex;
  //  if (getTraceFlag(treeID) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\ngetReweightedRandomPair() ENTRY ...\n");
  //  }
  if(RF_factorList[treeID][relativeFactorSize] == NULL) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Factor not allocated for size:  %10d", relativeFactorSize);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  randomGroupIndex = (uint) ceil(ran1B(treeID) * ((RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount) * 1.0));
  //  if (getTraceFlag(treeID) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nRandomly Selected Group Index:  %10d", randomGroupIndex);
  //  }
  createRandomBinaryPair(treeID, relativeFactorSize, absoluteFactorSize, randomGroupIndex, absoluteLevel, result);
  //  if (getTraceFlag(treeID) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\ngetReweightedRandomPair() EXIT ...\n");
  //  }
}
void getRandomPair (uint treeID, uint relativeFactorSize, uint absoluteFactorSize, double *absoluteLevel, uint *result) {
  uint randomGroupIndex;
  double randomValue;
  uint k;
  //  if (getTraceFlag(treeID) & FACT_LOW_TRACE) {
  //    RF_nativePrint("\ngetRandomPair() ENTRY ...\n");
  //  }
  if(RF_factorList[treeID][relativeFactorSize] == NULL) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Factor not allocated for size:  %10d", relativeFactorSize);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  double *cdf = dvector(1, RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount);
  if (relativeFactorSize <= MAX_EXACT_LEVEL) {
    for (k=1; k <= RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount; k++) {
      cdf[k] = (double) ((uint*) RF_factorList[treeID][relativeFactorSize] -> cardinalGroupSize)[k];
    }
  }
  else {
    for (k=1; k <= RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount; k++) {
      cdf[k] = ((double*) RF_factorList[treeID][relativeFactorSize] -> cardinalGroupSize)[k];
    }
  }
  //  if (getTraceFlag(treeID) & FACT_LOW_TRACE) {
  //    RF_nativePrint("\nFactor (relativeFactorSize, cardinalGroupCount):  (%10d, %10d) \n", relativeFactorSize, RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount);
  //  }
  for (k=2; k <= RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount; k++) {
    cdf[k] += cdf[k-1];
  }
  //  if (getTraceFlag(treeID) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nUpdated CDF based on cardinal group size:  ");
  //    RF_nativePrint("\n     index          cdf");
  //    for (k=1; k <= RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount; k++) {
  //      RF_nativePrint("\n%10d  %12f", k, cdf[k]);
  //    }
  //    RF_nativePrint("\n");
  //  }
  randomValue = ceil((ran1B(treeID) * cdf[RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount]));
  randomGroupIndex = 1;
  while (randomValue > cdf[randomGroupIndex]) {
    randomGroupIndex ++;
  }
  free_dvector(cdf, 1, RF_factorList[treeID][relativeFactorSize] -> cardinalGroupCount);
  //  if (getTraceFlag(treeID) & FACT_LOW_TRACE) {
  //    RF_nativePrint("\nRandomly Selected Group Index:  %10d", randomGroupIndex);
  //  }
  createRandomBinaryPair(treeID, relativeFactorSize, absoluteFactorSize, randomGroupIndex, absoluteLevel, result);
  //  if (getTraceFlag(treeID) & FACT_LOW_TRACE) {
  //    RF_nativePrint("\ngetRandomPair() EXIT ...\n");
  //  }
}
void createRandomBinaryPair(uint    treeID,
                            uint    relativeFactorSize,
                            uint    absoluteFactorSize,
                            uint    groupIndex,
                            double *absoluteLevel,
                            uint   *pair) {
  uint mwcpLevelIdentifier;
  uint mwcpSizeAbsolute;
  uint offset, levelSize, levelIndex;
  uint k;
  //  if (getTraceFlag(treeID) & FACT_LOW_TRACE) {
  //    RF_nativePrint("\ncreateRandomBinaryPair() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(treeID) & FACT_LOW_TRACE) {
  //    RF_nativePrint("\nrelativeSize absoluteSize   groupIndex ");
  //    RF_nativePrint("\n%12d %12d %12d", relativeFactorSize, absoluteFactorSize, groupIndex);
  //    RF_nativePrint("\n");
  //  }
  levelIndex = 0;  
  mwcpSizeAbsolute = RF_factorList[treeID][absoluteFactorSize] -> mwcpSize;
  uint *levelVector = uivector(1, relativeFactorSize);
  uint *randomLevel = uivector(1, groupIndex);
  for (k = 1; k <= relativeFactorSize; k++) {
    levelVector[k] = k;
  }
  levelSize = relativeFactorSize;
  for (k = 1; k <= groupIndex; k++) {
    randomLevel[k] = sampleUniformlyFromVector(treeID,
                                               levelVector,
                                               levelSize,
                                               &levelIndex);
    levelVector[levelIndex] = levelVector[levelSize];
    levelSize --;
  }
  //  if (getTraceFlag(treeID) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nAbsolute Levels:  ");
  //    RF_nativePrint("\n     index      level");
  //    for (k=1; k <= relativeFactorSize; k++) {
  //      RF_nativePrint("\n%10d  %10d", k, (uint) absoluteLevel[k]);
  //    }
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\nRandomly Selected Levels Prior to Remapping:  ");
  //    RF_nativePrint("\n     index      level");
  //    for (k=1; k <= groupIndex; k++) {
  //      RF_nativePrint("\n%10d  %10d", k, randomLevel[k]);
  //    }
  //    RF_nativePrint("\n");
  //  }
  for (k = 1; k <= groupIndex; k++) {
    randomLevel[k] = (uint) absoluteLevel[randomLevel[k]];
  }
  //  if (getTraceFlag(treeID) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nRandomly Selected Levels After Remapping:  ");
  //    RF_nativePrint("\n     index      level");
  //    for (k=1; k <= groupIndex; k++) {
  //      RF_nativePrint("\n%10d  %10d", k, randomLevel[k]);
  //    }
  //    RF_nativePrint("\n");
  //  }
  for (offset = 1; offset <= mwcpSizeAbsolute; offset++) {
    pair[offset] = 0;
  }
  for (k = 1; k <= groupIndex; k++) {
    mwcpLevelIdentifier = (randomLevel[k] >> (3 + ulog2(sizeof(uint)))) + ((randomLevel[k] & (MAX_EXACT_LEVEL - 1)) ? 1 : 0);
    //    if (getTraceFlag(treeID) & FACT_HGH_TRACE) {
    //      RF_nativePrint("\n MWCP Level Identifier:   %10d ", mwcpLevelIdentifier);
    //      RF_nativePrint("\n upower() bit:  %10d ", randomLevel[k] - ((mwcpLevelIdentifier - 1) * MAX_EXACT_LEVEL) - 1 );
    //    }
    pair[mwcpLevelIdentifier] += upower(2, randomLevel[k] - ((mwcpLevelIdentifier - 1) * MAX_EXACT_LEVEL) - 1 );
  }
  free_uivector(levelVector, 1, relativeFactorSize);
  free_uivector(randomLevel, 1, groupIndex);
  //  if (getTraceFlag(treeID) & FACT_LOW_TRACE) {
  //    RF_nativePrint("\ncreateRandomBinaryPair() EXIT ...\n");
  //  }
}
void convertRelToAbsBinaryPair(uint    treeID,
                               uint    relativeFactorSize,
                               uint    absoluteFactorSize,
                               uint    relativePair,
                               double *absoluteLevel,
                               uint   *pair) {
  uint mwcpLevelIdentifier;
  uint mwcpSizeAbsolute;
  uint coercedAbsoluteLevel;
  uint k, offset;
  //  if (getTraceFlag(0) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nconvertRelToAbsBinaryPair() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(0) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nrelativeSize absoluteSize      relPair");
  //    RF_nativePrint("\n%12d %12d %12x", relativeFactorSize, absoluteFactorSize, relativePair);
  //    RF_nativePrint("\n");
  //  }
  mwcpSizeAbsolute = RF_factorList[treeID][absoluteFactorSize] -> mwcpSize;
  //  if (getTraceFlag(0) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nAbsolute Levels:  ");
  //    RF_nativePrint("\n     index      level");
  //    for (k=1; k <= relativeFactorSize; k++) {
  //      RF_nativePrint("\n%10d  %10d", k, (uint) absoluteLevel[k]);
  //    }
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\nRelative Pair:  %8x \n", relativePair);
  //  }
  for (offset = 1; offset <= mwcpSizeAbsolute; offset++) {
    pair[offset] = 0;
  }
  for (k = 1; k <= relativeFactorSize; k++) {
    if (relativePair & ((uint) 0x01)) {
      coercedAbsoluteLevel = (uint) absoluteLevel[k];
      mwcpLevelIdentifier = (coercedAbsoluteLevel >> (3 + ulog2(sizeof(uint)))) + ((coercedAbsoluteLevel & (MAX_EXACT_LEVEL - 1)) ? 1 : 0);
      pair[mwcpLevelIdentifier] += upower(2, coercedAbsoluteLevel - ((mwcpLevelIdentifier - 1) * MAX_EXACT_LEVEL) - 1 );
      //      if (getTraceFlag(0) & FACT_HGH_TRACE) {
      //        RF_nativePrint("\n MWCP Level Identifier:   %10d ", mwcpLevelIdentifier);
      //        RF_nativePrint("\n upower() bit:  %10d ", coercedAbsoluteLevel - ((mwcpLevelIdentifier - 1) * MAX_EXACT_LEVEL) - 1);
      //      }
    }
    relativePair = relativePair >> 1;
  }
  //  if (getTraceFlag(0) & FACT_HGH_TRACE) {
  //    RF_nativePrint("\nconvertRelToAbsBinaryPair() EXIT ...\n");
  //  }
}
