
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "classification.h"
#include "termOps.h"
#include "nrutil.h"
#include "error.h"
void getMultiClassProb (uint       treeID,
                        Terminal  *parent,
                        uint      *repMembrIndx,
                        uint       repMembrSize,
                        uint      *allMembrIndx,
                        uint       allMembrSize,
                        uint      *rmbrIterator) {
  uint *membershipIndex;
  uint  membershipSize;
  double maxValue, maxClass;
  uint i, j, k;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetMultiClassProb() ENTRY ...\n");
  //  }
  if ( !(RF_opt & OPT_BOOT_TYP1) && (RF_opt & OPT_BOOT_TYP2) ) {
    membershipIndex = allMembrIndx;
    membershipSize = parent -> membrCount = allMembrSize;
    if (RF_optHigh & OPT_MEMB_INCG) {
      membershipIndex = RF_AMBR_ID_ptr[treeID];
    }
  }
  else {
    membershipIndex = repMembrIndx;
    membershipSize = parent -> membrCount = repMembrSize;
    if (RF_optHigh & OPT_MEMB_INCG) {
      membershipIndex = RF_RMBR_ID_ptr[treeID];
    }
  }
  if (membershipSize == 0) {
    if (!(RF_opt & OPT_OUTC_TYPE)) {
      RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      RF_nativeError("\nRF-SRC:  Zero node count encountered in (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
      RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      RF_nativeExit();
    }
  }
  if (!(RF_optHigh & OPT_TERM_INCG)) {
    stackMultiClassProb(parent, RF_rFactorCount, RF_rFactorSize);
    for (j=1; j <= RF_rFactorCount; j++) {
      for (k=1; k <= RF_rFactorSize[j]; k++) {
        (parent -> multiClassProb)[j][k] = 0;
      }
    }
    if (RF_optHigh & OPT_MEMB_OUTG) {
      for (i = 1; i <= membershipSize; i++) {
        RF_RMBR_ID_ptr[treeID][++(*rmbrIterator)] = membershipIndex[i];
        for (j=1; j <= RF_rFactorCount; j++) {
          (parent -> multiClassProb)[j][(uint) RF_response[treeID][RF_rFactorIndex[j]][membershipIndex[i]]] ++;
        }
      }
    }
    else if (RF_optHigh & OPT_MEMB_INCG) {
      for (i = 1; i <= membershipSize; i++) {
        ++(*rmbrIterator);
        for (j=1; j <= RF_rFactorCount; j++) {
          (parent -> multiClassProb)[j][(uint) RF_response[treeID][RF_rFactorIndex[j]][ membershipIndex[*rmbrIterator] ]] ++;
        }
      }
    }
    else {
      for (i = 1; i <= membershipSize; i++) {
        for (j=1; j <= RF_rFactorCount; j++) {
          (parent -> multiClassProb)[j][(uint) RF_response[treeID][RF_rFactorIndex[j]][membershipIndex[i]]] ++;
        }
      }
    }
    for (j = 1; j <= RF_rFactorCount; j++) {
      maxValue = 0;
      maxClass = 0;
      for (k=1; k <= RF_rFactorSize[j]; k++) {
        if (maxValue < (double) (parent -> multiClassProb[j][k])) {
          maxValue = (double) parent -> multiClassProb[j][k];
          maxClass = (double) k;
        }
      }
      (parent -> maxClass)[j] = maxClass;
    }
  }
  else {
    stackMultiClassProb(parent, RF_rFactorCount, RF_rFactorSize);
    for (j = 1; j <= RF_rFactorCount; j++) {
      for (k=1; k <= RF_rFactorSize[j]; k++) {      
        (parent -> multiClassProb)[j][k] = RF_TN_CLAS_ptr[treeID][parent -> nodeID][j][k];      
      }
    }
    for (j = 1; j <= RF_rFactorCount; j++) {
      maxValue = 0;
      maxClass = 0;
      for (k=1; k <= RF_rFactorSize[j]; k++) {
        if (maxValue < (double) (parent -> multiClassProb[j][k])) {
          maxValue = (double) parent -> multiClassProb[j][k];
          maxClass = (double) k;
        }
      }
      (parent -> maxClass)[j] = maxClass;
    }
  }
  //  if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //    if (getTraceFlag(treeID) & ENSB_LOW_TRACE) {
  //    RF_nativePrint("\nClass proportion vector for (tree, leaf):  (%10d, %10d) \n", treeID, parent -> nodeID);
  //    for (j=1; j <= RF_rFactorCount; j++) {
  //      RF_nativePrint("\nFactor Index:  %10d", RF_rFactorIndex[j]);
  //      RF_nativePrint("\nMember Count:  %10d", parent -> membrCount);
  //      RF_nativePrint("\nClass ->  ");
  //      for (k=1; k <= RF_rFactorSize[j]; k++) {
  //        RF_nativePrint("%10d", k);
  //      }
  //      RF_nativePrint("\n");
  //      RF_nativePrint("%10d", j);
  //      for (k=1; k <= RF_rFactorSize[j]; k++) {
  //        RF_nativePrint("%10d", (parent -> multiClassProb)[j][k]);
  //      }
  //    }
  //  }
  //  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetMultiClassProb() EXIT ...\n");
  //  }
}
void updateEnsembleMultiClass(char      mode,
                              uint      treeID,
                              char      normalizationFlag,
                              char      omitDenominator) {
  char oobFlag, fullFlag, outcomeFlag;
  Terminal ***termMembershipPtr;
  uint    *membershipIndex;
  uint     membershipSize;
  double   ***ensembleCLSptr;
  double   ***ensembleCLSnum;
  double     *ensembleDen;
#ifdef _OPENMP
  omp_lock_t   *lockDENptr;
#endif
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nupdateEnsembleMultiClass() ENTRY ...\n");
  //  }
  ensembleCLSnum = NULL;  
  ensembleDen    = NULL;  
  oobFlag = fullFlag = FALSE;
  switch (mode) {
  case RF_PRED:
    if (RF_opt & OPT_FENS) {
      fullFlag = TRUE;
    }
    termMembershipPtr = RF_ftTermMembership;
    break;
  default:
    if (RF_opt & OPT_OENS) {
      if (RF_oobSize[treeID] > 0) {
        oobFlag = TRUE;
      }
    }
    if (RF_opt & OPT_FENS) {
      fullFlag = TRUE;
    }
    termMembershipPtr = RF_tTermMembership;
    break;
  }
  outcomeFlag = TRUE;
  while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
    if (oobFlag == TRUE) {
      ensembleCLSptr = RF_oobEnsembleCLSptr;
      ensembleCLSnum = RF_oobEnsembleCLSnum;
      ensembleDen    = RF_oobEnsembleDen;
      membershipSize  = RF_oobSize[treeID];
      membershipIndex = RF_oobMembershipIndex[treeID];
#ifdef _OPENMP
      lockDENptr      = RF_lockDENoens;
#endif
    }
    else {
      ensembleCLSptr = RF_fullEnsembleCLSptr;
      ensembleCLSnum = RF_fullEnsembleCLSnum;
      ensembleDen    = RF_fullEnsembleDen;
      switch (mode) {
      case RF_PRED:
        membershipSize = RF_fobservationSize;
        membershipIndex = RF_fidentityMembershipIndex;
        break;
      default:
        membershipSize  = RF_observationSize;
        membershipIndex = RF_identityMembershipIndex;
        break;
      }
#ifdef _OPENMP
      lockDENptr      = RF_lockDENfens;
#endif
    }
    for (uint i = 1; i <= membershipSize; i++) {
      Terminal *parent;
      char selectionFlag;
      uint j, k, ii;
      ii = membershipIndex[i];
      parent = termMembershipPtr[treeID][ii];
      selectionFlag = TRUE;
      if (RF_opt & OPT_OUTC_TYPE) {
        if ((parent -> membrCount) > 0) {
        }
        else {
          //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
          //    RF_nativePrint("\nCLAS OUTC_TYPE case no predicted value:  %10d \n", ii);
          //  }
          selectionFlag = FALSE;
        }
      }
      if (selectionFlag) {
#ifdef _OPENMP        
        omp_set_lock(&(lockDENptr[ii]));
#endif
        if(!omitDenominator) {
          ensembleDen[ii] ++;          
          if (outcomeFlag == TRUE) {
            if (RF_optHigh & OPT_CSE) {              
              RF_cseDENptr[ii] ++;
            }
            if (RF_opt & OPT_VIMP) {
              RF_blkEnsembleDen[ii] ++;
            }
          }
        }
        for (j = 1; j <= RF_rTargetFactorCount; j++) {
          for (k = 1; k <= RF_rFactorSize[RF_rFactorMap[RF_rTargetFactor[j]]]; k++) {
            ensembleCLSnum[j][k][ii] += (double) (parent -> multiClassProb)[RF_rFactorMap[RF_rTargetFactor[j]]][k] / (double) (parent -> membrCount);
          }
        }
        if (outcomeFlag == TRUE) {
          if (RF_optHigh & OPT_CSE) {              
            for (j = 1; j <= RF_rTargetFactorCount; j++) {
              RF_cseNumCLSptr[j][ii] +=
                ( (parent -> maxClass)[RF_rFactorMap[RF_rTargetFactor[j]]] ==
                  (uint) RF_response[treeID][RF_rFactorMap[RF_rTargetFactor[j]]][ii] ) ? 1 : 0;
            }
          }
          if (RF_opt & OPT_VIMP) {
            for (j = 1; j <= RF_rTargetFactorCount; j++) {
              for (k = 1; k <= RF_rFactorSize[RF_rFactorMap[RF_rTargetFactor[j]]]; k++) {
                RF_blkEnsembleCLSnum[j][k][ii] += (double) (parent -> multiClassProb)[RF_rFactorMap[RF_rTargetFactor[j]]][k] / (double) (parent -> membrCount);
              }
            }
          }
        }
        if (outcomeFlag && normalizationFlag) {
          for (j = 1; j <= RF_rTargetFactorCount; j++) {
            for (k = 1; k <= RF_rFactorSize[RF_rFactorMap[RF_rTargetFactor[j]]]; k++) {
              ensembleCLSptr[j][k][ii] = ensembleCLSnum[j][k][ii] / ensembleDen[ii];
            }
          }
        }
#ifdef _OPENMP
        omp_unset_lock(&(lockDENptr[ii]));
#endif
      }
    }  
    //  if (getTraceFlag(treeID) & SUMM_HGH_TRACE) {
    //    uint obsSize = (mode == RF_PRED) ? RF_fobservationSize : RF_observationSize;
    //    if (oobFlag == TRUE) {
    //      RF_nativePrint("\nOOB Ensemble calculations follow: \n");
    //    }
    //    else {
    //      if (fullFlag == TRUE) {
    //        RF_nativePrint("\nFULL Ensemble calculations follow: \n");
    //      }
    //    }
    //    RF_nativePrint("\nCLAS Numerator calculation: \n");
    //    RF_nativePrint("          ");
    //    for (uint i = 1; i <= obsSize; i++) {
    //      RF_nativePrint("%10d", i);
    //    }
    //    for (uint j = 1; j <= RF_rTargetFactorCount; j++) {
    //      RF_nativePrint("\n  Target Factor (idx, rsp): (%10d, %10d)", j, RF_rTargetFactor[j]);
    //      for (uint k = 1; k <= RF_rFactorSize[RF_rFactorMap[RF_rTargetFactor[j]]]; k++) {
    //        RF_nativePrint("\n%10d", k);
    //        for (uint i  = 1; i <= obsSize; i++) {
    //          RF_nativePrint("%10.4f", ensembleCLSnum[j][k][i]);
    //        }
    //      }
    //      RF_nativePrint("\n");
    //    }
    //    RF_nativePrint("\nCLAS Denominator calculation: \n");
    //    RF_nativePrint("          ");
    //    for (uint i = 1; i <= obsSize; i++) {
    //      RF_nativePrint("%10d", i);
    //    }
    //    RF_nativePrint("\n          ");
    //    for (uint i = 1; i <= obsSize; i++) {
    //      RF_nativePrint("%10d", (uint) ensembleDen[i]);
    //    }
    //    RF_nativePrint("\n");
    //    if (outcomeFlag == TRUE) {
    //      if (RF_optHigh & OPT_CSE) {
    //        RF_nativePrint("\nCLAS CSE Denominator calculation: \n");
    //        RF_nativePrint("                    ");
    //        for (uint i = 1; i <= obsSize; i++) {
    //          RF_nativePrint("%10d", i);
    //        }
    //        RF_nativePrint("\n                    ");
    //        for (uint i = 1; i <= obsSize; i++) {
    //          RF_nativePrint("%10d", RF_cseDENptr[i]);
    //        }
    //        RF_nativePrint("\nCLAS CSE Numerator calculation: \n");
    //        for (uint j = 1; j <= RF_rTargetFactorCount; j++) {
    //          RF_nativePrint("\n");
    //          RF_nativePrint("%20d", j);
    //          for (uint i = 1; i <= obsSize; i++) {
    //            RF_nativePrint("%10.4f", RF_cseNumCLSptr[j][i]);
    //          }
    //        }
    //        RF_nativePrint("\n");
    //      }
    //    }
    //  }
    if (outcomeFlag == TRUE) {
      outcomeFlag = FALSE;
    }
    if (oobFlag == TRUE) {
      oobFlag = FALSE;
    }
    else {
      fullFlag = FALSE;
    }
  }  
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nupdateEnsembleMultiClass() EXIT ...\n");
  //  }
}
double getBrierScore(uint     obsSize,
                     uint     rTarget,
                     double  *responsePtr,
                     double **outcomeCLS,
                     double  *denomCount,
                     double  *cpv) {
  uint k;
  uint against;
  uint *oaaResponse;
  uint cumDenomCount;
  double result;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetBrierScore() ENTRY ...\n");
  //  }
  //  uint j;
  oaaResponse = uivector(1, obsSize);
  result = 0.0;
  cumDenomCount = 0;
  for (k = 1; k <= obsSize; k ++) {
    if (denomCount[k] != 0) {
      cumDenomCount += 1;
    }
  }
  for (against = 1; against <= RF_rFactorSize[RF_rFactorMap[rTarget]]; against++) {
    for (k = 1; k <= obsSize; k ++) {
      if ((uint) responsePtr[k] == against) {
        oaaResponse[k] = 1;
      }
      else {
        oaaResponse[k] = 0;
      }
    }
    cpv[against] = 0.0;
    for (k = 1; k <= obsSize; k ++) {
      if (denomCount[k] != 0) {
        cpv[against] += pow(((double) oaaResponse[k] - outcomeCLS[against][k]), 2.0);
      }
    }
    if (cumDenomCount == 0) {
      cpv[against] = RF_nativeNaN;
    }
    else {
      cpv[against] = cpv[against] / (double) cumDenomCount;
      result += cpv[against];
    }
    //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
    //    
    //    RF_nativePrint("\nBrier Score Conditional Classification complete for level:  %10d ", against);
    //    RF_nativePrint("\nLevel     ");
    //    for (j=1; j <= RF_rFactorSize[RF_rFactorMap[rTarget]]; j++) {
    //      RF_nativePrint(" %10d", j);
    //    }
    //    RF_nativePrint("\n          ");
    //    for (j=1; j <= RF_rFactorSize[RF_rFactorMap[rTarget]]; j++) {
    //      RF_nativePrint(" %10.4f", cpv[j]);
    //    }
    //    RF_nativePrint("\n");
    //  }
  }
  if (cumDenomCount == 0) {
    result = RF_nativeNaN;
  }
  else {
    result = result  * RF_rFactorSize[RF_rFactorMap[rTarget]] / (RF_rFactorSize[RF_rFactorMap[rTarget]] - 1);
  }
  for (against = 1; against <= RF_rFactorSize[RF_rFactorMap[rTarget]]; against++) {
    cpv[against] = cpv[against] * RF_rFactorSize[RF_rFactorMap[rTarget]] / (RF_rFactorSize[RF_rFactorMap[rTarget]] - 1);
  }
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    
  //    RF_nativePrint("\nBrier Score calculation complete:");
  //    RF_nativePrint("\nResult:                          %20.4f", result);
  //    RF_nativePrint("\n");
  //  }
  free_uivector(oaaResponse, 1, obsSize);
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetBrierScore() EXIT() ...\n");
  //  }
  return result;
}
void getConditionalClassificationIndexGrow(uint     size,
                                       uint     rTarget,
                                       double  *responsePtr,
                                       double **outcomeCLS,
                                       double  *maxVote,
                                       double  *denomCount,
                                       double  *cpv) {
  uint i, k;
  uint cumDenomCount;
  uint *condClassificationCount;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetConditionalClassificationIndexGrow() ENTRY ...\n");
  //  }
  cumDenomCount = 0;
  condClassificationCount = uivector(1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
    cpv[k] = condClassificationCount[k] = 0;
  }
  for (i = 1; i <= size; i++) {
    condClassificationCount[(uint) responsePtr[i]] ++;
    if (denomCount[i] != 0) {
      cumDenomCount += 1;
      if (responsePtr[i] == maxVote[i]) {
        cpv[(uint) responsePtr[i]] += 1.0;
      }
    }
  }  
  if (cumDenomCount == 0) {
    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
      cpv[k] = RF_nativeNaN;
    }
  }
  else {
    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
      if (condClassificationCount[k] != 0) {
        cpv[k] = 1.0 - cpv[k] / (double) condClassificationCount[k];
      }
      else {
        cpv[k] = RF_nativeNaN;
      }
    }
  }
  free_uivector(condClassificationCount, 1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    
  //    RF_nativePrint("\nConditional Classification and error update complete:  ");
  //    RF_nativePrint("\nTarget:  %10d ", rTarget);
  //    RF_nativePrint("\nLevel     ");
  //    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
  //      RF_nativePrint(" %10d", k);
  //    }
  //    RF_nativePrint("\n          ");
  //    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
  //      RF_nativePrint(" %10.4f", cpv[k]);
  //    }
  //    RF_nativePrint("\n");
  //  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetConditionalClassificationIndexGrow() EXIT() ...\n");
  //  }
  return;
}
void getConditionalClassificationIndexPred(uint     size,
                                           uint     rTarget,
                                           double  *responsePtr,
                                           double **outcomeCLS,
                                           double  *maxVote,
                                           double  *denomCount,
                                           double  *cpv) {
  uint i, k;
  uint cumDenomCount;
  uint *condClassificationCount;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetConditionalClassificationIndexGrow() ENTRY ...\n");
  //  }
  cumDenomCount = 0;
  condClassificationCount = uivector(1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
    cpv[k] = condClassificationCount[k] = 0;
  }
  for (i = 1; i <= size; i++) {
    if ( (uint) responsePtr[i] <= RF_rFactorSize[RF_rFactorMap[rTarget]]) {
      condClassificationCount[(uint) responsePtr[i]] ++;
      if (denomCount[i] != 0) {
        cumDenomCount += 1;
        if (responsePtr[i] == maxVote[i]) {
          cpv[(uint) responsePtr[i]] += 1.0;
        }
      }
    }
  }  
  if (cumDenomCount == 0) {
    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
      cpv[k] = RF_nativeNaN;
    }
  }
  else {
    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
      if (condClassificationCount[k] != 0) {
        cpv[k] = 1.0 - cpv[k] / (double) condClassificationCount[k];
      }
      else {
        cpv[k] = RF_nativeNaN;
      }
    }
  }
  free_uivector(condClassificationCount, 1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    
  //    RF_nativePrint("\nConditional Classification and error update complete:  ");
  //    RF_nativePrint("\nTarget:  %10d ", rTarget);
  //    RF_nativePrint("\nLevel     ");
  //    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
  //      RF_nativePrint(" %10d", k);
  //    }
  //    RF_nativePrint("\n          ");
  //    for (k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
  //      RF_nativePrint(" %10.4f", cpv[k]);
  //    }
  //    RF_nativePrint("\n");
  //  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetConditionalClassificationIndexGrow() EXIT() ...\n");
  //  }
  return;
}
double getClassificationIndex(uint     size,
                              uint     rTarget,
                              double  *responsePtr,
                              double  *denomCount,
                              double  *maxVote) {
  uint i;
  uint cumDenomCount;
  double result;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetClassificationIndex() ENTRY ...\n");
  //  }
  cumDenomCount = 0;
  result = 0.0;
  for (i=1; i <= size; i++) {
    if (denomCount[i] > 0) {
      cumDenomCount += 1;
      if (responsePtr[i] == maxVote[i]) {
        result += 1.0;
      }
    }
    else {
      maxVote[i] = RF_nativeNaN;
    }
  }  
  if (cumDenomCount == 0) {
    result = RF_nativeNaN;
  }
  else {
    result = 1.0 - result / (double) cumDenomCount;
  }
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    
  //    RF_nativePrint("\nPredicted Outcome used in Classification Index Calculations:  ");
  //    RF_nativePrint("\n        count     OOBcount     Response      Outcome");
  //    for (i=1; i <= size; i++) {
  //      RF_nativePrint("\n %12d %12d %12.4f %12.4f", i, denomCount[i], responsePtr[i], maxVote[i]);
  //    }
  //    RF_nativePrint("\n");
  //    
  //    RF_nativePrint("\nClassification and error update complete:  ");
  //    RF_nativePrint("\nCount of cumulative OOB count:         %20d", cumDenomCount);
  //    RF_nativePrint("\nResult:                                %20.4f", result);
  //    RF_nativePrint("\n");
  //  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetClassificationIndex() EXIT() ...\n");
  //  }
  return result;
}
double getGMeanIndexGrow(uint    size,
                         uint    rTarget,
                         double *responsePtr,
                         double *denomCount,
                         double *maxVote) {
  uint i, k;
  uint cumDenomCount;
  double *trueRate, *falseRate;
  double denom, result;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetGMeanIndexGrow() ENTRY ...\n");
  //  }
  cumDenomCount = 0;
  result = 1.0;
  trueRate  = dvector(1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  falseRate = dvector(1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  for (k = 1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
    trueRate[k] = falseRate[k] = 0;
  }
  for (i = 1; i <= size; i++) {
    if (denomCount[i] > 0) {
      cumDenomCount += 1;
      if (responsePtr[i] == maxVote[i]) {
        trueRate[(uint) responsePtr[i]] += 1.0;
      }
      else {
        falseRate[(uint) responsePtr[i]] += 1.0;
      }
    }
  }  
  if (cumDenomCount == 0) {
    result = RF_nativeNaN;
  }
  else {
    for (k = 1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
      denom = trueRate[k] + falseRate[k];
      if (denom > 0) {
        result = result * trueRate[k] / denom; 
      }
      else {
        result = result * (1 + trueRate[k]) / (1 + denom); 
      }
    }
    result = 1.0 - sqrt(result);
  }
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    RF_nativePrint("\nConfusion Matrix:");
  //    RF_nativePrint("\n                              Predicted L1         Predicted L2");
  //    RF_nativePrint("\n              True L1 %20d %20d", (uint) trueRate[1],  (uint) falseRate[1]);
  //    RF_nativePrint("\n              True L2 %20d %20d", (uint) falseRate[2], (uint) trueRate[2]);  
  //    RF_nativePrint("\n");
  //  }
  free_dvector(trueRate, 1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  free_dvector(falseRate, 1, RF_rFactorSize[RF_rFactorMap[rTarget]]);
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    
  //    RF_nativePrint("\nPredicted Outcome used in Classification Index Calculations:  ");
  //    RF_nativePrint("\n        count     OOBcount     Response      Outcome");
  //    for (i=1; i <= size; i++) {
  //      RF_nativePrint("\n %12d %12.4f %12.4f %12.4f", i, denomCount[i], responsePtr[i], maxVote[i]);
  //    }
  //    RF_nativePrint("\n");
  //    
  //    RF_nativePrint("\nClassification and error update complete:  ");
  //    RF_nativePrint("\nCount of cumulative OOB count:         %20d", cumDenomCount);
  //    RF_nativePrint("\nResult:                                %20.4f", result);
  //    RF_nativePrint("\n");
  //  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetGMeanIndexGrow() EXIT() ...\n");
  //  }
  return result;
}
double getGMeanIndexPred(uint    size,
                         uint    rTarget,
                         double *responsePtr,
                         double *denomCount,
                         double *maxVote) {
  uint i, k;
  uint cumDenomCount;
  double *trueRate, *falseRate;
  double denom, result;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetGMeanIndexGrow() ENTRY ...\n");
  //  }
  cumDenomCount = 0;
  result = 1.0;
  trueRate  = dvector(1, RF_rFactorSizeTest[RF_rFactorMap[rTarget]]);
  falseRate = dvector(1, RF_rFactorSizeTest[RF_rFactorMap[rTarget]]);
  for (k = 1; k <= RF_rFactorSizeTest[RF_rFactorMap[rTarget]]; k++) {
    trueRate[k] = falseRate[k] = 0;
  }
  for (i = 1; i <= size; i++) {
    if (denomCount[i] > 0) {
      cumDenomCount += 1;
      if (responsePtr[i] == maxVote[i]) {
        trueRate[(uint) responsePtr[i]] += 1.0;
      }
      else {
        falseRate[(uint) responsePtr[i]] += 1.0;
      }
    }
  }  
  if (cumDenomCount == 0) {
    result = RF_nativeNaN;
  }
  else {
    for (k = 1; k <= RF_rFactorSizeTest[RF_rFactorMap[rTarget]]; k++) {
      denom = trueRate[k] + falseRate[k];
      if (denom > 0) {
        result = result * trueRate[k] / denom; 
      }
      else {
        result = result * (1 + trueRate[k]) / (1 + denom); 
      }
    }
    result = 1.0 - sqrt(result);
  }
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    RF_nativePrint("\nConfusion Matrix:");
  //    RF_nativePrint("\n          ");
  //    for (i=1; i <= RF_rFactorSizeTest[RF_rFactorMap[rTarget]]; i++) {
  //      RF_nativePrint(" %10d", i);
  //    }
  //    RF_nativePrint("\ntrue:     ");
  //    for (i=1; i <= RF_rFactorSizeTest[RF_rFactorMap[rTarget]]; i++) {
  //      RF_nativePrint(" %10d", (uint) trueRate[i]);
  //    }
  //    RF_nativePrint("\nfalse:    ");
  //    for (i=1; i <= RF_rFactorSizeTest[RF_rFactorMap[rTarget]]; i++) {
  //      RF_nativePrint(" %10d", (uint) falseRate[i]);
  //    }
  //    RF_nativePrint("\n");
  //  }
  free_dvector(trueRate, 1, RF_rFactorSizeTest[RF_rFactorMap[rTarget]]);
  free_dvector(falseRate, 1, RF_rFactorSizeTest[RF_rFactorMap[rTarget]]);
  //  if (getTraceFlag(0) & ENSB_LOW_TRACE) {
  //    
  //    RF_nativePrint("\nPredicted Outcome used in Classification Index Calculations:  ");
  //    RF_nativePrint("\n        count     OOBcount     Response      Outcome");
  //    for (i=1; i <= size; i++) {
  //      RF_nativePrint("\n %12d %12.4f %12.4f %12.4f", i, denomCount[i], responsePtr[i], maxVote[i]);
  //    }
  //    RF_nativePrint("\n");
  //    
  //    RF_nativePrint("\nClassification and error update complete:  ");
  //    RF_nativePrint("\nCount of cumulative OOB count:         %20d", cumDenomCount);
  //    RF_nativePrint("\nResult:                                %20.4f", result);
  //    RF_nativePrint("\n");
  //  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetGMeanIndexGrow() EXIT() ...\n");
  //  }
  return result;
}
void restoreMultiClassProb(uint treeID) {
  LeafLinkedObj *leafLinkedPtr;
  Terminal *parent;
  uint leaf;
  uint j, k;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nrestoreMultiClassProb() ENTRY ...\n");
  //  }
  leafLinkedPtr = RF_leafLinkedObjHead[treeID] -> fwdLink;
  while (leafLinkedPtr != NULL) {
    parent = leafLinkedPtr -> termPtr;
    leaf = parent -> nodeID;
    if ((parent -> membrCount) > 0) {
      for (j = 1; j <= RF_rFactorCount; j++) {
        for (k = 1; k <= RF_rFactorSize[j]; k++) {
          (parent -> multiClassProb)[j][k] = RF_TN_CLAS_ptr[treeID][leaf][j][k];
        }
      }
    }
    else {
      RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      RF_nativeError("\nRF-SRC:  Zero node count encountered in restoreMultiClassProb() in (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
      RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      RF_nativeExit();
    }
    leafLinkedPtr = leafLinkedPtr -> fwdLink;
    //  if (getTraceFlag(treeID) & SUMM_HGH_TRACE) {
    //    RF_nativePrint("\nClass proportion vector for (tree, leaf):  (%10d, %10d) \n", treeID, leaf);
    //    for (j=1; j <= RF_rFactorCount; j++) {
    //      RF_nativePrint("\nFactor Index:  %10d ", RF_rFactorIndex[j]);
    //      RF_nativePrint("\nMember Count:  %10d", parent -> membrCount);
    //      RF_nativePrint("\nClass ->  ");
    //      for (k=1; k <= RF_rFactorSize[j]; k++) {
    //        RF_nativePrint("%10d", k);
    //      }
    //      RF_nativePrint("\n");
    //      RF_nativePrint("%10d", j);
    //      for (k=1; k <= RF_rFactorSize[j]; k++) {
    //        RF_nativePrint("%10d", (parent -> multiClassProb)[j][k]);
    //      }
    //    }
    //  }
  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nrestoreMultiClassProb() EXIT ...\n");
  //  }
}
void getMaxVote(uint     size,
                uint     rTarget,
                double **outcomeCLS,
                double  *denomCount,
                double  *maxVote) {
  uint i,k;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetMaxVote() ENTRY() ...\n");
  //  }
  if ((RF_opt & OPT_CLAS_RFQ) && RF_rFactorMinorityFlag[RF_rFactorMap[rTarget]]) {
    uint minorityClass = RF_rFactorMinority[RF_rFactorMap[rTarget]];
    uint majorityClass = RF_rFactorMajority[RF_rFactorMap[rTarget]];
    double threshold   = RF_rFactorThreshold[RF_rFactorMap[rTarget]];
    for (i = 1; i <= size; i++) {
      if (denomCount[i] > 0) {
        if (outcomeCLS[minorityClass][i] >= threshold) {
          maxVote[i] = (double) minorityClass;
        }
        else {
          maxVote[i] = (double) majorityClass;
        }
      }
      else {
        maxVote[i] = RF_nativeNaN;
      }
    }  
  }
  else {
    double maxValue, maxClass;
    for (i = 1; i <= size; i++) {
      if (denomCount[i] > 0) {
        maxValue = 0.0;
        maxClass = 0.0;
        for (k = 1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
          if (maxValue <= outcomeCLS[k][i]) {
            maxValue = outcomeCLS[k][i];
            maxClass = (double) k;
          }
        }
        maxVote[i] = maxClass;
      }
      else {
        maxVote[i] = RF_nativeNaN;
      }
    }  
 } 
  //  if (getTraceFlag(0) & SUMM_HGH_TRACE) {
  //    RF_nativePrint("\n  Target Factor: %10d", rTarget);
  //    for (int k=1; k <= RF_rFactorSize[RF_rFactorMap[rTarget]]; k++) {
  //      RF_nativePrint("\n%10d", k);
  //      for (i = 1; i <= size; i++) {
  //        RF_nativePrint("%10.4f", outcomeCLS[k][i]);
  //      }
  //    }
  //    RF_nativePrint("\n");
  //    RF_nativePrint("\nMax Vote calculation: \n");
  //    RF_nativePrint("          ");
  //    for (i = 1; i <= size; i++) {
  //      RF_nativePrint("%10d", i);
  //    }
  //    RF_nativePrint("\n");
  //    RF_nativePrint("          ");
  //    for (i = 1; i <= size; i++) {
  //      RF_nativePrint("%10.4f", maxVote[i]);
  //    }
  //    RF_nativePrint("\n");
  //  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetMaxVote() EXIT() ...\n");
  //  }
}
