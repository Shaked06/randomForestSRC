
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "treeJIT.h"
#include "nodeOps.h"
#include "nrutil.h"
#include "splitGreedy.h"
#include "polarity.h"
#include "treeUtil.h"
#include "tree.h"
#include "processEnsemble.h"
#include "bootstrap.h"
#include "rfsrcUtil.h"
// #include "error.h"
void acquireTreeJIT(char mode, uint r, uint treeID) {
  uint *bootMembrIndx;
  uint bootMembrSize;
  uint *membershipIndex;
  uint membershipSize;
  char result;
  uint i, ii;
  //  if (getTraceFlag(treeID) & SUMM_LOW_TRACE) {
  //    RF_nativePrint("\n\nStart of JIT acquisition:  %10d", treeID);
  //  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\nTree ran1A():  %20d", randomGetChain(treeID));
  //    RF_nativePrint("\nTree ran1B():  %20d", randomGetUChain(treeID));
  //  }
#ifdef _OPENMP
  //  if (getTraceFlag(treeID) & SUMM_LOW_TRACE) {
  //    RF_nativePrint("\n Hello from thread %d of %d", omp_get_thread_num(), omp_get_num_threads());
  //    RF_nativePrint("\n Parallel treeID:  %10d ", treeID);
  //  }
#endif
  //    if (getTraceFlag(treeID) & SUMM_LOW_TRACE) {
  //      RF_nativePrint("\nAllocating for treeID:  %10d", treeID);
  //    }
  RF_root[treeID] = makeNode(0);
  RF_root[treeID] -> parent = NULL;
  RF_root[treeID] -> nodeID = 0;
  result = (RF_tLeafCount[treeID] > 0) ? TRUE : FALSE;
  if (result) {
    stackAuxiliary(mode, treeID);
    RF_tTermList[treeID] = (Terminal **) new_vvector(1, RF_tLeafCount[treeID], NRUTIL_TPTR);
    for (uint b = 1; b <= RF_tLeafCount[treeID]; b++) {
      RF_tTermList[treeID][b] = NULL;
    }
    RF_tTermMembership[treeID] = (Terminal **) new_vvector(1, RF_observationSize, NRUTIL_TPTR);
    RF_leafLinkedObjHead[treeID] = RF_leafLinkedObjTail[treeID] = makeLeafLinkedObj();
    if (mode == RF_PRED) {
      RF_ftTermMembership[treeID] = (Terminal **) new_vvector(1, RF_fobservationSize, NRUTIL_TPTR);
    }
    if (mode == RF_REST) {
      for (i = 1; i <= RF_observationSize; i++) {
        RF_nodeMembership[treeID][i] = RF_root[treeID];
      }
      if ( (!(RF_opt & OPT_BOOT_TYP1) && !(RF_opt & OPT_BOOT_TYP2)) ||
           ( (RF_opt & OPT_BOOT_TYP1) &&  (RF_opt & OPT_BOOT_TYP2)) ) {
        bootMembrIndx  = uivector(1, RF_bootstrapSize);
        bootMembrSize = RF_bootstrapSize;
      }
      else {
        bootMembrIndx  = uivector(1, RF_observationSize);
        bootMembrSize = RF_observationSize;
      }
      bootstrap (mode,
                 treeID,
                 RF_root[treeID],
                 RF_identityMembershipIndex,
                 RF_observationSize,
                 bootMembrIndx,
                 bootMembrSize);
      if (RF_optHigh & OPT_WGHT) {
        membershipSize  = RF_observationSize;
        membershipIndex = RF_identityMembershipIndex;
      }
      else if ((RF_opt & OPT_FENS) ||
               ((RF_opt & OPT_PROX) && (RF_opt & OPT_PROX_IBG) && (RF_opt & OPT_PROX_OOB)) ||
               ((RF_optHigh & OPT_DIST) && (RF_optHigh & OPT_DIST_IBG) && (RF_optHigh & OPT_DIST_OOB))) {
        membershipSize  = RF_observationSize;
        membershipIndex = RF_identityMembershipIndex;
        //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //        RF_nativePrint("\n JIT target subset is (case 1):  all train data with size %10d", membershipSize); 
        //      }
      }
      else if ((RF_opt & OPT_OENS) ||
               ((RF_opt & OPT_PROX) && !(RF_opt & OPT_PROX_IBG) && (RF_opt & OPT_PROX_OOB)) ||
               ((RF_optHigh & OPT_DIST) && !(RF_optHigh & OPT_DIST_IBG) && (RF_optHigh & OPT_DIST_OOB))) {
        membershipSize  = RF_oobSize[treeID];
        membershipIndex = RF_oobMembershipIndex[treeID];
        //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //        RF_nativePrint("\n JIT target subset is (case 2):  oob train data with size %10d", membershipSize); 
        //      }
      }
      else if (((RF_opt & OPT_PROX) && (RF_opt & OPT_PROX_IBG) && !(RF_opt & OPT_PROX_OOB)) ||
               ((RF_optHigh & OPT_DIST) && (RF_optHigh & OPT_DIST_IBG) && !(RF_optHigh & OPT_DIST_OOB))) {
        membershipSize  = RF_ibgSize[treeID];
        membershipIndex = RF_ibgMembershipIndex[treeID];
        //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //        RF_nativePrint("\n JIT target subset is (case 3):  ibg train data with size %10d", membershipSize); 
        //      }
      }
      else {
        membershipSize  = RF_observationSize;
        membershipIndex = RF_identityMembershipIndex;
        //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //        RF_nativePrint("\n JIT target subset is (case 4):  all train data with size %10d", membershipSize); 
        //      }
      }
      for (i = 1; i <= membershipSize; i++) {
        ii = membershipIndex[i];
        restoreTerminalNodeJIT(treeID, RF_root[treeID], ii, RF_observation[treeID], RF_tTermMembership[treeID]);
      }
      //    if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
      //      RF_nativePrint("\nFinal REST Membership (subseted):  %10d", treeID);
      //      RF_nativePrint("\n     index       leaf\n");
      //      for (i = 1; i <= membershipSize; i++) {
      //        ii = membershipIndex[i];
      //        RF_nativePrint("%10d %10d \n", ii, RF_tTermMembership[treeID][ii] -> nodeID);
      //      }
      //    }
      if ( (!(RF_opt & OPT_BOOT_TYP1) && !(RF_opt & OPT_BOOT_TYP2)) ||
           ( (RF_opt & OPT_BOOT_TYP1) &&  (RF_opt & OPT_BOOT_TYP2)) ) {
        free_uivector(bootMembrIndx, 1, RF_bootstrapSize);
      }
      else {
        free_uivector(bootMembrIndx, 1, RF_observationSize);
      }
    }
    else {
      for (i = 1; i <= RF_fobservationSize; i++) {
        restoreTerminalNodeJIT(treeID, RF_root[treeID], i, RF_fobservation[treeID], RF_ftTermMembership[treeID]);
      }
      if (RF_optHigh & OPT_WGHT) {
        for (i = 1; i <= RF_observationSize; i++) {
          RF_nodeMembership[treeID][i] = RF_root[treeID];
        }
        if ( (!(RF_opt & OPT_BOOT_TYP1) && !(RF_opt & OPT_BOOT_TYP2)) ||
             ( (RF_opt & OPT_BOOT_TYP1) &&  (RF_opt & OPT_BOOT_TYP2)) ) {
          bootMembrIndx  = uivector(1, RF_bootstrapSize);
          bootMembrSize = RF_bootstrapSize;
        }
        else {
          bootMembrIndx  = uivector(1, RF_observationSize);
          bootMembrSize = RF_observationSize;
        }
        bootstrap (mode,
                   treeID,
                   RF_root[treeID],
                   RF_identityMembershipIndex,
                   RF_observationSize,
                   bootMembrIndx,
                   bootMembrSize);
        for (i = 1; i <= RF_ibgSize[treeID]; i++) {
          ii = RF_ibgMembershipIndex[treeID][i];
          restoreTerminalNodeJIT(treeID, RF_root[treeID], ii, RF_observation[treeID], RF_tTermMembership[treeID]);
        }
        if ( (!(RF_opt & OPT_BOOT_TYP1) && !(RF_opt & OPT_BOOT_TYP2)) ||
             ( (RF_opt & OPT_BOOT_TYP1) &&  (RF_opt & OPT_BOOT_TYP2)) ) {
          free_uivector(bootMembrIndx, 1, RF_bootstrapSize);
        }
        else {
          free_uivector(bootMembrIndx, 1, RF_observationSize);
        }
      }
      //    if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
      //      RF_nativePrint("\nFinal PRED Membership (all data):  %10d", treeID);
      //      RF_nativePrint("\n     index       leaf\n");
      //      for (i = 1; i <= RF_fobservationSize; i++) {
      //        RF_nativePrint("%10d %10d \n", i, RF_ftTermMembership[treeID][i] -> nodeID);
      //      }
      //    }
    }
    processEnsembleInSitu(mode, FALSE, treeID);
    unstackAuxiliary(mode, treeID);
  }
  //    if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //      RF_nativePrint("\nDe-allocating for treeID:  %10d", treeID);
  //    }
  freeTree(treeID, RF_root[treeID]);
  RF_root[treeID] = NULL;
  //  if (getTraceFlag(treeID) & SUMM_LOW_TRACE) {
  //    RF_nativePrint("\nEnd of JIT acquisition:  %10d", treeID);
  //  }
}
void restoreTerminalNodeJIT(uint treeID,
                            Node *root,
                            uint indv,
                            double **xArray,
                            Terminal **termMembership) {
  ulong rootIndex, nodeAbsIndex;
  uint  rmbrAbsOffset, ambrAbsOffset;
  uint  rmbrDummyIter, ambrDummyIter;
  ulong *rootMWCPoffset, *nodeAbsMWCPoffset;
  SplitInfo *info;
  char parseFlag;
  uint offset;
  char daughterFlag;
  char (*getDaughterPolarityGeneric) (uint       treeID,
                                      SplitInfo *info,
                                      uint       indv,
                                      void      *value,
                                      ...);
  void *gobsLocal;
  uint repMembrSize, leftRepMembrSize;
  uint adj, i, k;
  //  if (getTraceFlag(0) & SUMM_LOW_TRACE) {
  //    RF_nativePrint("\nrestoreTerminalNodeJIT() ENTRY ...\n");
  //  }
  rmbrDummyIter = ambrDummyIter = 0;
  nodeAbsIndex = rootIndex = RF_restoreTreeOffset[treeID];
  rmbrAbsOffset = ambrAbsOffset = 0;
    rootMWCPoffset    = ulvector(1, 1);
    nodeAbsMWCPoffset = ulvector(1, 1);
    rootMWCPoffset[1] = RF_restoreMWCPoffset[1][treeID];
  parseFlag = TRUE;
  Node *parent = root;
  while (parseFlag) {
    //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
    //        RF_nativePrint("\n JIT parent and absolute offset:  %20x %10d", parent, nodeAbsIndex); 
    //      }
    if (parent -> nodeID == 0) {
      //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //        RF_nativePrint("\n JIT parent is not initialized:  (parent = %20x) -> nodeID %10d", parent, parent -> nodeID); 
      //      }
      parent -> nodeID = RF_nodeID_[nodeAbsIndex];
      parent -> repMembrSize = RF_nodeSZ_[nodeAbsIndex];
      //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //        RF_nativePrint("\n JIT parent is being initialized to:  (parent = %20x) -> nodeID %10d", parent, parent -> nodeID); 
      //      }
      if (parent -> parent != NULL) {
        parent -> depth = (parent -> parent) -> depth + 1;
      }
      parent -> bnodeID = nodeAbsIndex - rootIndex + 1;
      if (RF_parmID_[1][nodeAbsIndex] != 0) {
        parent -> blnodeID = parent -> bnodeID + 1;
        parent -> brnodeID = RF_brnodeID_[nodeAbsIndex];
        info = parent -> splitInfo = makeSplitInfo(0);
        adj = 1;
        info -> mwcpSizeAbs = uivector(1, adj);
        info -> randomVar   = ivector(1, adj);
        info -> randomPts   = new_vvector(1, adj, NRUTIL_VPTR);
        for (k = 1; k <= adj; k++) {
          //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
          //        RF_nativePrint("\n  restoreTerminalNodeJIT() (h-idx, (parent -> splitInfo) -> randomVar[k])   = (%10d, %10d)", k, RF_parmID_[k][nodeAbsIndex]); 
          //      }
          info -> randomVar[k] = RF_parmID_[k][nodeAbsIndex];
          info -> mwcpSizeAbs[k] = RF_mwcpSZ_[k][nodeAbsIndex];
          //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
          //        RF_nativePrint("\n  restoreTerminalNodeJIT() (h-idx, (parent -> splitInfo) -> mwcpSizeAbs[k]) = (%10d, %10d)", k, RF_mwcpSZ_[k][nodeAbsIndex]);
          //      }
          if (RF_mwcpSZ_[k][nodeAbsIndex] > 0) {
            nodeAbsMWCPoffset[k] = rootMWCPoffset[k] + RF_fsrecID_[k][nodeAbsIndex];
            //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
            //        RF_nativePrint("\n (h-idx, absolute mwcp offset) -> (%10d %10d)", k, nodeAbsMWCPoffset[k]);
            //      }
            //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
            //        RF_nativePrint("\n  mwcpPT (reversed):  [%20x] ", RF_mwcpPT_[k]);
            //      }
            info -> randomPts[k] = uivector(1, RF_mwcpSZ_[k][nodeAbsIndex]);
            for (i = 1; i <= RF_mwcpSZ_[k][nodeAbsIndex]; i++) {
              ((uint *) info -> randomPts[k])[i] = RF_mwcpPT_[k][nodeAbsMWCPoffset[k]];
              //        if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
              //          RF_nativePrint("%8x ", RF_mwcpPT_[k][nodeAbsMWCPoffset[k]]);
              //        }
              nodeAbsMWCPoffset[k] ++;
            }
          }
          else {
            info -> randomPts[k] = dvector(1, 1);
            ((double *) info -> randomPts[k])[1] =  RF_contPT_[k][nodeAbsIndex];
          }
        }
        //  if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
        //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
        //      RF_nativePrint("\n restoreTerminalNodeJIT() parent -> splitInfo:  %20x", info);
        //      getSplitObjectInfo(parent -> splitInfo);
        //    }
        //  }
      }
      else {
        info = parent -> splitInfo = NULL;
      }
    }  
    else {
      info = parent -> splitInfo;
      //      if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //        RF_nativePrint("\n JIT parent is already initialized:  (parent = %20x) -> nodeID %10d", parent, parent -> nodeID); 
      //      }
    }
    daughterFlag = NEITHER;
    if (info != NULL) {
      gobsLocal = (double **) xArray;
        gobsLocal = (double *) ((double **) gobsLocal)[info -> randomVar[1]];
        if (info -> mwcpSizeAbs[1] > 0) {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
        }
        else {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
        }
        if (RF_fmRecordSize > 0) {
          if (RF_fmRecordMap[indv] > 0) {
            offset = RF_ySize + (info -> randomVar[1]);
            if (RF_fmpSign[offset][RF_fmRecordMap[indv]] == 1) {
              repMembrSize = parent -> repMembrSize;
              if (parent -> left != NULL) {
                leftRepMembrSize = (parent -> left) -> repMembrSize;
              }
              else {
                leftRepMembrSize = RF_nodeSZ_[nodeAbsIndex + 1];
              }
              //        if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
              //            RF_nativePrint("\nRandom assignment in JIT for missing (x-var, indv) = (%10d, %10d)", info -> randomVar[1], indv);
              //            RF_nativePrint("\n       with probability:  %10.4f", (double) leftRepMembrSize / repMembrSize);
              //        }
              if (ran1B(treeID) <= (double) leftRepMembrSize / repMembrSize) {
                daughterFlag = LEFT;
              }
              else {
                daughterFlag = RIGHT;
              }
            }
          }
        }
      //        if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //            RF_nativePrint("\nNode Membership:  %10d ", indv);
      //        }
      if (daughterFlag == NEITHER) {
        daughterFlag = getDaughterPolarityGeneric(treeID,
                                                  info,
                                                  indv,
                                                  gobsLocal,
                                                  parent,
                                                  RF_PRED);
      }
      if (daughterFlag == LEFT) {
        //            if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //              RF_nativePrint(" --> LEFT ");
        //            }
        nodeAbsIndex = nodeAbsIndex + 1;
        if (parent -> left == NULL) {
          parent -> left  = makeNode(0);
        }
        setParent(parent -> left, parent);
        parent = parent -> left;
      }
      else {
        //            if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //              RF_nativePrint(" --> RGHT ");
        //            }
        nodeAbsIndex = rootIndex + (parent -> brnodeID) - 1;
        i = parent -> nodeID;
        while(i < RF_nodeID_[nodeAbsIndex]) {
          rmbrAbsOffset = rmbrAbsOffset + RF_TN_RCNT_ptr[treeID][i];
          ambrAbsOffset = ambrAbsOffset + RF_TN_ACNT_ptr[treeID][i];
          i++;
        }
        if (parent -> right == NULL) {
          parent -> right = makeNode(0);
        }
        setParent(parent -> right, parent);
        parent = parent -> right;
      }
      //            if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //              RF_nativePrint("\nJIT running RMBR and AMBR offsets:  %10d and %10d", rmbrAbsOffset, ambrAbsOffset);
      //            }
    }  
    else {
      if (RF_tTermList[treeID][parent -> nodeID] != NULL) {
        //            if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //              RF_nativePrint("\nJIT terminal node exists:  %20x -> %10d", RF_tTermList[treeID][parent -> nodeID], RF_tTermList[treeID][parent -> nodeID] -> nodeID);
        //            }
      }
      else {
        RF_leafLinkedObjTail[treeID] = makeAndSpliceLeafLinkedObj(RF_leafLinkedObjTail[treeID],
                                                                  parent,
                                                                  RF_TN_RCNT_ptr[treeID][parent -> nodeID],
                                                                  RF_TN_ACNT_ptr[treeID][parent -> nodeID]);
        RF_tTermList[treeID][parent -> nodeID] = RF_leafLinkedObjTail[treeID] -> termPtr;
        //            if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
        //              RF_nativePrint("\nJIT terminal node does not exist (creating it):  %20x -> %10d", RF_tTermList[treeID][parent -> nodeID], RF_tTermList[treeID][parent -> nodeID] -> nodeID);
        //            }
        updateTerminalNodeOutcomes(RF_PRED,
                                   treeID,
                                   parent -> mate,
                                   & RF_RMBR_ID_ptr[treeID][rmbrAbsOffset],
                                   RF_TN_RCNT_ptr[treeID][parent -> nodeID],
                                   & RF_AMBR_ID_ptr[treeID][ambrAbsOffset],
                                   RF_TN_ACNT_ptr[treeID][parent -> nodeID],
                                   & rmbrDummyIter,
                                   & ambrDummyIter);
      }
      termMembership[indv] = parent -> mate;
      //            if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //              RF_nativePrint("\n JIT assignment of terminal node memebership to individual:  %10d -> %10d", indv, termMembership[indv] -> nodeID);
      //            }
      if (RF_optHigh & OPT_MEMB_USER) {
        RF_MEMB_ID_ptr[treeID][indv] = parent -> nodeID;
      }
      //            if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
      //              RF_nativePrint("\nJIT final RMBR and AMBR offsets:  %10d and %10d", rmbrAbsOffset, ambrAbsOffset);
      //              RF_nativePrint("\nJIT final RCNT and ACNT sizes:    %10d and %10d", RF_TN_RCNT_ptr[treeID][parent -> nodeID], RF_TN_ACNT_ptr[treeID][parent -> nodeID]);
      //            }
      parseFlag = FALSE;
    }
  }  
  free_ulvector(rootMWCPoffset, 1, 1);
  free_ulvector(nodeAbsMWCPoffset, 1, 1);
  //  if (getTraceFlag(0) & SUMM_LOW_TRACE) {
  //    RF_nativePrint("\nrestoreTerminalNodeJIT() EXIT ...\n");
  //  }
}
