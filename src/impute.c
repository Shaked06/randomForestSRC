
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "impute.h"
#include "bootstrap.h"
#include "rfsrcUtil.h"
#include "tree.h"
#include "polarity.h"
#include "nodeOps.h"
#include "nrutil.h"
#include "error.h"
char imputeNode (char     type,
                 char     termFlag,
                 char     chainFlag,
                 uint     treeID,
                 Node    *nodePtr,
                 uint    *repMembrIndx,
                 uint     repMembrSize,
                 uint    *allMembrIndx,
                 uint     allMembrSize) {
  double  **response;
  double  **predictor;
  double    imputedValue;
  uint     *mRecordMap;
  uint      mpIndexSize;
  int     **mpSign;
  int      *mpIndex;
  int      *mvNSptr;
  uint      mRecordSize;
  double *valuePtr, *imputePtr;
  char mPredictorFlag;
  int  signedSignatureIndex;
  uint unsignedIndexSource;
  uint unsignedIndexTarget;
  char result;
  uint   *glmpIndexPtr;
  double *glmpValuePtr;
  uint   *glmpIndexSize;
  uint  *glmpIndexParentPtr;
  uint   glmpIndexParentSize;
  char mvFlag;
  uint i,p;
  uint localDistributionSize;
  //  Node   ***nodeMembershipPtr = NULL;
  //  uint     *mRecordIndex      = 0;
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nimputeNode(%2d) ENTRY ...\n", type);
  //  }
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\n imputeNode Node ID:  %10d", nodePtr -> nodeID);
  //  }
  mvNSptr = NULL;  
  mpIndex = NULL;  
  mpSign  = NULL;  
  mpIndexSize  = 0;  
  mRecordMap = NULL;  
  mRecordSize = 0;    
  predictor  = NULL;  
  response   = NULL;  
  imputedValue = 0.0;  
  glmpIndexPtr = NULL;
  glmpValuePtr = NULL;
  glmpIndexSize = NULL;
  glmpIndexParentPtr = NULL;
  glmpIndexParentSize = 0;
  result = FALSE;
  switch (type) {
  case RF_PRED:
    mRecordSize = RF_fmRecordSize;
    if (mRecordSize > 0) {
      response = RF_fresponse[treeID];
      predictor = RF_fobservation[treeID];
      mRecordMap = RF_fmRecordMap;
      mpIndexSize = RF_fmpIndexSize;
      mpSign = RF_fmpSign;
      mpIndex = RF_fmpIndex;
      mvNSptr = nodePtr -> fmpSign;
      if (!termFlag) {
        if((nodePtr -> parent) == NULL) {
          glmpIndexParentPtr = uivector(1, mpIndexSize);
          glmpIndexParentSize = mpIndexSize;
          for (p = 1; p <= glmpIndexParentSize; p++) {
            glmpIndexParentPtr[p] = p;
          }
          stackNodeFLMPIndex(nodePtr, glmpIndexParentSize);
          glmpIndexPtr  = nodePtr -> flmpIndex;
          glmpIndexSize = & (nodePtr -> flmpIndexActualSize);
          *glmpIndexSize = 0;
          //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
          //    RF_nativePrint("\n root node detected");
          //  }
        }
        else {
          if((nodePtr -> parent) -> flmpIndexActualSize > 0) {
            glmpIndexParentPtr = (nodePtr -> parent) -> flmpIndex;
            glmpIndexParentSize = (nodePtr -> parent) -> flmpIndexActualSize;
            stackNodeFLMPIndex(nodePtr, glmpIndexParentSize);
            glmpIndexPtr  = nodePtr -> flmpIndex;
            glmpIndexSize = & (nodePtr -> flmpIndexActualSize);
            *glmpIndexSize = 0;
          }
          else {
            glmpIndexParentPtr  = NULL;
            glmpIndexParentSize = 0;
            glmpIndexPtr = glmpIndexSize = NULL;
          }
        }  
      }  
      else {
        glmpIndexParentPtr = uivector(1, mpIndexSize);
        glmpIndexParentSize = mpIndexSize;
        for (p = 1; p <= glmpIndexParentSize; p++) {
          glmpIndexParentPtr[p] = p;
        }
        stackNodeFLMPIndex(nodePtr, glmpIndexParentSize);
        glmpIndexPtr  = nodePtr -> flmpIndex;
        glmpValuePtr  = nodePtr -> flmpValue;
        glmpIndexSize = & (nodePtr -> flmpIndexActualSize);
        *glmpIndexSize = 0;
        //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
        //    RF_nativePrint("\n PRED mode terminal node missingness detected, flmv initializing per root protocol");
        //  }
      }
      //  nodeMembershipPtr = RF_fnodeMembership;
      //  mRecordIndex      = RF_fmRecordIndex;
      result = TRUE;
    }
    break;
  default:
    mRecordSize = RF_mRecordSize;
    if (mRecordSize > 0) {
      response = RF_response[treeID];
      predictor = RF_observation[treeID];
      mRecordMap = RF_mRecordMap;
      mpIndexSize = RF_mpIndexSize;
      mpSign = RF_mpSign;
      mpIndex = RF_mpIndex;
      mvNSptr = nodePtr -> mpSign;
      if (!termFlag) {
        if((nodePtr -> parent) == NULL) {
          glmpIndexParentPtr = uivector(1, mpIndexSize);
          glmpIndexParentSize = mpIndexSize;
          for (p = 1; p <= glmpIndexParentSize; p++) {
            glmpIndexParentPtr[p] = p;
          }
          stackNodeLMPIndex(nodePtr, glmpIndexParentSize);
          glmpIndexPtr  = nodePtr -> lmpIndex;
          glmpIndexSize = & (nodePtr -> lmpIndexActualSize);
          *glmpIndexSize = 0;
          //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
          //    RF_nativePrint("\n root node detected");
          //  }
        }
        else {
          if((nodePtr -> parent) -> lmpIndexActualSize > 0) {
            glmpIndexParentPtr = (nodePtr -> parent) -> lmpIndex;
            glmpIndexParentSize = (nodePtr -> parent) -> lmpIndexActualSize;
            stackNodeLMPIndex(nodePtr, glmpIndexParentSize);
            glmpIndexPtr  = nodePtr -> lmpIndex;
            glmpIndexSize = & (nodePtr -> lmpIndexActualSize);
            *glmpIndexSize = 0;
          }
          else {
            glmpIndexParentPtr = NULL;
            glmpIndexParentSize = 0;
            glmpIndexPtr = glmpIndexSize = NULL;
          }
        }  
      }  
      else {
        glmpIndexParentPtr = uivector(1, mpIndexSize);
        glmpIndexParentSize = mpIndexSize;
        for (p = 1; p <= glmpIndexParentSize; p++) {
          glmpIndexParentPtr[p] = p;
        }
        stackNodeLMPIndex(nodePtr, glmpIndexParentSize);
        glmpIndexPtr  = nodePtr -> lmpIndex;
        glmpValuePtr  = nodePtr -> lmpValue;
        glmpIndexSize = & (nodePtr -> lmpIndexActualSize);
        *glmpIndexSize = 0;
        //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
        //    RF_nativePrint("\n !PRED mode terminal node missingness detected, lmv initializing per root protocol");
        //  }
      }
      //  nodeMembershipPtr = RF_nodeMembership;
      //  mRecordIndex      = RF_mRecordIndex;
      result = TRUE;
    }
    break;
  }
  if (result == FALSE) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Attempt to impute node with no missingness in type:  %10d", type);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  nodePtr -> imputed = TRUE;
  //  if (getTraceFlag(treeID) & MISS_MED_TRACE) {
  //    if (type != RF_PRED) {
  //      RF_nativePrint("\nData for type GROW (before imputation) in leaf:  ");
  //    }
  //    else {
  //      RF_nativePrint("\nData for type PRED (before imputation) in leaf:  ");
  //    }
  //    RF_nativePrint("%10d", nodePtr -> nodeID);
  //    RF_nativePrint("\n     target   imputation -> \n");
  //    RF_nativePrint(  "          ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint(" %12d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n");
  //    for (i = 1; i <= mRecordSize; i++) {
  //      
  //      if (nodeMembershipPtr[treeID][mRecordIndex[i]] == nodePtr) {
  //        RF_nativePrint("%10d", mRecordIndex[i]);
  //        for (p = 1; p <= mpIndexSize; p++) {
  //          if (mpIndex[p] < 0) {
  //            valuePtr = response[(uint) abs(mpIndex[p])];
  //          }
  //          else {
  //            valuePtr = predictor[(uint) mpIndex[p]];
  //          }
  //          RF_nativePrint(" %12.4f", valuePtr[mRecordIndex[i]]);
  //        }
  //        RF_nativePrint("\n");
  //      }
  //    }
  //  }
  double *localDistribution = dvector(1, repMembrSize + 1);
  for (p = 1; p <= glmpIndexParentSize; p++) {
    if (mvNSptr[glmpIndexParentPtr[p]] != -1) {
      signedSignatureIndex = mpIndex[glmpIndexParentPtr[p]];
      if (signedSignatureIndex < 0) {
        unsignedIndexSource = unsignedIndexTarget = (uint) abs(signedSignatureIndex);
        valuePtr = RF_response[treeID][(uint) abs(signedSignatureIndex)];
        imputePtr = response[(uint) abs(signedSignatureIndex)];
      }
      else {
        unsignedIndexSource = RF_ySize + (uint) signedSignatureIndex;
        switch (type) {
        case RF_PRED:
          if (RF_frSize > 0) {
            unsignedIndexTarget = RF_ySize + (uint) signedSignatureIndex;
          }
          else {
            unsignedIndexTarget = (uint) signedSignatureIndex;
          }
          break;
        default:
          unsignedIndexTarget = RF_ySize + (uint) signedSignatureIndex;
          break;
        }
        valuePtr = RF_observation[treeID][(uint) signedSignatureIndex];
        imputePtr = predictor[(uint) signedSignatureIndex];
      }
      //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
      //     RF_nativePrint("\n  Source Offset Index:  %10d", unsignedIndexSource);
      //     RF_nativePrint("\n  Target Offset Index:  %10d", unsignedIndexTarget);
      //  }
      localDistributionSize = 0;
      mPredictorFlag = FALSE;
      for (i = 1; i <= repMembrSize; i++) {
        if (RF_mRecordSize > 0) {
          mPredictorFlag = TRUE;
          if (RF_mRecordMap[repMembrIndx[i]] == 0) {
            mPredictorFlag = FALSE;
          }
          else if (RF_mpSign[unsignedIndexSource][RF_mRecordMap[repMembrIndx[i]]] == 0) {
            mPredictorFlag = FALSE;
          }
        }
        if (mPredictorFlag == FALSE) {
          localDistributionSize ++;
          localDistribution[localDistributionSize] = valuePtr[repMembrIndx[i]];
        }
      }  
      if (termFlag) {
        if (localDistributionSize == 0) {
          for (i = 1; i <= repMembrSize; i++) {
            mPredictorFlag = TRUE;
            if (RF_mRecordMap[repMembrIndx[i]] == 0) {
              mPredictorFlag = FALSE;
            }
            else if (RF_mpSign[unsignedIndexSource][RF_mRecordMap[repMembrIndx[i]]] == 0) {
              mPredictorFlag = FALSE;
            }
            if (mPredictorFlag == TRUE) {
              localDistributionSize ++;
              localDistribution[localDistributionSize] = valuePtr[repMembrIndx[i]];
            }
          }  
        }
        if (localDistributionSize > 0) {
          if (signedSignatureIndex < 0) {
            if (RF_rType[(uint) abs(signedSignatureIndex)] == 'T') {
              imputedValue = getMeanValue(localDistribution, localDistributionSize);
              imputedValue = getNearestMasterTime(imputedValue, chainFlag, treeID);
            }
            else if (RF_rType[(uint) abs(signedSignatureIndex)] == 'S') {
              imputedValue = getMaximalValue(localDistribution, localDistributionSize, chainFlag, treeID);
            }
            else if (RF_rType[(uint) abs(signedSignatureIndex)] == 'B') {
              imputedValue = getMaximalValue(localDistribution, localDistributionSize, chainFlag, treeID);
            }
            else if (RF_rType[(uint) abs(signedSignatureIndex)] == 'R') {
              imputedValue = getMeanValue(localDistribution, localDistributionSize);
            }
            else if (RF_rType[(uint) abs(signedSignatureIndex)] == 'I') {
              imputedValue = getMaximalValue(localDistribution, localDistributionSize, chainFlag, treeID);
            }
            else if (RF_rType[(uint) abs(signedSignatureIndex)] == 'C') {
              imputedValue = getMaximalValue(localDistribution, localDistributionSize, chainFlag, treeID);
            }
            //              if (getTraceFlag(0) & MISS_HGH_TRACE) {
            //                RF_nativePrint("\nTerminal Node Response Imputed Value for:  ");
            //                RF_nativePrint("\n[resp] = [%10d] \n", (uint) abs(signedSignatureIndex));
            //                RF_nativePrint("%12.4f \n", imputedValue);
            //              }
          }
          else {
            if (RF_xType[(uint) signedSignatureIndex] == 'R') {
              imputedValue = getMeanValue(localDistribution, localDistributionSize);
            }
            else {
              imputedValue = getMaximalValue(localDistribution, localDistributionSize, chainFlag, treeID);
            }
            //              if (getTraceFlag(0) & MISS_HGH_TRACE) {
            //                RF_nativePrint("\nTerminal Node Predictor Imputed Value for:  ");
            //                RF_nativePrint("\n[pred] = [%10d] \n", signedSignatureIndex);
            //                RF_nativePrint("%12.4f \n", imputedValue);
            //              }
          }
        }  
        else {
          if (!(RF_opt & OPT_OUTC_TYPE)) {
            RF_nativeError("\nRF-SRC:  *** ERROR *** ");
            RF_nativeError("\nRF-SRC:  NULL distribution encountered during imputation in type:  %10d", type);
            RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
            RF_nativeExit();
          }
          else {
            imputedValue = RF_nativeNaN;
          }
        }
      }  
      mvFlag = FALSE;
      for (i = 1; i <= allMembrSize; i++) {
        if (mRecordMap[allMembrIndx[i]] > 0) {
          if(mpSign[unsignedIndexTarget][mRecordMap[allMembrIndx[i]]] == 1) {
            mvFlag = TRUE;
            if (localDistributionSize > 0) {
              if (termFlag) {
                imputePtr[allMembrIndx[i]] = imputedValue;
              }
              else {
                imputePtr[allMembrIndx[i]] = getSampleValue(localDistribution, localDistributionSize, chainFlag, treeID);
              }
              //                if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
              //                  RF_nativePrint("\nNode Imputed Value for:  ");
              //                  RF_nativePrint("\n[indv, coord] = [%10d, %10d] \n", allMembrIndx[i], mpIndex[p]);
              //                  RF_nativePrint("%12.4f \n", imputePtr[allMembrIndx[i]]);
              //                }
            }
            else {
            }
          }
        }
      }  
      if (mvFlag) {
        glmpIndexPtr[++(*glmpIndexSize)] = glmpIndexParentPtr[p];
        if (termFlag) {
          glmpValuePtr[(*glmpIndexSize)] = imputedValue;
        }
      }
      //  if (localDistributionSize == 0) {
      //    
      //    
      //    
      //    if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
      //      RF_nativePrint("\nNode Value Not Imputable for:  ");
      //      RF_nativePrint("\n[coord] = [%10d] \n", mpIndex[p]);
      //      RF_nativePrint("\nPrevious value retained. \n");
      //    }
      //  }
    }  
  }  
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\n Local GLM Missingness:  ");
  //      RF_nativePrint("\n   glmpIndexParentSize:  %10d:  ", glmpIndexParentSize);
  //      if (glmpIndexParentSize > 0) {
  //        RF_nativePrint("\n");
  //        RF_nativePrint("\nglmp (cnt)");
  //        for (p=1; p <= *glmpIndexSize; p++) {
  //          RF_nativePrint(" %10d", p);
  //        }
  //        RF_nativePrint("\nglmp (raw)");
  //        for (p = 1; p <= *glmpIndexSize; p++) {
  //          RF_nativePrint(" %10d", glmpIndexPtr[p]);
  //        }
  //        RF_nativePrint("\nglmp (act)");
  //        for (p = 1; p <= *glmpIndexSize; p++) {
  //          RF_nativePrint(" %10d", mpIndex[glmpIndexPtr[p]]);
  //        }
  //        RF_nativePrint("\n");
  //      }
  //      RF_nativePrint("\n");
  //  }
  free_dvector(localDistribution, 1, repMembrSize + 1);
  if (!termFlag) {
    if((nodePtr -> parent) == NULL) {
      free_uivector(glmpIndexParentPtr, 1, mpIndexSize);
    }
  }
  else {
    free_uivector(glmpIndexParentPtr, 1, mpIndexSize);
  }
  //  if (getTraceFlag(treeID) & MISS_MED_TRACE) {
  //    if (type != RF_PRED) {
  //      RF_nativePrint("\nData for type GROW (after imputation) in leaf:  ");
  //    }
  //    else {
  //      RF_nativePrint("\nData for type PRED (after imputation) in leaf:  ");
  //    }
  //    RF_nativePrint("%10d", nodePtr -> nodeID);
  //    RF_nativePrint("\n     target   imputation -> \n");
  //    RF_nativePrint(  "          ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint(" %12d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n");
  //    for (i = 1; i <= mRecordSize; i++) {
  //      
  //      if (nodeMembershipPtr[treeID][mRecordIndex[i]] == nodePtr) {
  //        RF_nativePrint("%10d", mRecordIndex[i]);
  //        for (p = 1; p <= mpIndexSize; p++) {
  //          if (mpIndex[p] < 0) {
  //            valuePtr = response[(uint) abs(mpIndex[p])];
  //          }
  //          else {
  //            valuePtr = predictor[(uint) mpIndex[p]];
  //          }
  //          RF_nativePrint(" %12.4f", valuePtr[mRecordIndex[i]]);
  //        }
  //        RF_nativePrint("\n");
  //      }
  //    }
  //  }
  if((nodePtr -> parent) != NULL) {
    if( ((((nodePtr -> parent) -> left) -> imputed) == TRUE) && ((((nodePtr -> parent) -> right) -> imputed) == TRUE) ) {
      switch (type) {
      case RF_PRED:
        unstackNodeFLMPIndex(nodePtr -> parent);
        break;
      default:
        unstackNodeLMPIndex(nodePtr -> parent);
        break;
      }
    }
  }
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nimputeNode(%2d) EXIT ...\n", type);
  //  }
  return TRUE;
}  
char restoreNodeMembership(char  mode,
                           char  rootFlag,
                           uint  treeID,
                           Node *parent,
                           uint *repMembrIndx,
                           uint  repMembrSize,
                           uint *allMembrIndx,
                           uint  allMembrSize,
                           uint *ngAllMembrIndx,
                           uint  ngAllMembrSize,
                           uint *bootMembrIndxIter,
                           uint *rmbrIterator,
                           uint *ambrIterator) {
  char  bootResult;
  char leftResult, rghtResult;
  char terminalFlag;
  uint *bootMembrIndx;
  uint *leftRepMembrIndx;
  uint *rghtRepMembrIndx;
  uint *leftAllMembrIndx;
  uint *rghtAllMembrIndx;
  uint *ngLeftAllMembrIndx;  
  uint *ngRghtAllMembrIndx;  
  uint bootMembrSize;
  uint leftRepMembrSize, rghtRepMembrSize;
  uint leftAllMembrSize, ngLeftAllMembrSize;
  uint rghtAllMembrSize, ngRghtAllMembrSize;
  uint jLeft;
  uint jRght;
  char daughterFlag;
  SplitInfo *info;
  void *obsLocal, *fobsLocal;
  char (*getDaughterPolarityGeneric) (uint       treeID,
                                      SplitInfo *info,
                                      uint       index,
                                      void      *value,
                                      ...);
  uint i;
  //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
  //    RF_nativePrint("\nrestoreNodeMembership() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
  //    RF_nativePrint("\nrestoreNodeMembership (%10d) leaf:  %10d \n", treeID, parent -> nodeID);
  //    RF_nativePrint("\n  called with   rep size:  %10d", repMembrSize);
  //    RF_nativePrint("\n  called with   all size:  %10d", allMembrSize);
  //    RF_nativePrint("\n  called with ngAll size:  %10d", ngAllMembrSize);
  //  }
  getDaughterPolarityGeneric = NULL;  
  bootResult = TRUE;
  terminalFlag = TRUE;
  if (rootFlag) {
    if ( (!(RF_opt & OPT_BOOT_TYP1) && !(RF_opt & OPT_BOOT_TYP2)) ||
         ( (RF_opt & OPT_BOOT_TYP1) &&  (RF_opt & OPT_BOOT_TYP2)) ) {
      bootMembrIndx  = uivector(1, RF_bootstrapSize);
      bootMembrSize = RF_bootstrapSize;
    }
    else {
      bootMembrIndx  = uivector(1, allMembrSize);
      bootMembrSize = allMembrSize;
    }
    bootResult = bootstrap (mode,
                            treeID,
                            parent,
                            allMembrIndx,
                            allMembrSize,
                            bootMembrIndx,
                            bootMembrSize);
    if (rootFlag & bootResult) {
      repMembrIndx = bootMembrIndx;
      repMembrSize = bootMembrSize;
    }
  }
  else {
    bootMembrIndx = repMembrIndx;
    bootMembrSize = repMembrSize;
    parent -> mpSign = (parent -> parent) -> mpSign;
    parent -> fmpSign = (parent -> parent) -> fmpSign;
  }
  if (bootResult) {
    if (RF_mRecordSize > 0) {
      imputeNode(RF_GROW,
                 FALSE,
                 TRUE,
                 treeID,
                 parent,
                 bootMembrIndx,
                 bootMembrSize,
                 allMembrIndx,
                 allMembrSize);
      if (RF_timeIndex > 0) {
        if (RF_mTimeFlag == TRUE) {
          updateTimeIndexArray(treeID,
                               allMembrIndx,
                               allMembrSize,
                               RF_time[treeID],
                               FALSE,
                               FALSE,
                               RF_masterTimeIndex[treeID]);
        }
      }
    }
    switch (mode) {
    case RF_PRED:
      if (RF_fmRecordSize > 0) {
        imputeNode(RF_PRED,
                   FALSE,
                   FALSE,
                   treeID,
                   parent,
                   bootMembrIndx,
                   bootMembrSize,
                   ngAllMembrIndx,
                   ngAllMembrSize);
      }
      break;
    default:
      break;
    }
  }  
  if (bootResult) {
    if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
      info = parent -> splitInfo;
      terminalFlag = FALSE;
      //    if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
      //      RF_nativePrint("\nForking On:  ");
      //      getNodeInfo(parent);
      //    }
      leftAllMembrIndx = rghtAllMembrIndx = NULL;
      leftAllMembrSize = rghtAllMembrSize = 0;
      leftRepMembrIndx = rghtRepMembrIndx = NULL;
      leftRepMembrSize = rghtRepMembrSize = 0;
      if (RF_optHigh & OPT_MEMB_INCG) {
      }
      else {
        uint *indicator = uivector(1, RF_observationSize);
        leftAllMembrSize = rghtAllMembrSize = 0;
          obsLocal = RF_observation[treeID];
          obsLocal = ((double **) obsLocal)[info -> randomVar[1]];
          if (info -> mwcpSizeAbs[1] > 0) {
            getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
          }
          else {
            getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
          }
        for (i = 1; i <= allMembrSize; i++) {
          //        if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
          //            RF_nativePrint("\ngNode Membership:  %10d %10d ", i, allMembrIndx[i]);
          //        }
          daughterFlag = getDaughterPolarityGeneric(treeID,
                                                    info,
                                                    allMembrIndx[i],
                                                    obsLocal,
                                                    parent,
                                                    RF_REST);
          indicator[allMembrIndx[i]] = daughterFlag;
          if (daughterFlag == LEFT) {
            leftAllMembrSize ++;
            //          if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
            //            RF_nativePrint(" --> LEFT ");
            //          }
          }
          else {
            rghtAllMembrSize ++;
            //          if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
            //            RF_nativePrint(" --> RGHT ");
            //          }
          }
        }  
        //        if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
        //          RF_nativePrint("\n");
        //        }
        //  if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
        //    RF_nativePrint("\n\nLeft Size:  %10d, Right Size:  %10d", leftAllMembrSize, rghtAllMembrSize);
        //    RF_nativePrint("\n");
        //  }
        leftAllMembrIndx  = uivector(1, leftAllMembrSize + 1);
        rghtAllMembrIndx  = uivector(1, rghtAllMembrSize + 1);
        jLeft = jRght = 0;
        for (i = 1; i <= allMembrSize; i++) {
          if (indicator[allMembrIndx[i]] == LEFT) {
            leftAllMembrIndx[++jLeft] = allMembrIndx[i];
          }
          else {
            rghtAllMembrIndx[++jRght] = allMembrIndx[i];
          }
        }
        leftRepMembrIndx  = uivector(1, bootMembrSize + 1);
        rghtRepMembrIndx  = uivector(1, bootMembrSize + 1);
        leftRepMembrSize = rghtRepMembrSize = 0;
        for (i = 1; i <= bootMembrSize; i++) {
          if (indicator[bootMembrIndx[i]] == LEFT) {
            leftRepMembrIndx[++leftRepMembrSize] = bootMembrIndx[i];
          }
          else {
            rghtRepMembrIndx[++rghtRepMembrSize] = bootMembrIndx[i];
          }
        }
        free_uivector(indicator, 1, RF_observationSize);
      }  
      ngLeftAllMembrIndx = ngRghtAllMembrIndx = NULL;
      ngLeftAllMembrSize = ngRghtAllMembrSize = 0;
      if (mode == RF_PRED) {
        uint *indicator = uivector(1, RF_fobservationSize);
        fobsLocal = RF_fobservation[treeID];
        fobsLocal = ((double **) fobsLocal)[info -> randomVar[1]];
        if (info -> mwcpSizeAbs[1] > 0) {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
        }
        else {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
        }
        for (i=1; i <= ngAllMembrSize; i++) {
          //        if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
          //            RF_nativePrint("\nngNode Membership:  %10d %10d ", i, ngAllMembrIndx[i]);
          //        }
          daughterFlag = getDaughterPolarityGeneric(treeID,
                                                    info,
                                                    ngAllMembrIndx[i],
                                                    fobsLocal,
                                                    parent,
                                                    RF_PRED);
          indicator[ngAllMembrIndx[i]] = daughterFlag;
          if (daughterFlag == LEFT) {
            ngLeftAllMembrSize ++;
            //            if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
            //              RF_nativePrint(" --> LEFT ");
            //            }
          }
          else {
            ngRghtAllMembrSize ++;
            //            if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
            //              RF_nativePrint(" --> RGHT ");
            //            }
          }
        }  
        ngLeftAllMembrIndx  = uivector(1, ngLeftAllMembrSize + 1);
        ngRghtAllMembrIndx  = uivector(1, ngRghtAllMembrSize + 1);
        jLeft = jRght = 0;
        for (i = 1; i <= ngAllMembrSize; i++) {
          if (indicator[ngAllMembrIndx[i]] == LEFT) {
            ngLeftAllMembrIndx[++jLeft] = ngAllMembrIndx[i];
          }
          else {
            ngRghtAllMembrIndx[++jRght] = ngAllMembrIndx[i];
          }
        }
        free_uivector(indicator, 1, RF_fobservationSize);
      }  
      //        if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //          RF_nativePrint("\nrestoreNodeMembership(%10d) LEFT:  \n", treeID);
      //        }
      leftResult = restoreNodeMembership(mode,
                                         FALSE,
                                         treeID,
                                         parent -> left,
                                         leftRepMembrIndx,
                                         leftRepMembrSize,
                                         leftAllMembrIndx,
                                         leftAllMembrSize,
                                         ngLeftAllMembrIndx,
                                         ngLeftAllMembrSize,
                                         bootMembrIndxIter,
                                         rmbrIterator,
                                         ambrIterator);
      if(!leftResult) {
        //        if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
        //          RF_nativePrint("\nrestoreNodeMembership(%10d) LEFT call failed:  ", treeID);
        //        }
      }
      //        if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //          RF_nativePrint("\nrestoreNodeMembership(%10d) RGHT:  \n", treeID);
      //        }
      rghtResult = restoreNodeMembership(mode,
                                         FALSE,
                                         treeID,
                                         parent -> right,
                                         rghtRepMembrIndx,
                                         rghtRepMembrSize,
                                         rghtAllMembrIndx,
                                         rghtAllMembrSize,
                                         ngRghtAllMembrIndx,
                                         ngRghtAllMembrSize,
                                         bootMembrIndxIter,
                                         rmbrIterator,
                                         ambrIterator);
      if(!rghtResult) {
        //        if (getTraceFlag(treeID) & BOOT_MED_TRACE) {
        //          RF_nativePrint("\nrestoreNodeMembership(%10d) RGHT call failed:  ", treeID);
        //        }
      }
      if (RF_optHigh & OPT_MEMB_INCG) {
      }
      else {
        free_uivector(leftAllMembrIndx, 1, leftAllMembrSize + 1);
        free_uivector(rghtAllMembrIndx, 1, rghtAllMembrSize + 1);
        free_uivector(leftRepMembrIndx, 1, bootMembrSize + 1);
        free_uivector(rghtRepMembrIndx, 1, bootMembrSize + 1);
      }
      if (mode == RF_PRED) {
        free_uivector(ngLeftAllMembrIndx, 1, ngLeftAllMembrSize + 1);
        free_uivector(ngRghtAllMembrIndx, 1, ngRghtAllMembrSize + 1);
      }
    }  
    else {
      //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //    RF_nativePrint("\n  Terminal node encountered for (nodeID, Node, Terminal):  %10d %20x %20x",  parent -> nodeID, parent, parent -> mate);
      //  }
    }
  }  
  else {
    if (rootFlag) {
      if (!bootResult) {
        terminalFlag = FALSE;
        //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //        RF_nativePrint("\nbootstrap(%10d) FAILED at root node.  Tree rejected.  \n", treeID);
        //      }
      }
    }
  }   
  if (terminalFlag) {
    if (RF_optHigh & OPT_MEMB_INCG) {
      RF_leafLinkedObjTail[treeID] = makeAndSpliceLeafLinkedObj(RF_leafLinkedObjTail[treeID],
                                                                parent,
                                                                RF_TN_RCNT_ptr[treeID][parent -> nodeID],
                                                                RF_TN_ACNT_ptr[treeID][parent -> nodeID]);
    }
    else {
      RF_leafLinkedObjTail[treeID] = makeAndSpliceLeafLinkedObj(RF_leafLinkedObjTail[treeID],
                                                                parent,
                                                                bootMembrSize,
                                                                allMembrSize);
    }
    if ((RF_mRecordSize > 0) || (RF_fmRecordSize > 0)) {
      imputeNodeAndSummarize(1,  
                             mode,
                             treeID,
                             parent,
                             bootMembrIndx,
                             bootMembrSize,
                             allMembrIndx,
                             allMembrSize,
                             ngAllMembrIndx,
                             ngAllMembrSize);
    }
    if (mode == RF_PRED) {
      if (ngAllMembrSize > 0) {
        for (i = 1; i <= ngAllMembrSize; i++) {
          RF_ftTermMembership[treeID][ngAllMembrIndx[i]] = parent -> mate;
        }
      }
    }
    if (RF_optHigh & OPT_MEMB_USER) {
      if (mode == RF_PRED) {      
        if (ngAllMembrSize > 0) {
          for (i = 1; i <= ngAllMembrSize; i++) {
            RF_MEMB_ID_ptr[treeID][ngAllMembrIndx[i]] = parent -> nodeID;
          }
        }
      }
      else {
        if (RF_optHigh & OPT_MEMB_INCG) {
          uint userIterator = *ambrIterator;
          for (i = 1; i <= RF_TN_ACNT_ptr[treeID][parent -> nodeID]; i++) {
            ++(userIterator);
            RF_MEMB_ID_ptr[treeID][RF_AMBR_ID_ptr[treeID][(userIterator)]] = parent -> nodeID;
          }
        }
        else {
          for (i = 1; i <= allMembrSize; i++) {
            RF_MEMB_ID_ptr[treeID][allMembrIndx[i]] = parent -> nodeID;
          }
        }
      }
    }  
    if (RF_optHigh & OPT_MEMB_INCG) {
      updateTerminalNodeOutcomes(mode,
                                 treeID,
                                 RF_leafLinkedObjTail[treeID] -> termPtr,
                                 bootMembrIndx,
                                 RF_TN_RCNT_ptr[treeID][parent -> nodeID],
                                 allMembrIndx,
                                 RF_TN_ACNT_ptr[treeID][parent -> nodeID],
                                 rmbrIterator,
                                 ambrIterator);
    }
    else {
      updateTerminalNodeOutcomes(mode,
                                 treeID,
                                 RF_leafLinkedObjTail[treeID] -> termPtr,
                                 bootMembrIndx,
                                 bootMembrSize,
                                 allMembrIndx,
                                 allMembrSize,
                                 rmbrIterator,
                                 ambrIterator);
    }
    if (RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
      getSplitPath(treeID, parent);
    }
  }  
  if (rootFlag) {
    if ( (!(RF_opt & OPT_BOOT_TYP1) && !(RF_opt & OPT_BOOT_TYP2)) ||
         ( (RF_opt & OPT_BOOT_TYP1) &&  (RF_opt & OPT_BOOT_TYP2)) ) {
      free_uivector(bootMembrIndx, 1, RF_bootstrapSize);
    }
    else {
      free_uivector(bootMembrIndx, 1, allMembrSize);
    }
  }
  //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
  //    RF_nativePrint("\nrestoreNodeMembership() EXIT ...\n");
  //  }
  return bootResult;
}  
void imputeNodeAndSummarize(uint     r,
                            char     mode,
                            uint     treeID,
                            Node    *parent,
                            uint    *repMembrIndx,
                            uint     repMembrSize,
                            uint    *allMembrIndx,
                            uint     allMembrSize,
                            uint    *ngAllMembrIndx,
                            uint     ngAllMembrSize) {
  uint multImpFlag;
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeNodeAndSummarize(%2d) ENTRY ...\n", mode);
  //  }
  if (r == 1) {
    if (RF_mRecordSize > 0) {
      unstackNodeLMPIndex(parent);
      imputeNode(RF_GROW, 
                 TRUE,    
                 TRUE,    
                 treeID,
                 parent,
                 repMembrIndx,
                 repMembrSize,
                 allMembrIndx,
                 allMembrSize);
      if (mode != RF_PRED) {
        xferMissingness(RF_GROW, parent, parent -> mate);
      }
    }
    if (mode == RF_PRED) {
      if (RF_fmRecordSize > 0) {
        unstackNodeFLMPIndex(parent);
        imputeNode(RF_PRED, 
                   TRUE,    
                   FALSE,   
                   treeID,
                   parent,
                   repMembrIndx,
                   repMembrSize,
                   ngAllMembrIndx,
                   ngAllMembrSize);
        xferMissingness(RF_PRED, parent, parent -> mate);
      }
    }
  }
  else {
    multImpFlag = FALSE;
    if (r < RF_nImpute) {
      multImpFlag = TRUE;
    }
    else {
      if (RF_opt & OPT_IMPU_ONLY) {
        multImpFlag = TRUE;
      }
    }
    if (multImpFlag) {
      if (RF_mRecordSize > 0) {
        imputeNode(RF_GROW, 
                   TRUE,    
                   FALSE,   
                   treeID,
                   parent,
                   repMembrIndx,
                   repMembrSize,
                   allMembrIndx,
                   allMembrSize);
        xferMissingness(RF_GROW, parent, parent -> mate);
      }
    }
  }
  //  if (getTraceFlag(treeID) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeNodeAndSummarize(%2d) EXIT ...\n", mode);
  //  }
}
void imputeUpdateShadow (char      mode,
                         double  **shadowResponse,
                         double  **shadowPredictor) {
  uint     mRecordSize;
  uint    *mRecordIndex;
  uint     mpIndexSize;
  int    **mpSign;
  int     *mpIndex;
  double **outResponse;
  double **outPredictor;
  double  *valuePtr;
  double  *outputPtr;
  uint unsignedIndex;
  char outcomeFlag, predictorFlag;
  uint rspSize;
  uint i, p;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nimputeUpdateShadow(%2d) ENTRY ...\n", mode);
  //  }
  mRecordSize  = 0;     
  mRecordIndex = NULL;  
  mpIndexSize  = 0;     
  mpSign       = NULL;  
  mpIndex      = NULL;  
  outResponse  = NULL;  
  outPredictor = NULL;  
  valuePtr     = NULL;  
  outputPtr    = NULL;  
  unsignedIndex = 0;    
  switch (mode) {
  case RF_PRED:
    mRecordSize = RF_fmRecordSize;
    mRecordIndex = RF_fmRecordIndex;
    mpIndexSize = RF_fmpIndexSize;
    mpSign = RF_fmpSign;
    mpIndex = RF_fmpIndex;
    if (shadowResponse != NULL) {
      outResponse  = RF_sImputeResponsePtr;
    }
    if (shadowPredictor != NULL) {
      outPredictor = RF_sImputePredictorPtr;
    }
    rspSize = RF_frSize;
    break;
  default:
    mRecordSize = RF_mRecordSize;
    mRecordIndex = RF_mRecordIndex;
    mpIndexSize = RF_mpIndexSize;
    mpSign = RF_mpSign;
    mpIndex = RF_mpIndex;
    if (shadowResponse != NULL) {
      outResponse  = RF_sImputeResponsePtr;
    }
    if (shadowPredictor != NULL) {
      outPredictor = RF_sImputePredictorPtr;
    }
    rspSize = RF_ySize;
    break;
  }
  if (mRecordSize == 0) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Attempt to update shadow data with no missingness in mode:  %10d", mode);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nImputed Shadow Data:  (before transfer)");
  //    RF_nativePrint("\n       index   imputation -> \n");
  //    RF_nativePrint(  "          ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint(" %12d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n");
  //    for (i = 1; i <= mRecordSize; i++) {
  //      RF_nativePrint("%12d", mRecordIndex[i]);
  //      for (p = 1; p <= mpIndexSize; p++) {
  //        if (mpIndex[p] < 0) {
  //          if (shadowResponse != NULL) {
  //            RF_nativePrint(" %12.4f", shadowResponse[(uint) abs(mpIndex[p])][mRecordIndex[i]]);
  //          }
  //        }
  //        else {
  //          if (shadowPredictor != NULL) {
  //            RF_nativePrint(" %12.4f", shadowPredictor[(uint) mpIndex[p]][mRecordIndex[i]]);
  //          }
  //        }
  //      }
  //      RF_nativePrint("\n");
  //    }
  //  }
  for (p = 1; p <= mpIndexSize; p++) {
    if (mpIndex[p] < 0) {
      if (shadowResponse != NULL) {
        unsignedIndex = (uint) abs(mpIndex[p]);
        valuePtr = shadowResponse[(uint) abs(mpIndex[p])];
        outputPtr = outResponse[(uint) abs(mpIndex[p])];
        outcomeFlag = TRUE;
      }
      else {
        outcomeFlag = FALSE;
      }
      predictorFlag = FALSE;
    }
    else {
      if (shadowPredictor != NULL) {
        unsignedIndex = (uint) mpIndex[p] + rspSize;
        valuePtr = shadowPredictor[(uint) mpIndex[p]];
        outputPtr = outPredictor[(uint) mpIndex[p]];
        predictorFlag = TRUE;
      }
      else {
        predictorFlag = FALSE;
      }
      outcomeFlag = FALSE;
    }
    if ( (outcomeFlag && (shadowResponse != NULL))  || (predictorFlag && (shadowPredictor != NULL)) ) {
      for (i = 1; i <= mRecordSize; i++) {
        if (mpSign[unsignedIndex][i] == 1) {
          if (RF_nativeIsNaN(outputPtr[i])) {
            //            if (getTraceFlag(0) & MISS_MED_TRACE) {
            //              RF_nativePrint("\nShadow Data Receiving NA from S.E.X.P. Output:  ");
            //              RF_nativePrint("\n[indv, OUT/PRED] = [%10d, %10d] \n", mRecordIndex[i], mpIndex[p]);
            //            }
          }
          valuePtr[mRecordIndex[i]] = outputPtr[i];
        }
      }
    }  
  }  
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nImputed Shadow Data:  (after transfer)");
  //    RF_nativePrint("\n       index   imputation -> \n");
  //    RF_nativePrint(  "            ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint(" %12d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n");
  //    for (i = 1; i <= mRecordSize; i++) {
  //      RF_nativePrint("%12d", mRecordIndex[i]);
  //      for (p = 1; p <= mpIndexSize; p++) {
  //        if (mpIndex[p] < 0) {
  //          if (shadowResponse != NULL) {
  //            RF_nativePrint(" %12.4f", shadowResponse[(uint) abs(mpIndex[p])][mRecordIndex[i]]);
  //          }
  //        }
  //        else {
  //          if (shadowPredictor != NULL) {
  //            RF_nativePrint(" %12.4f", shadowPredictor[(uint) mpIndex[p]][mRecordIndex[i]]);
  //          }
  //        }
  //      }
  //      RF_nativePrint("\n");
  //    }
  //  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeUpdateShadow(%2d) EXIT ...\n", mode);
  //  }
}
void imputeSummary(char      mode,
                   char      selectionFlag) {
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeSummary(%2d) ENTRY ...\n", selectionFlag);
  //  }
  imputeCommon(mode,
               1,
               RF_ntree,
               RF_serialTreeIndex,
               selectionFlag,
               TRUE);
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeSummary(%2d) EXIT ...\n", selectionFlag);
  //  }
}
void imputeResponse(char      mode,
                    uint      loSerialTreeID,
                    uint      hiSerialTreeID,
                    uint     *serialTreePtr,
                    double  **tempResponse) {
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeResponse() ENTRY ...\n");
  //  }
  switch(mode) {
  case RF_PRED:
    imputeCommon(mode, loSerialTreeID, hiSerialTreeID, serialTreePtr, ACTIVE, FALSE);
    imputeUpdateShadow(mode, tempResponse, NULL);
    break;
  default:
    imputeCommon(mode, loSerialTreeID, hiSerialTreeID, serialTreePtr, FALSE, FALSE);
    imputeUpdateShadow(mode, tempResponse, NULL);
    break;
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeResponse() EXIT ...\n");
  //  }
}
void imputeCommon(char      mode,
                  uint      loSerialTreeID,
                  uint      hiSerialTreeID,
                  uint     *serialTreePtr,
                  char      selectionFlag,
                  char      predictorFlag) {
  uint  rgIdentifier;
  uint *overriddenSerialTreePtr;
  char mFlag;
  char outcomeFlag;
  uint     mRecordSize;
  uint    *mRecordIndex;
  uint     mpIndexSize;
  int    **mpSign;
  int     *mpIndex;
  double **outResponse;
  double **outPredictor;
  double *valuePtr;
  double *naivePtr;
  uint    unsignedSignatureIndex;
  Terminal ***termMembershipPtr;
  Terminal *info;
  double imputedValue;
  uint localDistributionSize;
  uint maxDistributionSize;
  uint rspSize;
  char result;
  uint i, p, v, tree;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nimputeCommon(%2d) ENTRY ...\n", selectionFlag);
  //  }
  //  uint    targetRecordIndex;
  valuePtr      = NULL;  
  naivePtr      = NULL;  
  unsignedSignatureIndex = 0;     
  maxDistributionSize = 0;  
  outResponse         = 0;  
  outPredictor        = 0;  
  rspSize = 0;  
  mpIndex = 0;  
  mpSign  = 0;  
  mpIndexSize  = 0;  
  mRecordIndex = 0;  
  mRecordSize  = 0;  
  termMembershipPtr = NULL;  
  if ((selectionFlag != TRUE) && (selectionFlag != FALSE) && (selectionFlag != ACTIVE)) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Invalid selectionFlag in imputeCommon():  %10d", selectionFlag);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  result = FALSE;
  switch (mode) {
  case RF_PRED:
    if (RF_fmRecordSize > 0) {
      mRecordSize = RF_fmRecordSize;
      mRecordIndex = RF_fmRecordIndex;
      mpIndexSize = RF_fmpIndexSize;
      mpSign = RF_fmpSign;
      mpIndex = RF_fmpIndex;
      maxDistributionSize = ((RF_observationSize) > (RF_ntree)) ? (RF_observationSize) : (RF_ntree);
      outResponse  = RF_sImputeResponsePtr;
      outPredictor = RF_sImputePredictorPtr;
      rspSize = RF_frSize;
      termMembershipPtr = RF_ftTermMembership;
      result = TRUE;
    }
    break;
  default:
    if (RF_mRecordSize > 0) {
      mRecordSize = RF_mRecordSize;
      mRecordIndex = RF_mRecordIndex;
      mpIndexSize = RF_mpIndexSize;
      mpSign = RF_mpSign;
      mpIndex = RF_mpIndex;
      maxDistributionSize = ((RF_observationSize) > (RF_ntree)) ? (RF_observationSize) : (RF_ntree);
      outResponse  = RF_sImputeResponsePtr;
      outPredictor = RF_sImputePredictorPtr;
      rspSize = RF_ySize;
      termMembershipPtr = RF_tTermMembership;
      result = TRUE;
    }
    break;
  }
  if (result == FALSE) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Attempt to impute in imputeCommon() with no missingness in mode:  %10d", mode);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  if ((loSerialTreeID == 1) && (hiSerialTreeID == RF_ntree)) {
    overriddenSerialTreePtr = RF_orderedTreeIndex;
  }
  else {
    overriddenSerialTreePtr = serialTreePtr;
  }
  rgIdentifier = hiSerialTreeID;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nOutputs (S.E.X.P.) (before update):  ");
  //    RF_nativePrint("\n       index   imputation -> \n");
  //    RF_nativePrint(  "            ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint(" %12d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n");
  //    for (i = 1; i <= mRecordSize; i++) {
  //      RF_nativePrint("%12d", mRecordIndex[i]);
  //      for (p = 1; p <= mpIndexSize; p++) {
  //        if (mpIndex[p] < 0) {
  //          RF_nativePrint(" %12.4f", outResponse[(uint) abs(mpIndex[p])][i]);
  //        }
  //        else {
  //          if (predictorFlag == TRUE) {
  //            RF_nativePrint(" %12.4f", outPredictor[(uint) mpIndex[p]][i]);
  //          }
  //        }
  //      }
  //      RF_nativePrint("\n");
  //    }
  //  }
  imputedValue = 0.0;  
  double *localDistribution = dvector(1, maxDistributionSize);
  char  *naiveFlag = cvector(1, mpIndexSize);
  char **naiveSign = cmatrix(1, mRecordSize, 1, mpIndexSize);
  for (p = 1; p <= mpIndexSize; p++) {
    naiveFlag[p] = FALSE;
  }
  for (i = 1; i <= mRecordSize; i++) {
    outcomeFlag = TRUE;
    for (p = 1; p <= mpIndexSize; p++) {
      naiveSign[i][p] = FALSE;
      if (mpIndex[p] < 0) {
        unsignedSignatureIndex = (uint) abs(mpIndex[p]);
      }
      else {
        if (predictorFlag == TRUE) {
          unsignedSignatureIndex = (uint) mpIndex[p] + rspSize;
        }
        outcomeFlag = FALSE;
      }
      if (outcomeFlag || predictorFlag) {
        if (mpSign[unsignedSignatureIndex][i] == 1) {
          localDistributionSize = 0;
          for (tree = loSerialTreeID; tree <= hiSerialTreeID; tree++) {
            if (RF_tLeafCount[overriddenSerialTreePtr[tree]] > 0) {
              if ((RF_dmRecordBootFlag[overriddenSerialTreePtr[tree]][i] == selectionFlag) || (selectionFlag == ACTIVE)) {
                // targetRecordIndex = mRecordIndex[i];
                info = termMembershipPtr[overriddenSerialTreePtr[tree]][mRecordIndex[i]];
                //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
                //    RF_nativePrint("\nElement (overriddenSerialTreePtr[tree], target, resp/covr, node) N_DOM:  (%10d, %10d, %10d, %20x, %10d)", overriddenSerialTreePtr[tree], targetRecordIndex, mpIndex[p], *info, info -> nodeID);
                //  }
                for (v = 1; v <= info -> lmiSize; v++) {
                  if ((info -> lmiIndex)[v] == p) {
                    if (!RF_nativeIsNaN((info -> lmiValue)[v])) {
                      localDistribution[++localDistributionSize] = (info -> lmiValue)[v];
                    }
                    else {
                      //                    if (getTraceFlag(0) & MISS_HGH_TRACE) {
                      //                      
                      //                      RF_nativePrint("\nRF-SRC:  Unimputed value, NA:  [overriddenSerialTreePtr[tree]][i][p] = [%10d][%10d][%10d] ", overriddenSerialTreePtr[tree], i, p);
                      //
                      //                      RF_nativePrint("\nTarget [indv, outcome/predictor] = [%10d][%10d] ", mRecordIndex[i], mpIndex[p]);
                      //                    }
                    }  
                    v = info -> lmiSize;
                  }
                }
              }  
            }  
            else {
            }
          }  
          if (localDistributionSize > 0) {
            if (mpIndex[p] < 0) {
              if (RF_rType[(uint) abs(mpIndex[p])] == 'T') {
                imputedValue = getMeanValue(localDistribution, localDistributionSize);
              }
              else if (RF_rType[(uint) abs(mpIndex[p])] == 'S') {
                imputedValue = getMaximalValue(localDistribution, localDistributionSize, FALSE, rgIdentifier);
              }
              else if (RF_rType[(uint) abs(mpIndex[p])] == 'B') {
                imputedValue = getMeanValue(localDistribution, localDistributionSize);
              }
              else if (RF_rType[(uint) abs(mpIndex[p])] == 'R') {
                imputedValue = getMeanValue(localDistribution, localDistributionSize);
              }
              else if (RF_rType[(uint) abs(mpIndex[p])] == 'I') {
                imputedValue = getMaximalValue(localDistribution, localDistributionSize, FALSE, rgIdentifier);
              }
              else if (RF_rType[(uint) abs(mpIndex[p])] == 'C') {
                imputedValue = getMaximalValue(localDistribution, localDistributionSize, FALSE, rgIdentifier);
              }
              outResponse[(uint) abs(mpIndex[p])][i] = imputedValue;
              //              if (getTraceFlag(0) & MISS_HGH_TRACE) {
              //                RF_nativePrint("\nSummary Response Imputed Value for:  ");
              //                RF_nativePrint("\n[indv, resp] = [%10d, %10d] \n", mRecordIndex[i], (uint) abs(mpIndex[p]));
              //                RF_nativePrint("%12.4f \n", outResponse[(uint) abs(mpIndex[p])][i]);
              //              }
            }  
            else {
              if (RF_xType[(uint) mpIndex[p]] == 'R') {
                imputedValue = getMeanValue(localDistribution, localDistributionSize);
              }
              else {
                imputedValue = getMaximalValue(localDistribution, localDistributionSize, FALSE, rgIdentifier);
              }
              outPredictor[(uint) mpIndex[p]][i] = imputedValue;
              //              if (getTraceFlag(0) & MISS_HGH_TRACE) {
              //                RF_nativePrint("\nSummary Predictor Imputed Value for:  ");
              //                RF_nativePrint("\n[indv, pred] = [%10d, %10d] \n", mRecordIndex[i], mpIndex[p]);
              //                RF_nativePrint("%12.4f \n", outPredictor[(uint) mpIndex[p]][i]);
              //              }
            }
          }  
          else {
            naiveFlag[p] = TRUE;
            naiveSign[i][p] = TRUE;
            //            if (getTraceFlag(0) & MISS_HGH_TRACE) {
            //              RF_nativePrint("\n[idx, mRecIdx, p, mvIdx] requiring naive imputation:  (%10d, %10d %10d %10d) ", i, mRecordIndex[i], p, mpIndex[p]);
            //            }
          }
        }  
      }  
      else {
        p = mpIndexSize;
      }
    }  
    //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
    //    
    //  }
  }  
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\n");
  //  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nOutputs (S.E.X.P.) (before naive imputation):  ");
  //    RF_nativePrint("\n       index   imputation -> \n");
  //    RF_nativePrint(  "            ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint(" %12d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n");
  //    for (i = 1; i <= mRecordSize; i++) {
  //      RF_nativePrint("%12d", mRecordIndex[i]);
  //      for (p = 1; p <= mpIndexSize; p++) {
  //        if (mpIndex[p] < 0) {
  //          RF_nativePrint(" %12.4f", outResponse[(uint) abs(mpIndex[p])][i]);
  //        }
  //        else {
  //          if (predictorFlag == TRUE) {
  //            RF_nativePrint(" %12.4f", outPredictor[(uint) mpIndex[p]][i]);
  //          }
  //        }
  //      }
  //      RF_nativePrint("\n");
  //    }
  //  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nNaive Imputation Flag of Outcomes and Predictors:  ");
  //    RF_nativePrint("\n     index flags -> \n");
  //    RF_nativePrint(  "          ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint("%4d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n          ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint("%4d", naiveFlag[p]);
  //    }
  //    for (i=1; i <= mRecordSize; i++) {
  //      RF_nativePrint("\n");
  //      RF_nativePrint("%10d", mRecordIndex[i]);
  //      for (p=1; p <= mpIndexSize; p++) {
  //        if (mpIndex[p] < 0) {
  //          RF_nativePrint("%4d", naiveSign[i][p]);
  //        }
  //        else {
  //          if (predictorFlag == TRUE) {
  //            RF_nativePrint("%4d", naiveSign[i][p]);
  //          }
  //        }
  //      }
  //    }
  //    RF_nativePrint("\n");
  //  }
  outcomeFlag = TRUE;
  for (p = 1; p <= mpIndexSize; p++) {
    if (mpIndex[p] < 0) {
      unsignedSignatureIndex = (uint) abs(mpIndex[p]);
      valuePtr = RF_responseIn[(uint) abs(mpIndex[p])];
      naivePtr = outResponse[(uint) abs(mpIndex[p])];
    }
    else {
      if (predictorFlag == TRUE) {
        unsignedSignatureIndex = (uint) mpIndex[p] + rspSize;
        valuePtr = RF_observationIn[(uint) mpIndex[p]];
        naivePtr = outPredictor[(uint) mpIndex[p]];
      }
      outcomeFlag = FALSE;
    }
    if (outcomeFlag || predictorFlag) {
      if (naiveFlag[p] == TRUE) {
        localDistributionSize = 0;
        for (i=1; i <= RF_observationSize; i++) {
          mFlag = TRUE;
          if (RF_mRecordMap[i] == 0) {
            mFlag = FALSE;
          }
          else if (RF_mpSign[unsignedSignatureIndex][RF_mRecordMap[i]] == 0) {
            mFlag = FALSE;
          }
          if (mFlag == FALSE) {
            localDistribution[++localDistributionSize] = valuePtr[i];
          }
        }  
        if (localDistributionSize > 0) {
          for (i=1; i <= mRecordSize; i++) {
            if (naiveSign[i][p] == TRUE) {
              naivePtr[i] = getSampleValue(localDistribution, localDistributionSize, FALSE, rgIdentifier);
              //              if (getTraceFlag(0) & MISS_HGH_TRACE) {
              //                RF_nativePrint("\nSummary Imputed Value for:  ");
              //                RF_nativePrint("\n[indv, outcome/pred] = [%10d, %10d] \n", mRecordIndex[i], mpIndex[p]);
              //                RF_nativePrint("%12.4f \n", naivePtr[i]);
              //              }
            }
          }
        }  
        else {
          //          if (getTraceFlag(0) & MISS_HGH_TRACE) {
          //            RF_nativePrint("\nNaive imputation failed for outcome/predictor:  %10d", mpIndex[p]);
          //          }
          if (mpIndex[p] < 0) {
            RF_nativeError("\nRF-SRC:  *** ERROR *** ");
            RF_nativeError("\nRF-SRC:  Naive imputation failed for [indv, outcome] = [%10d, %10d] \n", mRecordIndex[i], mpIndex[p]);
            RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
            RF_nativeExit();
          }
          else {
          }
        }
      }  
    }  
    else {
      p = mpIndexSize;
    }
  }  
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nOutputs (S.E.X.P.) (after naive imputation):  ");
  //    RF_nativePrint("\n        index   imputation -> \n");
  //    RF_nativePrint(  "             ");
  //    for (p=1; p <= mpIndexSize; p++) {
  //      RF_nativePrint(" %12d", mpIndex[p]);
  //    }
  //    RF_nativePrint("\n");
  //    for (i = 1; i <= mRecordSize; i++) {
  //      RF_nativePrint(" %12d", mRecordIndex[i]);
  //      for (p = 1; p <= mpIndexSize; p++) {
  //        if (mpIndex[p] < 0) {
  //          RF_nativePrint(" %12.4f", outResponse[(uint) abs(mpIndex[p])][i]);
  //        }
  //        else {
  //          if (predictorFlag == TRUE) {
  //            RF_nativePrint(" %12.4f", outPredictor[(uint) mpIndex[p]][i]);
  //          }
  //        }
  //      }
  //      RF_nativePrint("\n");
  //    }
  //  }
  free_dvector(localDistribution, 1, maxDistributionSize);
  free_cvector(naiveFlag, 1, mpIndexSize);
  free_cmatrix(naiveSign, 1, mRecordSize, 1, mpIndexSize);
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nimputeCommon() EXIT ...\n");
  //  }
}
void imputeMultipleTime (char selectionFlag) {
  double  *outTime;
  char     result;
  uint i;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nimputeMultipleTime() ENTRY ...\n");
  //  }
  result = FALSE;
    if (RF_timeIndex > 0) {
      if (RF_mRecordSize > 0) {
      if (RF_mTimeFlag == TRUE) {
        result = TRUE;
      }
      else {
        RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        RF_nativeError("\nRF-SRC:  Attempt to multiply impute time with no missingness in time vector.");
      }
    }
  }
  else {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Attempt to multiply impute time in a !SURV environment.");
  }
  if (result == FALSE) {
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  outTime  = RF_sImputeResponsePtr[RF_timeIndex];
  for (i=1; i <= RF_mRecordSize; i++) {
    if(RF_mpSign[RF_timeIndex][i] == 1) {
      //        if (getTraceFlag(0) & MISS_LOW_TRACE) {
      //          RF_nativePrint("\nSummary time needed for missing record idx:  %10d", i);
      //        }
      outTime[i] = getNearestMasterTime(outTime[i], FALSE, 1);
    }
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nimputeMultipleTime EXIT ...\n");
  //  }
}
double getNearestMasterTime (double   meanValue,
                             char     chainFlag,
                             uint     treeID) {
  double leftDistance, rightDistance;
  uint minimumIndex;
  uint j;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\ngetNearestMasterTime() ENTRY ...\n");
  //  }
    leftDistance = meanValue - RF_masterTime[1];
    rightDistance = RF_masterTime[RF_masterTimeSize] - meanValue;
    if ( ((leftDistance > EPSILON) || (fabs(leftDistance) < EPSILON)) &&
         ((rightDistance > EPSILON) || (fabs(rightDistance) < EPSILON)) ) {
    }
    else {
      RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      RF_nativeError("\nRF-SRC:  The summary mean value for time is out of range:  %12.4f <= %12.4f <= %12.4f", RF_masterTime[1], meanValue, RF_masterTime[RF_masterTimeSize]);
      RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      RF_nativeExit();
    }
  leftDistance = rightDistance = 0;
  minimumIndex = RF_masterTimeSize;
  for (j = 1; j <= RF_masterTimeSize; j++) {
    if (meanValue <= RF_masterTime[j]) {
      minimumIndex = j;
      j = RF_masterTimeSize;
    }
  }
  if (minimumIndex == 1) {
  }
  else {
    leftDistance = meanValue - RF_masterTime[minimumIndex-1];
    rightDistance = RF_masterTime[minimumIndex] - meanValue;
    if (leftDistance < rightDistance) {
      minimumIndex = minimumIndex - 1;
    }
    else {
      if (fabs(leftDistance - rightDistance) < EPSILON) {
        //  if (getTraceFlag(treeID) & RAND_DEF_TRACE) {
        //    if(chainFlag) {
        //      RF_nativePrint("\nSample value ran1A():  %20d", randomGetChain(treeID));
        //    }
        //    else {
        //      RF_nativePrint("\nSample value ran1B():  %20d", randomGetUChain(treeID));
        //    }
        //  }
        if(chainFlag) {
          if (ran1A(treeID) <= 0.5) {
            minimumIndex = minimumIndex - 1;
          }
        }
        else {
          if (ran1B(treeID) <= 0.5) {
            minimumIndex = minimumIndex - 1;
          }
        }
      }
    }
  }
  //        if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //          if ((leftDistance > 0) && (rightDistance > 0)) {
  //            RF_nativePrint("\nSummary time overridden with nearest neighbour for individual at master time idx %10d ", minimumIndex);
  //            RF_nativePrint("\nSummary -> NearestNhbr:  %12.4f -> %12.4f  \n", meanValue, RF_masterTime[minimumIndex]);
  //          }
  //        }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\ngetNearestMasterTime() EXIT ...\n");
  //  }
  return RF_masterTime[minimumIndex];
}
double getMaximalValue(double *value, uint size, char chainFlag, uint treeID) {
  double result;
  uint classCount, maximalClassSize, maximalClassCount;
  uint randomIndex;
  uint j;
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetMaximalValue() ENTRY ...\n");
  //  }
  uint   *classSize  = uivector(1, size);
  for (j = 1; j <= size; j++) {
    classSize[j] = 0;
  }
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMaximal Class Values (raw):  ");
  //    RF_nativePrint("\n     index       value \n");
  //    for (j=1; j <= size; j++) {
  //      RF_nativePrint("%10d %10d \n", j, (int) value[j]);
  //    }
  //  }
  hpsort(value, size);
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMaximal Class Values (sorted):  ");
  //    RF_nativePrint("\n     index       value \n");
  //    for (j=1; j <= size; j++) {
  //      RF_nativePrint("%10d %10d \n", j, (int) value[j]);
  //    }
  //  }
  classCount = 1;
  classSize[1] = 1;
  for (j = 2; j <= size; j++) {
    if (value[j] > value[classCount]) {
      classCount ++;
      value[classCount] = value[j];
    }
    classSize[classCount] ++;
  }
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMaximal Class Values (summary):  ");
  //    RF_nativePrint("\n      size      value \n");
  //    for (j=1; j <= classCount; j++) {
  //      RF_nativePrint("%10d %10d \n", classSize[j], (int) value[j]);
  //    }
  //  }
  maximalClassSize = maximalClassCount = 0;
  for (j=1; j <= classCount; j++) {
    if (classSize[j] > maximalClassSize) {
      maximalClassSize = classSize[j];
    }
  }
  for (j=1; j <= classCount; j++) {
    if (classSize[j] == maximalClassSize) {
      maximalClassCount ++;
    }
  }
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMaximal Class (size, number):  ");
  //    RF_nativePrint("\n(%10d, %10d) \n", maximalClassSize, maximalClassCount);
  //  }
  if (maximalClassCount > 1) {
    //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
    //    RF_nativePrint("\nUsing generator to break class size tie. ");
    //  }
    //  if (getTraceFlag(treeID) & RAND_DEF_TRACE) {
    //    if(chainFlag) {
    //      RF_nativePrint("\nMaximal value ran1A():  %20d", randomGetChain(treeID));
    //    }
    //    else {
    //      RF_nativePrint("\nMaximal value ran1B():  %20d", randomGetUChain(treeID));
    //    }
    //  }
    if(chainFlag) {
      randomIndex = (uint) ceil(ran1A(treeID)*((maximalClassCount)*1.0));
    }
    else {
      randomIndex = (uint) ceil(ran1B(treeID)*((maximalClassCount)*1.0));
    }
  }
  else {
    randomIndex = 1;
  }
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMaximal Class Random Index:  ");
  //    RF_nativePrint("\n%10d \n", randomIndex);
  //  }
  j = 0;
  while (randomIndex > 0) {
    j++;
    if (classSize[j] == maximalClassSize) {
      randomIndex --;
    }
  }
  result = value[j];
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMaximal Class Value:  ");
  //    RF_nativePrint("\n%10d \n", (int) result);
  //  }
  free_uivector(classSize, 1, size);
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetMaximalValue() EXIT ...\n");
  //  }
  return result;
}
double getMedianValue(double *value, uint size) {
  double result;
  uint medianIndex;
  //  uint j;
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetMedianValue() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMedian Values (raw):  \n");
  //    for (j=1; j <= size; j++) {
  //      RF_nativePrint("%10d %12.4f \n", j, value[j]);
  //    }
  //  }
  qksort(value, size);
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMedian Values (sorted):  \n");
  //    for (j=1; j <= size; j++) {
  //      RF_nativePrint("%10d %12.4f \n", j, value[j]);
  //    }
  //  }
  if (size > 1) {
    medianIndex = (uint) ceil(size/2);
  }
  else {
    medianIndex = 1;
  }
  result = value[medianIndex];
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMedian Index Index:  ");
  //    RF_nativePrint("\n%10d \n", medianIndex);
  //    RF_nativePrint("\nMedian Value:  ");
  //    RF_nativePrint("\n%12.4f \n", result);
  //  }
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetMedianValue() EXIT ...\n");
  //  }
  return result;
}
double getMeanValue(double *value, uint size) {
  double result;
  uint j;
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetMeanValue() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMean Values (raw):  \n");
  //    for (j=1; j <= size; j++) {
  //      RF_nativePrint("%10d %12.4f \n", j, value[j]);
  //    }
  //  }
  result = 0.0;
  for (j = 1; j <= size; j++) {
    result = result + value[j];
  }
  result = result / size;
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nMean Value:  \n");
  //    RF_nativePrint("%12.4f \n", result);
  //  }
  //  if (getTraceFlag(0) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetMeanValue() EXIT ...\n");
  //  }
  return result;
}
double getSampleValue(double *value, uint size, char chainFlag, uint treeID) {
  uint randomIndex;
  //  uint j;
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetSampleValue() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\nSample Values (raw):  \n");
  //    for (j=1; j <= size; j++) {
  //      RF_nativePrint("%10d %12.4f \n", j, value[j]);
  //    }
  //  }
  //  if (getTraceFlag(treeID) & RAND_DEF_TRACE) {
  //    if(chainFlag) {
  //      RF_nativePrint("\nSample value ran1A():  %20d", randomGetChain(treeID));
  //    }
  //    else {
  //      RF_nativePrint("\nSample value ran1B():  %20d", randomGetUChain(treeID));
  //    }
  //  }
  if(chainFlag) {
    randomIndex = (uint) ceil(ran1A(treeID)*((size)*1.0));
  }
  else {
    randomIndex = (uint) ceil(ran1B(treeID)*((size)*1.0));
  }
  //  if (getTraceFlag(treeID) & MISS_HGH_TRACE) {
  //    RF_nativePrint("\ngetSampleValue() EXIT ...\n");
  //  }
  return value[randomIndex];
}
uint getRecordMap(uint    *map,
                  uint     obsSize,
                  double **resp,
                  double **data) {
  uint i, p, r;
  uint mSize;
  char mFlag;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\ngetRecordMap() ENTRY ...\n");
  //  }
  mSize  = 0;
  for (i = 1; i <= obsSize; i++) {
    mFlag = FALSE;
    if (resp != NULL) {
      for (r = 1; r <= RF_ySize; r++) {
        if (RF_nativeIsNaN(resp[r][i])) {
          mFlag = TRUE;
          r = RF_ySize;
        }
      }
    }
    if (mFlag == FALSE) {
      for (p = 1; p <= RF_xSize; p++) {
        if (RF_nativeIsNaN(data[p][i])) {
          mFlag = TRUE;
          p = RF_xSize;
        }
      }
    }
    if (mFlag == TRUE) {
      mSize ++;
      map[i] = mSize;
    }
    else {
      map[i] = 0;
    }
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nMapping of Individuals to Missing Record Index:  ");
  //    RF_nativePrint("\n  orgIndex     mIndex \n");
  //    for (i = 1; i <= obsSize; i++) {
  //      if (map[i] != 0) {
  //        RF_nativePrint("%10d %10d \n", i, map[i]);
  //      }
  //    }
  //  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\ngetRecordMap() EXIT ...\n");
  //  }
  return mSize;
}
void updateTimeIndexArray(uint    treeID,
                          uint   *allMembrIndx,
                          uint    allMembrSize,
                          double *time,
                          char    naAllowFlag,
                          char    noIdxAllowFlag,
                          uint   *masterTimeIndex) {
  uint *membrIndx;
  char idxFoundFlag;
  uint i,k;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nupdateTimeIndexArray() ENTRY ...\n");
  //  }
  if (allMembrIndx == NULL) {
    membrIndx = uivector(1, allMembrSize);
    for (i = 1; i <= allMembrSize; i++) {
      membrIndx[i] = i;
    }
  }
  else {
    membrIndx = allMembrIndx;
  }
  for (i=1; i <= allMembrSize; i++) {
    idxFoundFlag = FALSE;
    if (!RF_nativeIsNaN(time[membrIndx[i]])) {
      k = 1;
      while (k <= RF_masterTimeSize) {
        if (time[membrIndx[i]] == RF_masterTime[k]) {
          masterTimeIndex[membrIndx[i]] = k;
          idxFoundFlag = TRUE;
          k = RF_masterTimeSize;
        }
        k++;
      }
    }
    else {
      if (naAllowFlag == FALSE) {
        RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        RF_nativeError("\nRF-SRC:  Missing event time encountered for individual:  %10d, %12.4f", i, time[membrIndx[i]]);
        RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        RF_nativeExit();
      }
      else {
        masterTimeIndex[membrIndx[i]] = 0;
        idxFoundFlag = TRUE;
      }
    }
    if (idxFoundFlag == FALSE) {
      if (noIdxAllowFlag == FALSE) {
        RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        RF_nativeError("\nRF-SRC:  Invalid event time encountered for individual:  %10d, %12.4f", i, time[membrIndx[i]]);
        RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        RF_nativeExit();
      }
      else {
        masterTimeIndex[membrIndx[i]] = 0;
      }
    }
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nMaster Time Index at node:  \n");
  //      RF_nativePrint("\n       Indv      Index         Time:  \n");
  //      for (i = 1; i <= allMembrSize; i++) {
  //        if(masterTimeIndex[membrIndx[i]] > 0) {
  //          RF_nativePrint(" %10d %10d %12.4f \n", i, masterTimeIndex[membrIndx[i]], RF_masterTime[masterTimeIndex[membrIndx[i]]]);
  //        }
  //        else {
  //          RF_nativePrint(" %10d %10d           XX \n", i, masterTimeIndex[membrIndx[i]]);
  //        }
  //      }
  //  }
  if (allMembrIndx == NULL) {
    free_uivector(membrIndx, 1, allMembrSize);
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\nupdateTimeIndexArray() EXIT ...\n");
  //  }
}
void updateEntryTimeIndexArray(uint    treeID,
                               uint   *allMembrIndx,
                               uint    allMembrSize,
                               double *entryTime,
                               uint   *masterEntryTimeIndex) {
  uint *membrIndx;
  uint i, k;
  if (allMembrIndx == NULL) {
    membrIndx = uivector(1, allMembrSize);
    for (i = 1; i <= allMembrSize; i++) {
      membrIndx[i] = i;
    }
  }
  else {
    membrIndx = allMembrIndx;
  }
  for (i = 1; i <= allMembrSize; i++) {
    k = 1;
    while ((k <= RF_masterTimeSize) && (RF_masterTime[k] < entryTime[membrIndx[i]])) {
      k++;
    }
    masterEntryTimeIndex[membrIndx[i]] = k;
  }
  if (allMembrIndx == NULL) {
    free_uivector(membrIndx, 1, allMembrSize);
  }
}
void updateEventTypeSubsets(double *summaryStatus,
                            uint    mRecordSize,
                            int   **mpSign,
                            uint   *mRecordIndex,
                            uint   *meIndividualSize,
                            uint  **eIndividual) {
  uint i, j;
  //  uint n;
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nRF-SRC:  updateEventTypeSubsets() ENTRY ...\n");
  //  }
  if (RF_eventTypeSize == 1) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Attempt to update event type subsets in a non-CR analysis.");
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  for (j = 1; j <= RF_eventTypeSize; j++) {
    for (i = 1; i <= RF_eIndividualSize[j]; i++) {
      eIndividual[j][i] = RF_eIndividualIn[j][i];
    }
  }
  if (RF_mStatusSize > 0) {
    uint *eventCounter = uivector(1, RF_eventTypeSize);
    for (j = 1; j <= RF_eventTypeSize; j++) {
      eventCounter[j] = RF_eIndividualSize[j];
    }
    for (i = 1; i <= mRecordSize; i++) {
      if (mpSign[RF_statusIndex][i] == 1) {
        if ((uint) summaryStatus[mRecordIndex[i]] > 0) {
          j = RF_eventTypeIndex[(uint) summaryStatus[mRecordIndex[i]]];
          eventCounter[j] ++;
          eIndividual[j][eventCounter[j]] = mRecordIndex[i];
        }
        else {
          for (j=1; j <= RF_eventTypeSize; j++) {
            eventCounter[j] ++;
            eIndividual[j][eventCounter[j]] = mRecordIndex[i];
          }
        }
      }
    }
    for (j = 1; j <= RF_eventTypeSize; j++) {
      meIndividualSize[j] = eventCounter[j];
    }
    free_uivector(eventCounter, 1, RF_eventTypeSize);
  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nEvent Type Subset Sizes:  \n");
  //    for (j=1; j <= RF_eventTypeSize; j++) {
  //      RF_nativePrint("%10d %10d %10d \n", j, RF_eventType[j], meIndividualSize[j]);
  //    }
  //    RF_nativePrint("\nEvent Type Subsets:  \n");
  //    RF_nativePrint("          ");
  //    for (n=1; n <= RF_observationSize; n++) {
  //      RF_nativePrint("%10d", n);
  //    }
  //    RF_nativePrint("\n");
  //    for (j=1; j <= RF_eventTypeSize; j++) {
  //      RF_nativePrint("%10d", j);
  //      for (n=1; n <= meIndividualSize[j]; n++) {
  //        RF_nativePrint("%10d", eIndividual[j][n]);
  //      }
  //      RF_nativePrint("\n");
  //    }
  //  }
  //  if (getTraceFlag(0) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nRF-SRC:  updateEventTypeSubsets() EXIT ...\n");
  //  }
}
void stackShadow (char mode, uint treeID) {
  uint i, p;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nstackShadow() ENTRY ...\n");
  //  }
  if (RF_mResponseFlag == TRUE) {
    RF_response[treeID] = (double **) new_vvector(1, RF_ySize, NRUTIL_DPTR);
    for (p = 1; p <= RF_ySize; p++) {
      RF_response[treeID][p] = RF_responseIn[p];
    }
    for (p = 1; p <= RF_mpIndexSize; p++) {
      if (RF_mpIndex[p] < 0) {
        RF_response[treeID][(uint) abs(RF_mpIndex[p])] = dvector(1, RF_observationSize);
        for (i = 1; i <= RF_observationSize; i++) {
          RF_response[treeID][(uint) abs(RF_mpIndex[p])][i] = RF_responseIn[(uint) abs(RF_mpIndex[p])][i];
        }
      }
      else {
        p = RF_mpIndexSize;
      }
    }
    if (RF_timeIndex > 0) {
      RF_time[treeID] = RF_response[treeID][RF_timeIndex];
      if (RF_mTimeFlag == TRUE) {
        RF_masterTimeIndex[treeID] = uivector(1, RF_observationSize);
        for (i = 1; i <= RF_observationSize; i++) {
          RF_masterTimeIndex[treeID][i] = RF_masterTimeIndexIn[i];
        }
      }
      else {
        RF_masterTimeIndex[treeID] = RF_masterTimeIndexIn;
      }
    }
    if (RF_statusIndex > 0) {
      RF_status[treeID] =  RF_response[treeID][RF_statusIndex];
    }
  }
  else {
  }
  if (mode == RF_PRED) {
    if (RF_frSize > 0) {
      if (RF_fmResponseFlag == TRUE) {
        RF_fresponse[treeID] = (double **) new_vvector(1, RF_ySize, NRUTIL_DPTR);
        for (p = 1; p <= RF_frSize; p++) {
          RF_fresponse[treeID][p] = RF_fresponseIn[p];
        }
        for (p = 1; p <= RF_fmpIndexSize; p++) {
          if (RF_fmpIndex[p] < 0) {
            RF_fresponse[treeID][(uint) abs(RF_fmpIndex[p])] = dvector(1, RF_fobservationSize);
            for (i = 1; i <= RF_fobservationSize; i++) {
              RF_fresponse[treeID][(uint) abs(RF_fmpIndex[p])][i] = RF_fresponseIn[(uint) abs(RF_fmpIndex[p])][i];
            }
          }
          else {
            p = RF_fmpIndexSize;
          }
        }
      }
      else {
      }
    }
  }
  if(RF_mPredictorFlag == TRUE) {
    RF_observation[treeID] = (double **) new_vvector(1, RF_xSize, NRUTIL_DPTR);
    for (p = 1; p <= RF_xSize; p++) {
      RF_observation[treeID][p] = RF_observationIn[p];
    }
    for (p = 1; p <= RF_mpIndexSize; p++) {
      if (RF_mpIndex[p] > 0) {
        RF_observation[treeID][(uint) RF_mpIndex[p]] = dvector(1, RF_observationSize);
        for (i = 1; i <= RF_observationSize; i++) {
          RF_observation[treeID][(uint) RF_mpIndex[p]][i] = RF_observationIn[(uint) RF_mpIndex[p]][i];
        }
      }
    }
  }
  else {
  }
  if (mode == RF_PRED) {
    if(RF_fmPredictorFlag == TRUE) {
      RF_fobservation[treeID] = (double **) new_vvector(1, RF_xSize, NRUTIL_DPTR);
      for (p = 1; p <= RF_xSize; p++) {
        RF_fobservation[treeID][p] = RF_fobservationIn[p];
      }
      for (p = 1; p <= RF_fmpIndexSize; p++) {
        if (RF_fmpIndex[p] > 0) {
          RF_fobservation[treeID][(uint) RF_fmpIndex[p]] = dvector(1, RF_fobservationSize);
          for (i = 1; i <= RF_fobservationSize; i++) {
            RF_fobservation[treeID][(uint) RF_fmpIndex[p]][i] = RF_fobservationIn[(uint) RF_fmpIndex[p]][i];
          }
        }
      }
    }
    else {
    }
  }  
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //      RF_nativePrint("\nShadow train Data for Tree:  %10d", treeID);
  //      RF_nativePrint("\n       index");
  //      if (RF_ySize > 0) {
  //        RF_nativePrint("  outc/resp->");
  //        for (p = 1; p <= RF_ySize - 1; p++) {
  //          RF_nativePrint(" %12s", " ");
  //        }
  //      }
  //      RF_nativePrint(" predictors->");
  //      for (p = 1; p <= RF_xSize - 1; p++) {
  //        RF_nativePrint(" %12s", " ");
  //      }
  //      RF_nativePrint("\n            ");
  //      if (RF_ySize > 0) {
  //        for (p = 1; p <= RF_ySize; p++) {
  //          RF_nativePrint(" %12c", RF_rType[p]);
  //        }
  //      }
  //      for (p = 1; p <= RF_xSize; p++) {
  //        RF_nativePrint(" %12d", p);
  //      }
  //      RF_nativePrint("\n");
  //      for (i = 1; i <= RF_observationSize; i++) {
  //        RF_nativePrint("%12d", i);
  //        if (RF_ySize > 0) {
  //          for (p = 1; p <= RF_ySize; p++) {
  //            RF_nativePrint(" %12.4f", RF_response[treeID][p][i]);
  //          }
  //        }
  //        for (p = 1; p <= RF_xSize; p++) {
  //          RF_nativePrint(" %12.4f", RF_observation[treeID][p][i]);
  //        }
  //        RF_nativePrint("\n");
  //      }
  //    }
  //  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nstackShadow() EXIT ...\n");
  //  }
}
void unstackShadow (char mode, uint treeID) {
  uint p;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nunstackShadow() ENTRY ...\n");
  //  }
  if (RF_mResponseFlag == TRUE) {
    for (p = 1; p <= RF_mpIndexSize; p++) {
      if (RF_mpIndex[p] < 0) {
        free_dvector(RF_response[treeID][(uint) abs(RF_mpIndex[p])], 1, RF_observationSize);
      }
      else {
        p = RF_mpIndexSize;
      }
    }
    free_new_vvector(RF_response[treeID], 1, RF_ySize, NRUTIL_DPTR);
    if (RF_timeIndex > 0) {
      if (RF_mTimeFlag == TRUE) {
        free_uivector(RF_masterTimeIndex[treeID], 1, RF_observationSize);
      }
    }
  }
  if (mode == RF_PRED) {
    if (RF_frSize > 0) {
      if (RF_fmResponseFlag == TRUE) {
        for (p = 1; p <= RF_fmpIndexSize; p++) {
          if (RF_fmpIndex[p] < 0) {
            free_dvector(RF_fresponse[treeID][(uint) abs(RF_fmpIndex[p])], 1, RF_fobservationSize);
          }
          else {
            p = RF_fmpIndexSize;
          }
        }
        free_new_vvector(RF_fresponse[treeID], 1, RF_ySize, NRUTIL_DPTR);
      }
    }
  }
  if(RF_mPredictorFlag == TRUE) {
    for (p = 1; p <= RF_mpIndexSize; p++) {
      if (RF_mpIndex[p] > 0) {
        free_dvector(RF_observation[treeID][(uint) RF_mpIndex[p]], 1, RF_observationSize);
      }
    }
    free_new_vvector(RF_observation[treeID], 1, RF_xSize, NRUTIL_DPTR);
  }
  if (mode == RF_PRED) {
    if(RF_fmPredictorFlag == TRUE) {
      for (p = 1; p <= RF_fmpIndexSize; p++) {
        if (RF_fmpIndex[p] > 0) {
          free_dvector(RF_fobservation[treeID][(uint) RF_fmpIndex[p]], 1, RF_fobservationSize);
        }
      }
      free_new_vvector(RF_fobservation[treeID], 1, RF_xSize, NRUTIL_DPTR);
    }
  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nunstackShadow() EXIT ...\n");
  //  }
}
char xferMissingness(char mode, Node *source, Terminal *destination) {
  uint   *sourceIndexPtr;
  double *sourceValuePtr;
  uint   *sourceAllocSizePtr;
  uint   *sourceActualSizePtr;
  char result;
  char xferFlag;
  //  int     *mpIndex;
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nxferMissingness(%2d) ENTRY ...\n", mode);
  //  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    RF_nativePrint("\n Node ID:  %10d", source -> nodeID);
  //  }
  //  mpIndex = NULL;
  sourceIndexPtr = NULL;  
  sourceValuePtr = NULL;  
  sourceAllocSizePtr = NULL;  
  sourceActualSizePtr = NULL;  
  result = FALSE;
  switch (mode) {
  case RF_PRED:
    if (RF_fmRecordSize > 0) {
      result = TRUE;
      // mpIndex = RF_fmpIndex;
      sourceIndexPtr = source -> flmpIndex;
      sourceValuePtr = source -> flmpValue;
      sourceAllocSizePtr = & (source -> flmpIndexAllocSize);
      sourceActualSizePtr = & (source -> flmpIndexActualSize);
    }
    break;
  default:
    if (RF_mRecordSize > 0) {
      result = TRUE;
      // mpIndex = RF_mpIndex;
      sourceIndexPtr = source -> lmpIndex;
      sourceValuePtr = source -> lmpValue;
      sourceAllocSizePtr = & (source -> lmpIndexAllocSize);
      sourceActualSizePtr = & (source -> lmpIndexActualSize);
    }
    break;
  }
  if (result == FALSE) {
    RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    RF_nativeError("\nRF-SRC:  Attempt to update forest impute data with no missingness in mode:  %10d", mode);
    RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    RF_nativeExit();
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    uint p;
  //    RF_nativePrint("\nSource LMV Vector:    ");
  //    RF_nativePrint("\nCount:    ");
  //    for (p = 1; p <= source -> lmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10d", p);
  //    }
  //    RF_nativePrint("\nIndex:    ");
  //    for (p = 1; p <= source -> lmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10d", (source -> lmpIndex)[p]);
  //    }
  //    RF_nativePrint("\nActual:   ");
  //    for (p = 1; p <= source -> lmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10d", mpIndex[(source -> lmpIndex)[p]]);
  //    }
  //    RF_nativePrint("\nValue:    ");
  //    for (p = 1; p <= source -> lmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10.4f", (source -> lmpValue)[p]);
  //    }
  //    RF_nativePrint("\nFLMV Vector:    ");
  //    RF_nativePrint("\nCount:    ");
  //    for (p = 1; p <= source -> flmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10d", p);
  //    }
  //    RF_nativePrint("\nIndex:    ");
  //    for (p = 1; p <= source -> flmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10d", (source -> flmpIndex)[p]);
  //    }
  //    RF_nativePrint("\nActual:   ");
  //    for (p = 1; p <= source -> flmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10d", mpIndex[(source -> flmpIndex)[p]]);
  //    }
  //    RF_nativePrint("\nValue:    ");
  //    for (p = 1; p <= source -> flmpIndexActualSize; p++) {
  //      RF_nativePrint(" %10.4f", (source -> flmpValue)[p]);
  //    }
  //  }
  if (*sourceActualSizePtr > 0) {
    (destination -> lmiIndex) = sourceIndexPtr;
    (destination -> lmiValue) = sourceValuePtr;
    (destination -> lmiSize)  = *sourceActualSizePtr;
    (destination -> lmiAllocSize)  = *sourceAllocSizePtr;    
    sourceIndexPtr = NULL;
    sourceValuePtr = NULL;
    *sourceAllocSizePtr  = 0;
    *sourceActualSizePtr  = 0;
    xferFlag = TRUE;
    //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
    //    RF_nativePrint("\nLMI xfer alloc size:   %10d", destination -> lmiAllocSize);
    //    RF_nativePrint("\nLMI xfer actual size:  %10d", destination -> lmiSize);
    //  }
  }
  else {
    xferFlag = FALSE;
  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //    uint p;
  //    RF_nativePrint("\nDestination LMI Vector:    ");
  //    RF_nativePrint("\nCount:    ");
  //    for (p = 1; p <= destination -> lmiSize; p++) {
  //      RF_nativePrint(" %10d", p);
  //    }
  //    RF_nativePrint("\nIndex:    ");
  //    for (p = 1; p <= destination -> lmiSize; p++) {
  //      RF_nativePrint(" %10d", (destination -> lmiIndex)[p]);
  //    }
  //    RF_nativePrint("\nValue:    ");
  //    for (p = 1; p <= destination -> lmiSize; p++) {
  //      RF_nativePrint(" %10.4f", (destination -> lmiValue)[p]);
  //    }
  //  }
  //  if (getTraceFlag(0) & MISS_LOW_TRACE) {
  //      RF_nativePrint("\nxferMissingness(%2d) EXIT ...\n", mode);
  //  }
  return xferFlag;
}
