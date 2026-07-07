
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "marginal.h"
#include "polarity.h"
#include "nrutil.h"
// #include "error.h"
void getMarginalMembership(char mode, uint treeID) {
  double **observationPtr;
  uint    *gallMembrIndx;
  uint     obsSize;
  uint i;
  switch (mode) {
  case RF_PRED:
    obsSize = RF_fobservationSize;
    observationPtr = RF_fobservation[treeID];
    gallMembrIndx  = RF_fidentityMembershipIndex;
    break;
  default:
    obsSize = RF_observationSize;
    observationPtr = RF_observation[treeID];
    gallMembrIndx  = RF_identityMembershipIndex;
    break;
  }
  RF_utTermMembership[treeID] =       (uint **) new_vvector(1, obsSize, NRUTIL_UPTR);
  RF_utTermMembershipCount[treeID] =   (uint *) uivector(1, obsSize);
  RF_utTermMembershipAlloc[treeID] =   (uint *) uivector(1, obsSize);
  for (i = 1; i <= obsSize; i++) {
    RF_utTermMembership[treeID][i] = uivector(1, MARGINAL_SIZE);
    RF_utTermMembershipCount[treeID][i] = 0;
    RF_utTermMembershipAlloc[treeID][i] = 1;            
  }
  //      if (getTraceFlag(treeID) & SUMM_LOW_TRACE) {
  //        RF_nativePrint("\nBeginning of Marginal Membership Restoration:  %10d", treeID);
  //      }
  marginalMembership(treeID,
                     RF_root[treeID],
                     gallMembrIndx,
                     obsSize,
                     obsSize,
                     observationPtr);
  //      if (getTraceFlag(treeID) & SUMM_LOW_TRACE) {
  //        RF_nativePrint("\nEnd of Marginal Membership Restoration:  %10d", treeID);
  //      }
  //    if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //      if (RF_xMarginalSize > 0) {
  //        RF_nativePrint("\nFinal MARGINAL Membership (all data):  %10d", treeID);
  //        RF_nativePrint("\n       index         leaf ->\n");
  //        for (i=1; i <= obsSize; i++) {
  //          RF_nativePrint("%12d ", i);
  //          for (uint j=1; j <= RF_utTermMembershipCount[treeID][i]; j++) {
  //            RF_nativePrint("%12d ", RF_utTermMembership[treeID][i][j]);
  //          }
  //          RF_nativePrint("\n");
  //        }
  //      }
  //    }
}
void releaseMarginalMembership(char mode, uint treeID) {
  uint     obsSize;
  switch (mode) {
  case RF_PRED:
    obsSize = RF_fobservationSize;
    break;
  default:
    obsSize = RF_observationSize;
    break;
  }
  free_uivector(RF_utTermMembershipCount[treeID], 1, obsSize);
  for (uint i = 1; i <= obsSize; i++) {
    free_uivector(RF_utTermMembership[treeID][i], 1, MARGINAL_SIZE * RF_utTermMembershipAlloc[treeID][i]);
  }
  free_new_vvector(RF_utTermMembership[treeID], 1, obsSize, NRUTIL_UPTR);
  free_uivector(RF_utTermMembershipAlloc[treeID], 1, obsSize);
}
void marginalMembership(uint     treeID,
                        Node    *parent,
                        uint    *genAllMembrIndx,
                        uint     genAllMembrSize,
                        uint     obsSize,
                        double **xArray) {
  char terminalFlag;
  uint *leftAllMembrIndx;
  uint *rghtAllMembrIndx;
  uint leftAllMembrSize;
  uint rghtAllMembrSize;
  uint jLeft;
  uint jRght;
  char daughterFlag;
  SplitInfo *info;
  void *obsLocal;
  char (*getDaughterPolarityGeneric) (uint       treeID,
                                      SplitInfo *info,
                                      uint       index,
                                      void      *value,
                                      ...);
  uint i, j;
  //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
  //    RF_nativePrint("\nmarginalMembership() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
  //    RF_nativePrint("\nmarginalMembership (%10d) leaf:  %10d \n", treeID, parent -> nodeID);
  //    RF_nativePrint("\n  called with   all size:  %10d", genAllMembrSize);
  //  }
  getDaughterPolarityGeneric = NULL;  
  terminalFlag = TRUE;
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    info = parent -> splitInfo;
    terminalFlag = FALSE;
    //    if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
    //      RF_nativePrint("\nForking On:  ");
    //      getNodeInfo(parent);
    //    }
    leftAllMembrIndx = rghtAllMembrIndx = NULL;
    leftAllMembrSize = rghtAllMembrSize = 0;
    uint *indicator = uivector(1, obsSize);
    leftAllMembrSize = rghtAllMembrSize = 0;
    obsLocal = xArray[info -> randomVar[1]];
    if (info -> mwcpSizeAbs[1] > 0) {
      getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
    }
    else {
      getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
    }
    daughterFlag = RIGHT;
    if (RF_xMarginalFlag[info -> randomVar[1]] ==  TRUE) { 
      daughterFlag = NEITHER;
      for (i = 1; i <= genAllMembrSize; i++) {
        indicator[genAllMembrIndx[i]] = daughterFlag;
        leftAllMembrSize ++;
        rghtAllMembrSize ++;
        //            if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
        //              RF_nativePrint("\nmNode Membership:  %10d %10d %20.8f", i, genAllMembrIndx[i], (uint) RF_observation[treeID][info -> randomVar[1]][genAllMembrIndx[i]]);
        //              RF_nativePrint(" --> BOTH ");
        //            }
      }        
    }
    if (daughterFlag != NEITHER) {
      for (i = 1; i <= genAllMembrSize; i++) {
        //        if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
        //            RF_nativePrint("\nmNode Membership:  %10d %10d ", i, genAllMembrIndx[i]);
        //        }
        daughterFlag = getDaughterPolarityGeneric(treeID,
                                                  info,
                                                  genAllMembrIndx[i],
                                                  obsLocal);
        //      if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
        //        if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
        //          if(daughterFlag == LEFT) {
        //            RF_nativePrint("\nNon-Greedy Daughter LEFT :  %10d %10d", i, genAllMembrIndx[i]);
        //          }
        //          else {
        //            RF_nativePrint("\nNon-Greedy Daughter RGHT :  %10d %10d", i, genAllMembrIndx[i]);
        //          }
        //        }
        //      }
        indicator[genAllMembrIndx[i]] = daughterFlag;
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
    }  
    leftAllMembrIndx  = uivector(1, leftAllMembrSize + 1);
    rghtAllMembrIndx  = uivector(1, rghtAllMembrSize + 1);
    jLeft = jRght = 0;
    if (daughterFlag == NEITHER) {
      for (i = 1; i <= genAllMembrSize; i++) {
        leftAllMembrIndx[++jLeft] = genAllMembrIndx[i];
        rghtAllMembrIndx[++jRght] = genAllMembrIndx[i];
      }
    }
    else {
      for (i = 1; i <= genAllMembrSize; i++) {
        if (indicator[genAllMembrIndx[i]] == LEFT) {
          leftAllMembrIndx[++jLeft] = genAllMembrIndx[i];
        }
        else {
          rghtAllMembrIndx[++jRght] = genAllMembrIndx[i];
        }
      }
    }
    free_uivector(indicator, 1, obsSize);
    //        if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
    //          RF_nativePrint("\nmarginalMembership(%10d) LEFT:  \n", treeID);
    //        }
    marginalMembership(treeID,
                              parent -> left,
                              leftAllMembrIndx,
                              leftAllMembrSize,
                              obsSize,
                              xArray);
    //        if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
    //          RF_nativePrint("\nmarginalMembership(%10d) RGHT:  \n", treeID);
    //        }
    marginalMembership(treeID,
                              parent -> right,
                              rghtAllMembrIndx,
                              rghtAllMembrSize,
                              obsSize,
                              xArray);
    free_uivector(leftAllMembrIndx, 1, leftAllMembrSize + 1);
    free_uivector(rghtAllMembrIndx, 1, rghtAllMembrSize + 1);
  }  
  else {
    //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
    //    RF_nativePrint("\n  Terminal node encountered for leaf:  %10d %20x ",  parent -> nodeID, parent);
    //  }
  }
  if (terminalFlag) {
    for (i = 1; i <= genAllMembrSize; i++) {
      RF_utTermMembership[treeID][genAllMembrIndx[i]][++ RF_utTermMembershipCount[treeID][genAllMembrIndx[i]]] = (parent -> nodeID);
      if ((RF_utTermMembershipCount[treeID][genAllMembrIndx[i]]) == (RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] * MARGINAL_SIZE)) {
        RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] ++;
        uint *utTermMembershipNew = uivector(1, RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] * MARGINAL_SIZE);
        for (j = 1; j <= RF_utTermMembershipCount[treeID][genAllMembrIndx[i]]; j++) {
          utTermMembershipNew[j] = RF_utTermMembership[treeID][genAllMembrIndx[i]][j];
        }
        free_uivector(RF_utTermMembership[treeID][genAllMembrIndx[i]], 1, (RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] - 1) * MARGINAL_SIZE);
        RF_utTermMembership[treeID][genAllMembrIndx[i]] = utTermMembershipNew;
      }
    }
    //            if (getTraceFlag(treeID) & FORK_DEF_TRACE) {
    //              RF_nativePrint("\nMarginal Split Multiple Terminal Node Membership: ");
    //              RF_nativePrint("\n      index       indv membership -> ");
    //              for (i = 1; i <= genAllMembrSize; i++) {
    //                RF_nativePrint("\n %10d %10d", i, genAllMembrIndx[i]);
    //                for (j = 1; j <= RF_utTermMembershipCount[treeID][genAllMembrIndx[i]]; j++) {
    //                  RF_nativePrint(" %10d", RF_utTermMembership[treeID][genAllMembrIndx[i]][j]);
    //                }
    //              }
    //            }
  }  
  //  if (getTraceFlag(treeID) & SPLT_DEF_TRACE) {
  //    RF_nativePrint("\nmarginalMembership() EXIT ...\n");
  //  }
}
