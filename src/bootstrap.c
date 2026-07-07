
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "bootstrap.h"
#include "random.h"
#include "nrutil.h"
#include "sampling.h"
#include "nodeOps.h"
#include "error.h"
char bootstrap (char     mode,
                uint     treeID,
                Node    *nodePtr,
                uint    *subsetIndex,
                uint     subsetSize,
                uint    *index,  
                uint     indexSize) {  
  char *permissible;
  char result;
  uint i, j, k;
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\nbootstrap() ENTRY ...\n");
  //  }
  result = TRUE;
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\nBootstrap ran1A() for tree:    %10d, %20d", treeID, randomGetChain(treeID));
  //    RF_nativePrint("\n  called with subset size:     %10d", subsetSize);
  //    RF_nativePrint("\n  called with bootstrap size:  %10d", indexSize);
  //    RF_nativePrint("\n  for current nodeID:          %10d", nodePtr -> nodeID);
  //  }
  if (!(RF_opt & OPT_BOOT_TYP1) && (RF_opt & OPT_BOOT_TYP2)) {
    for (i=1; i <= subsetSize; i++) {
      index[i] = subsetIndex[i];
    }
  }
  else {
    if ( (RF_opt & OPT_BOOT_TYP1) && (RF_opt & OPT_BOOT_TYP2) ) {
      i = 0;
      for (k = 1; k <= RF_subjSize; k++) {
        for (j = 1; j <= RF_bootstrapIn[treeID][k]; j++) {
          index[++i] = k;
        }
      }
    }
    else {
      if (RF_subjWeightType == RF_WGHT_UNIFORM) {
        if (RF_optHigh & OPT_BOOT_SWOR) {
          uint *sworVector = uivector(1, subsetSize);
          uint sworVectorSize = subsetSize;
          uint sworIndex;
          for (j = 1; j <= sworVectorSize; j++) {
            sworVector[j] = subsetIndex[j];
          }
          for (j = 1; j <= indexSize; j++) {
            sworIndex = (uint) ceil(ran1A(treeID) * (sworVectorSize * 1.0));
            index[j] = sworVector[sworIndex];
            sworVector[sworIndex] = sworVector[sworVectorSize];
            sworVectorSize --;
          }
          free_uivector (sworVector, 1, subsetSize);
        }
        else {    
          for (i = 1; i <= indexSize; i++) {
            //  if (getTraceFlag(treeID) & RAND_DEF_TRACE) {
            //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
            //      RF_nativePrint("\nBootstrap ran1A() for indv:  %10d, %20d", i, randomGetChain(treeID));
            //    }
            //  }
            k = (uint) ceil(ran1A(treeID)*(subsetSize * 1.0));
            index[i] = subsetIndex[k];
          }
        }
      }
      else {
        if (RF_subjWeightType != RF_WGHT_UNIFORM) {
          permissible = cvector(1, RF_subjSize);
          for (i = 1; i <= RF_subjSize; i++) {
            permissible[i] = FALSE;
          }
          for (i = 1; i <= subsetSize; i++) {
            permissible[subsetIndex[i]] = TRUE;
          }
        }
        else {
          permissible = NULL;
        }
        DistributionObj *obj = makeDistributionObjRaw();
        obj -> permissibleIndex = (RF_subjWeightType == RF_WGHT_UNIFORM) ? subsetIndex : NULL;
        obj -> permissible       = (RF_subjWeightType == RF_WGHT_UNIFORM) ? NULL : permissible;
        obj -> permissibleSize   = (RF_subjWeightType == RF_WGHT_UNIFORM) ? subsetSize : RF_subjSize;
        obj -> augmentationSize = NULL;
        obj -> weightType = RF_subjWeightType;
        obj -> weight = RF_subjWeight;
        obj -> weightSorted = RF_subjWeightSorted;
        obj -> densityAllocSize = RF_subjWeightDensitySize;
        initializeCDFNew(treeID, obj);
        for (i = 1; i <= indexSize; i++) {
          //  if (getTraceFlag(treeID) & RAND_DEF_TRACE) {
          //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
          //      RF_nativePrint("\nBootstrap ran1A() for indv:  %10d, %20d", i, randomGetChain(treeID));
          //    }
          //  }
          index[i] = sampleFromCDFNew(ran1A, treeID, obj);
          if (RF_optHigh & OPT_BOOT_SWOR) {
            if (index[i] != 0) {
              //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
              //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
              //      RF_nativePrint("\nUpdating CDF:  %10d %10d", i, index[i]);
              //    }
              //  }
              updateCDFNew(treeID, obj);
            }
            else {
              RF_nativeError("\nRF-SRC:  *** ERROR *** ");
              RF_nativeError("\nRF-SRC:  No cases left to select for bootstrap SWOR of size:  %10d", indexSize);
              RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
              RF_nativeExit();
            }
          }
        }
        discardCDFNew(treeID, obj);
        freeDistributionObjRaw(obj);
        if (RF_subjWeightType != RF_WGHT_UNIFORM) {
          free_cvector(permissible, 1, RF_subjSize);
        }
      }
    }
  }
  uint iter;
  for (i = 1; i <= RF_observationSize; i++) {
    RF_bootMembershipFlag[treeID][i]  = FALSE;
    RF_oobMembershipFlag[treeID][i]   = TRUE;
    RF_bootMembershipCount[treeID][i] = 0;
  }
  iter = 0;
  for (i = 1; i <= indexSize; i++) {
    RF_bootMembershipIndex[treeID][++iter] = index[i];
    RF_bootMembershipFlag[treeID][index[i]] = TRUE;
    RF_oobMembershipFlag[treeID][index[i]]  = FALSE;
    RF_bootMembershipCount[treeID][index[i]] ++;
    if (RF_optHigh & OPT_MEMB_USER) {
      RF_BOOT_CT_ptr[treeID][index[i]] ++;
    }
  }
  RF_oobSize[treeID] = 0;
  RF_ibgSize[treeID] = 0;
  for (i = 1; i <= RF_observationSize; i++) {
    if (RF_bootMembershipFlag[treeID][i] == FALSE) {
      RF_oobMembershipIndex[treeID][++RF_oobSize[treeID]] = i;
    }
    else {
      RF_ibgMembershipIndex[treeID][++RF_ibgSize[treeID]] = i;
    }
  }
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\n\nIncoming Subset:  ");
  //    RF_nativePrint("\n   counter   orgIndex (in original data) \n");
  //    for (i=1; i <=  subsetSize; i++) {
  //      RF_nativePrint("%10d %10d \n", i, subsetIndex[i]);
  //    }
  //    RF_nativePrint("\n\nReplicates:  ");
  //    RF_nativePrint("\n   counter   orgIndex (in original data) \n");
  //    for (i=1; i <=  indexSize; i++) {
  //      RF_nativePrint("%10d %10d \n", i, index[i]);
  //    }
  //    RF_nativePrint("\n\nRelative Membership Flags:  ");
  //    RF_nativePrint("\n  orgIndex       flag \n");
  //    for (i=1; i <=  subsetSize; i++) {
  //      RF_nativePrint("%10d %10d \n", subsetIndex[i], RF_bootMembershipFlag[treeID][subsetIndex[i]]);
  //    }
  //    RF_nativePrint("\n\nGlobal Membership Flags:  ");
  //    RF_nativePrint("\n          i        ibg        oob        cnt");
  //    for (i = 1; i <= RF_observationSize; i++) {
  //      RF_nativePrint("\n %10d %10d %10d %10d ", i, RF_bootMembershipFlag[treeID][i], RF_oobMembershipFlag[treeID][i], RF_bootMembershipCount[treeID][i]); 
  //    }
  //    RF_nativePrint("\n\nIBG Size:  %10d ", RF_ibgSize[treeID]);
  //    RF_nativePrint(  "\nOOB Size:  %10d ", RF_oobSize[treeID]);
  //  }
  if (result) {
    result = getNodeSign(mode, treeID, nodePtr, index, indexSize);
    if (!result) {
      //    if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
      //      RF_nativePrint("\nRF-SRC:  Responses are all missing in the sample.  Bootstrap sample has been discarded.");
      //    }
    }
  }
  else {
    //    if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
    //      RF_nativePrint("\nRF-SRC:  Zero OOB count.  Bootstrap sample has been discarded.");
    //    }
  }
  if (result) {
    if (mode == RF_PRED) {
    }
    //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
    //    RF_nativePrint("\nRF-SRC:  Bootstrapping complete.");
    //  }
  } 
  else {
    //    if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
    //      RF_nativePrint("\nBootstrap sample is invalid.");
    //    }
  }
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\nbootstrap() EXIT ...\n");
  //  }
  return result;
}
char getNodeSign (char mode,
                  uint treeID,
                  Node *nodePtr,
                  uint *bmIndex,
                  uint repMembrSize) {
  int   *mvNSptr;
  int   *fmvNSptr;
  char result;
  uint i,p,q,m;
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\ngetNodeSign() ENTRY ...\n");
  //  }
  result = TRUE;
  switch (mode) {
  case RF_PRED:
    if (RF_mRecordSize > 0) {
      stackMPSign(nodePtr, RF_mpIndexSize);
      mvNSptr = nodePtr -> mpSign;
    }
    else {
      mvNSptr = NULL;
    }
    if (RF_fmRecordSize > 0) {
      stackFMPSign(nodePtr, RF_fmpIndexSize);
      fmvNSptr = nodePtr -> fmpSign;
    }
    else {
      fmvNSptr = NULL;
    }
    break;
  default:
    if (RF_mRecordSize > 0) {
      stackMPSign(nodePtr, RF_mpIndexSize);
      mvNSptr = nodePtr -> mpSign;
    }
    else {
      mvNSptr = NULL;
    }
    fmvNSptr = NULL;
    break;
  }  
  if (mvNSptr != NULL) {
    int **mvBootstrapSign = imatrix(1, RF_mpIndexSize, 1, repMembrSize);
    for (p = 1; p <= RF_mpIndexSize; p++) {
      for (i = 1; i <= repMembrSize; i++) {
        mvBootstrapSign[p][i] = 0;
      }
    }
    for (p = 1; p <= RF_mpIndexSize; p++) {
      mvNSptr[p] = 0;
    }
    for (i = 1; i <= repMembrSize; i++) {
      m = bmIndex[i];
      if (RF_mRecordMap[m] != 0) {
        for (p = 1; p <= RF_mpIndexSize; p++) {
          if (RF_mpIndex[p] < 0) {
            mvBootstrapSign[p][i] = RF_mpSign[(uint) abs(RF_mpIndex[p])][RF_mRecordMap[m]];
          }
          else {
            mvBootstrapSign[p][i] = RF_mpSign[RF_ySize + (uint) RF_mpIndex[p]][RF_mRecordMap[m]];
          }
        }
      }
      else {
        for (p = 1; p <= RF_mpIndexSize; p++) {
          mvBootstrapSign[p][i] = 0;
        }
      }
      for (p = 1; p <= RF_mpIndexSize; p++) {
        mvNSptr[p] = mvNSptr[p] + mvBootstrapSign[p][i];
      }
    }
    m = 0;
    for (p = 1; p <= RF_mpIndexSize; p++) {
      if (mvNSptr[p] > 0) {
        if ((uint) mvNSptr[p] == repMembrSize) {
          mvNSptr[p] = -1;
        }
        else {
          mvNSptr[p] = 1;
        }
      }
      if(RF_mpIndex[p] < 0) {
        if (mvNSptr[p] == -1) result = FALSE;
      }
      else {
        if (mvNSptr[p] == -1) m ++;
      }
    }  
    if (m == RF_mpIndexSize) {
      result = FALSE;
    }
    //    if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
    //      RF_nativePrint("\n\nGROW Individual Sample Signature For Tree:  ");
    //      RF_nativePrint("\n bootIndex   signatures ->\n");
    //      RF_nativePrint(  "            ");
    //      for (i=1; i <= RF_mpIndexSize; i++) {
    //        RF_nativePrint("%3d", RF_mpIndex[i]);
    //      }
    //      RF_nativePrint("\n");
    //      for (i=1; i <=  repMembrSize; i++) {
    //        RF_nativePrint("%10d  ", i);
    //        for (p=1; p <= RF_mpIndexSize; p++) {
    //          RF_nativePrint("%3d", mvBootstrapSign[p][i]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //    }
    //    if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
    //      RF_nativePrint("\nGROW Missing Sample Signature For Node:  \n");
    //      RF_nativePrint(  "            ");
    //      for (p=1; p <= RF_mpIndexSize; p++) {
    //        RF_nativePrint("%3d", RF_mpIndex[p]);
    //      }
    //      RF_nativePrint("\n");
    //      RF_nativePrint(  "            ");
    //      for (p=1; p <= RF_mpIndexSize; p++) {
    //        RF_nativePrint("%3d", mvNSptr[p]);
    //      }
    //      RF_nativePrint("\n");
    //    }
    free_imatrix(mvBootstrapSign, 1, RF_mpIndexSize, 1, repMembrSize);
  }
  if (fmvNSptr != NULL) {
    for (p = 1; p <= RF_fmpIndexSize; p++) {
      fmvNSptr[p] = 1;
    }
    if (RF_mRecordSize > 0) {
      p = q = 1;
      while ((p <= RF_mpIndexSize) && (q <= RF_fmpIndexSize)) {
        if (RF_mpIndex[p] == RF_fmpIndex[q]) {
          if (mvNSptr[p] == -1) {
            fmvNSptr[q] = -1;
          }
          p++;
          q++;
        }
        else if (RF_fmpIndex[q] < 0) {
          if (RF_mpIndex[p] > 0) {
            q++;
          }
          else {
            if (abs(RF_fmpIndex[q]) < abs(RF_mpIndex[p])) {
              q++;
            }
            else {
              p++;
            }
          }
        }
        else {
          if (RF_fmpIndex[q] < RF_mpIndex[p]) {
            q++;
          }
          else {
            p++;
          }
        }
      }  
    }  
    //    if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
    //      RF_nativePrint("\nPRED Missing Signature For Node:  \n");
    //      for (p=1; p <= RF_fmpIndexSize; p++) {
    //        RF_nativePrint("%3d", RF_fmpIndex[p]);
    //      }
    //      RF_nativePrint("\n");
    //      for (p=1; p <= RF_fmpIndexSize; p++) {
    //        RF_nativePrint("%3d", fmvNSptr[p]);
    //      }
    //      RF_nativePrint("\n");
    //    }
  }  
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\ngetNodeSign() EXIT ...\n");
  //  }
  return result;
}
char bootstrapSubject (char     mode,
                       uint     treeID,
                       Node    *nodePtr,
                       uint   **index,
                       uint    *indexSize) {
  char   *permissible;
  uint   *permissibleIndex;
  uint   *subjIndex;
  uint   *iterativeIndex;
  char result;
  uint i, j, k;
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\nbootstrapSubject() ENTRY ...\n");
  //  }
  subjIndex = NULL;  
  result = TRUE;
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\nBootstrap ran1A() for tree:    %10d, %20d", treeID, randomGetChain(treeID));
  //    RF_nativePrint("\n  for current nodeID:          %10d", nodePtr -> nodeID);
  //  }
  iterativeIndex = uivector(1, RF_bootstrapSize);
  if (!(RF_opt & OPT_BOOT_TYP1) && (RF_opt & OPT_BOOT_TYP2)) {
    for (i = 1; i <= RF_bootstrapSize; i++) {
      iterativeIndex[i] = i;
    }
  }
  else {
    if ( (RF_opt & OPT_BOOT_TYP1) && (RF_opt & OPT_BOOT_TYP2) ) {
      i = 0;
      for (k = 1; k <= RF_subjSize; k++) {
        for (j = 1; j <= RF_bootstrapIn[treeID][k]; j++) {
          subjIndex[++i] = k;
        }
      }
    }
    else {
      if ((RF_subjWeightType == RF_WGHT_UNIFORM) && !(RF_optHigh & OPT_BOOT_SWOR)) {
        for (i = 1; i <= RF_bootstrapSize; i++) {
          //  if (getTraceFlag(treeID) & RAND_DEF_TRACE) {
          //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
          //      RF_nativePrint("\nBootstrap ran1A() for indv:  %10d, %20d", i, randomGetChain(treeID));
          //    }
          //  }
          k = (uint) ceil(ran1A(treeID)*(RF_subjSize * 1.0));
          iterativeIndex[i] = k;
        }
      }
      else {
        if (RF_subjWeightType != RF_WGHT_UNIFORM) {
          permissible = cvector(1, RF_subjSize);
          for (i = 1; i <= RF_subjSize; i++) {
            permissible[i] = TRUE;
          }
          permissibleIndex = NULL;
        }
        else {
          permissibleIndex = uivector(1, RF_subjSize);
          for (i = 1; i <= RF_subjSize; i++) {
            permissibleIndex[i] = i;
          }
          permissible = NULL;
        }
        DistributionObj *obj = makeDistributionObjRaw();
        obj -> permissibleIndex = (RF_subjWeightType == RF_WGHT_UNIFORM) ? permissibleIndex : NULL;
        obj -> permissible       = (RF_subjWeightType == RF_WGHT_UNIFORM) ? NULL : permissible;
        obj -> permissibleSize   = (RF_subjWeightType == RF_WGHT_UNIFORM) ? RF_subjSize : RF_subjSize;
        obj -> augmentationSize = NULL;
        obj -> weightType = RF_subjWeightType;
        obj -> weight = RF_subjWeight;
        obj -> weightSorted = RF_subjWeightSorted;
        obj -> densityAllocSize = RF_subjWeightDensitySize;
        initializeCDFNew(treeID, obj);
        for (i = 1; i <= RF_bootstrapSize; i++) {
          //  if (getTraceFlag(treeID) & RAND_DEF_TRACE) {
          //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
          //      RF_nativePrint("\nBootstrap ran1A() for indv:  %10d, %20d", i, randomGetChain(treeID));
          //    }
          //  }
          iterativeIndex[i] = sampleFromCDFNew(ran1A, treeID, obj);
          if (RF_optHigh & OPT_BOOT_SWOR) {
            if (iterativeIndex[i] != 0) {
              //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
              //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
              //      RF_nativePrint("\nUpdating CDF:  %10d %10d", i, iterativeIndex[i]);
              //    }
              //  }
              updateCDFNew(treeID, obj);
            }
            else {
              RF_nativeError("\nRF-SRC:  *** ERROR *** ");
              RF_nativeError("\nRF-SRC:  No cases left to select for bootstrap SWOR of size:  %10d", RF_bootstrapSize);
              RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
              RF_nativeExit();
            }
          }
        }
        discardCDFNew(treeID, obj);
        freeDistributionObjRaw(obj);
        if (RF_subjWeightType != RF_WGHT_UNIFORM) {
          free_cvector(permissible, 1, RF_subjSize);
        }
        else {
          free_uivector(permissibleIndex, 1, RF_subjSize);
        }
      }
    }
  }
  *indexSize = 0;
  for (i = 1; i <= RF_bootstrapSize; i++) {
    (*indexSize) += RF_subjSlotCount[iterativeIndex[i]];
  }
  *index = uivector(1, *indexSize);
  k = 0;
  for (i = 1; i <= RF_bootstrapSize; i++) {
    for (j = 1; j <= RF_subjSlotCount[iterativeIndex[i]]; j++) {    
      (*index)[++k] = RF_subjList[iterativeIndex[i]][j];
    }
  }
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\n\nIterative Index of Subjects in Bootstrap:  ");
  //    RF_nativePrint("\n      iter  iterIndex  subjCount \n");
  //    for (i = 1; i <=  RF_bootstrapSize; i++) {
  //      RF_nativePrint("%10d %10d \n", i, iterativeIndex[i]);
  //    }
  //    RF_nativePrint("\n\nIterative Index of Subjects in Bootstrap:  ");
  //    RF_nativePrint("\n      iter  iterIndex  subjCount \n");
  //    for (i = 1; i <=  RF_bootstrapSize; i++) {
  //      RF_nativePrint("%10d %10d %10d \n", i, iterativeIndex[i], RF_subjSlotCount[iterativeIndex[i]]);
  //    }
  //    RF_nativePrint("\n\nReplicates by Subject and Case:  ");
  //    RF_nativePrint("\n      iter     subjID       case \n");
  //    for (i = 1; i <=  *indexSize; i++) {
  //      RF_nativePrint("%10d %10d %10d \n", i, RF_caseMap[(*index)[i]], (*index)[i]);
  //    }
  //  }
  free_uivector(iterativeIndex, 1, RF_bootstrapSize);
  result = getNodeSign(mode, treeID, nodePtr, *index, *indexSize);
  if (result == FALSE) {
    //    if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
    //      RF_nativePrint("\nRF-SRC:  Responses are all missing in the sample.  Bootstrap sample has been discarded.");
    //    }
  }
  if (result == TRUE) {
    if (mode == RF_PRED) {
    }
    //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
    //    RF_nativePrint("\nRF-SRC:  Bootstrapping complete.");
    //  }
  } 
  else {
    //    if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
    //      RF_nativePrint("\nBootstrap sample is invalid.");
    //    }
  }
  //  if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
  //    RF_nativePrint("\nbootstrapSubject() EXIT ...\n");
  //  }
  return result;
}
