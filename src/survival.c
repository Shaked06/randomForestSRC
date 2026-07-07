
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "survival.h"
#include "rfsrcUtil.h"
#include "termOps.h"
#include "nrutil.h"
#include "error.h"
void getAtRiskAndEventCount(uint       treeID,
                             Terminal  *parent,
                             uint      *repMembrIndx,
                             uint       repMembrSize,
                             uint      *allMembrIndx,
                             uint       allMembrSize,
                             uint      *rmbrIterator) {
  uint *membershipIndex;
  uint  membershipSize;
  uint i, j, k;
  uint ii;
  char eventFlag;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetAtRiskAndEventCount() ENTRY ...\n");
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
    stackAtRiskAndEventCount(parent, RF_eventTypeSize, RF_masterTimeSize);
    for (j = 1; j <= RF_masterTimeSize; j++) {
      (parent -> atRiskCount)[j] = 0;
      for (k = 1; k <= RF_eventTypeSize; k++) {
        (parent -> eventCount)[k][j] = 0;
      }
    }
    if (RF_optHigh & OPT_MEMB_OUTG) {
      for (i = 1; i <= membershipSize; i++) {
        ii = membershipIndex[i];
        RF_RMBR_ID_ptr[treeID][++(*rmbrIterator)] = ii;
        for (j = RF_masterEntryTimeIndex[treeID][ii]; j <= RF_masterTimeIndex[treeID][ii]; j++) {
          (parent -> atRiskCount)[j] ++;
        }
        if (RF_status[treeID][ii] > 0) {
          if (RF_eventTypeSize > 1) {
            k = RF_eventTypeIndex[(uint) RF_status[treeID][ii]];
          }
          else {
            k = 1;
          }
          (parent -> eventCount)[k][RF_masterTimeIndex[treeID][ii]] ++;
        }
      }
    }
    else if (RF_optHigh & OPT_MEMB_INCG) {
      for (i = 1; i <= membershipSize; i++) {
        ii = membershipIndex[++(*rmbrIterator)];
        for (j = RF_masterEntryTimeIndex[treeID][ii]; j <= RF_masterTimeIndex[treeID][ii]; j++) {
          (parent -> atRiskCount)[j] ++;
        }
        if (RF_status[treeID][ii] > 0) {
          if (RF_eventTypeSize > 1) {
            k = RF_eventTypeIndex[(uint) RF_status[treeID][ii]];
          }
          else {
            k = 1;
          }
          (parent -> eventCount)[k][RF_masterTimeIndex[treeID][ii]] ++;
        }
      }
    }
    else {
      for (i = 1; i <= membershipSize; i++) {
        ii = membershipIndex[i];
        for (j = RF_masterEntryTimeIndex[treeID][ii]; j <= RF_masterTimeIndex[treeID][ii]; j++) {
          (parent -> atRiskCount)[j] ++;
        }
        if (RF_status[treeID][ii] > 0) {
          if (RF_eventTypeSize > 1) {
            k = RF_eventTypeIndex[(uint) RF_status[treeID][ii]];
          }
          else {
            k = 1;
          }
          (parent -> eventCount)[k][RF_masterTimeIndex[treeID][ii]] ++;
        }
      }
    }
    uint *tempEventTimeIndex = uivector(1, RF_masterTimeSize);
    parent -> eTimeSize = 0;
    i = 0;    
    for (j = 1; j <= RF_masterTimeSize; j++) {
      eventFlag = FALSE;
      for (k = 1; k <= RF_eventTypeSize; k++) {
        if ((parent -> eventCount)[k][j] > 0) {
          eventFlag = TRUE;
          k = RF_eventTypeSize;
        }
      }
      if (eventFlag == TRUE) {
        tempEventTimeIndex[++i] = j;        
        (parent -> eTimeSize)++;
      }
    }
    stackEventTimeIndex(parent, parent -> eTimeSize);
    for (j = 1; j <= parent -> eTimeSize; j++) {
      (parent -> eventTimeIndex)[j] = tempEventTimeIndex[j];
    }
    free_uivector(tempEventTimeIndex, 1, RF_masterTimeSize);
    //        if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
    //          if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
    //            RF_nativePrint("\nNode Specific At Risk and Event Counts for (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
    //            RF_nativePrint("Mstr Time ");
    //            for (i=1; i <= RF_masterTimeSize; i++) {
    //              RF_nativePrint("%10d", i);
    //            }
    //            RF_nativePrint("\n");
    //            RF_nativePrint("At Risk   ");
    //            for (i=1; i <= RF_masterTimeSize; i++) {
    //              RF_nativePrint("%10d", (parent -> atRiskCount)[i]);
    //            }
    //            RF_nativePrint("\n");
    //            for (j=1; j <= RF_eventTypeSize; j++) {
    //              RF_nativePrint("Ev %7d", j);
    //              for (i=1; i <= RF_masterTimeSize; i++) {
    //                RF_nativePrint("%10d", (parent -> eventCount)[j][i]);
    //              }
    //              RF_nativePrint("\n");
    //            }
    //            RF_nativePrint("\nEv Tm Idx ");
    //            for (i=1; i <= parent -> eTimeSize; i++) {
    //              RF_nativePrint("%10d", i);
    //            }
    //            RF_nativePrint("\n          ");
    //            for (i=1; i <= parent -> eTimeSize; i++) {
    //              RF_nativePrint("%10d", (parent -> eventTimeIndex)[i]);
    //            }
    //            RF_nativePrint("\n");
    //          }
    //        }
  }
  else {
  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetAtRiskAndEventCount() EXIT ...\n");
  //  }
}
void getLocalRatio(uint treeID, Terminal *parent) {
  uint j, q;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalRatio() ENTRY ...\n");
  //  }
  if (parent -> membrCount > 0) {
    if(parent -> eTimeSize > 0) {
      stackLocalRatio(parent, RF_eventTypeSize, parent -> eTimeSize);
      for (j = 1; j <= RF_eventTypeSize; j++) {
        for (q = 1; q <= parent -> eTimeSize; q++) {
          if ((parent -> eventCount)[j][(parent -> eventTimeIndex)[q]] > 0) {
            if ((parent -> atRiskCount)[(parent -> eventTimeIndex)[q]] >= 1) {
              (parent -> localRatio)[j][q] = (double) ((parent -> eventCount)[j][(parent -> eventTimeIndex)[q]]) / (parent -> atRiskCount)[(parent -> eventTimeIndex)[q]];
            }
            else {
              RF_nativeError("\nRF-SRC:  *** ERROR *** ");
              RF_nativeError("\nRF-SRC:  Zero At Risk Count encountered in local ratio calculation for (tree, leaf) = (%10d, %10d)", treeID, parent -> nodeID);
              RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
              RF_nativeExit();
            }
          }
          else {
            (parent -> localRatio)[j][q] = 0.0;
          }
        }
      }
    }
    //      if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
    //        RF_nativePrint("\nNode specific local ratios:  [RF_eventTypeSize] x [nodeEventTimeSize] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
    //        RF_nativePrint("              mTimIdx       time ");
    //        for (j=1; j <= RF_eventTypeSize; j++) {
    //          RF_nativePrint("%10d ", j);
    //        }
    //        RF_nativePrint("\n");
    //        for (q=1; q <= parent -> eTimeSize; q++) {
    //          RF_nativePrint("%10d %10d %10.4f ", q, (parent -> eventTimeIndex)[q], RF_masterTime[(parent -> eventTimeIndex)[q]]);
    //          for (j=1; j <= RF_eventTypeSize; j++) {
    //            RF_nativePrint("%10.4f ", (parent -> localRatio)[j][q]);
    //          }
    //          RF_nativePrint("\n");
    //        }
    //      }
  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalRatio() EXIT ...\n");
  //  }
}
void getLocalSurvival(uint treeID, Terminal *parent) {
  uint j, q;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalSurvival() ENTRY ...\n");
  //  }
  if(parent -> eTimeSize > 0) {
    stackLocalSurvival(parent, parent -> eTimeSize);
    for (q = 1; q <= parent -> eTimeSize; q++) {
      (parent -> localSurvival)[q] = 0.0;
      for (j = 1; j <= RF_eventTypeSize; j++) {
        (parent -> localSurvival)[q] += (parent -> localRatio)[j][q];
      }
      (parent -> localSurvival)[q] = 1.0 - (parent -> localSurvival)[q];
    }  
    for (q = 2; q <= parent -> eTimeSize; q++) {
      (parent -> localSurvival)[q] *= (parent -> localSurvival)[q-1];
    }
    //      if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
    //        RF_nativePrint("\nNode specific local survival function of length [parent -> eTimeSize] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
    //        RF_nativePrint("              mTimIdx       time   survival \n");
    //        for (q = 1; q <= parent -> eTimeSize; q++) {
    //          RF_nativePrint("%10d %10d %10.4f %10.4f \n", q, (parent -> eventTimeIndex)[q], RF_masterTime[(parent -> eventTimeIndex)[q]], (parent -> localSurvival)[q]);
    //        }
    //      }
  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalSurvival() EXIT ...\n");
  //  }
}
void getLocalNelsonAalen(uint treeID, Terminal *parent) {
  uint q;
  //  uint j;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalNelsonAalen() ENTRY ...\n");
  //  }
  if (parent -> eTimeSize > 0) {
    stackLocalNelsonAalen(parent, parent -> eTimeSize);
    for (q = 1; q <= parent -> eTimeSize; q++) {
      (parent -> localNelsonAalen)[q] = (parent -> localRatio)[1][q];
    }
    for (q = 2; q <= parent -> eTimeSize; q++) {
      (parent -> localNelsonAalen)[q] += (parent -> localNelsonAalen)[q-1];
    }
  }
  //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
  //    if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
  //        RF_nativePrint("\nLocal Nelson-Aalen estimator for (tree, leaf):  (%10d, %10d) \n", treeID, parent -> nodeID);
  //        for (j=1; j <= parent -> eTimeSize; j++) {
  //          RF_nativePrint("%10d", j);
  //        }
  //        RF_nativePrint("\n");
  //        for (j=1; j <= parent -> eTimeSize; j++) {
  //          RF_nativePrint("%10.4f", parent -> localNelsonAalen[j]);
  //        }
  //        RF_nativePrint("\n");
  //    }
  //  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalNelsonAalen() EXIT ...\n");
  //  }
}
void getLocalCSH(uint treeID, Terminal *parent) {
  uint j, q;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalCSH() ENTRY ...\n");
  //  }
    if (parent -> eTimeSize > 0) {
      stackLocalCSH(parent, RF_eventTypeSize, parent -> eTimeSize);
      for (j = 1; j <= RF_eventTypeSize; j++) {
        for (q = 1; q <= parent -> eTimeSize; q++) {
          (parent -> localCSH)[j][q] = (parent -> localRatio)[j][q];
        }
        for (q = 2; q <= parent -> eTimeSize; q++) {
          (parent -> localCSH)[j][q] += (parent -> localCSH)[j][q-1];
        }
      }
      //      if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
      //        RF_nativePrint("\nNode specific local CSH:  [RF_eventTypeSize] x [nodeEventTimeSize] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
      //        RF_nativePrint("              mTimIdx       time ");
      //        for (j=1; j <= RF_eventTypeSize; j++) {
      //          RF_nativePrint("%10d ", j);
      //        }
      //        RF_nativePrint("\n");
      //        for (q=1; q <= parent -> eTimeSize; q++) {
      //          RF_nativePrint("%10d %10d %10.4f ", q, (parent -> eventTimeIndex)[q], RF_masterTime[(parent -> eventTimeIndex)[q]]);
      //          for (j=1; j <= RF_eventTypeSize; j++) {
      //            RF_nativePrint("%10.4f ", (parent -> localCSH)[j][q]);
      //          }
      //          RF_nativePrint("\n");
      //        }
      //      }
    }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalCSH() EXIT ...\n");
  //  }
}
void getLocalCIF(uint treeID, Terminal *parent) {
  uint j, q;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalCIF() ENTRY ...\n");
  //  }
  if(parent -> eTimeSize > 0) {
    stackLocalCIF(parent, RF_eventTypeSize, parent -> eTimeSize);
    for (j = 1; j <= RF_eventTypeSize; j++) {
      (parent -> localCIF)[j][1] = (parent -> localRatio)[j][1];
      for (q = 2; q <= parent -> eTimeSize; q++) {
        (parent -> localCIF)[j][q] = (parent -> localSurvival)[q-1] * (parent -> localRatio)[j][q];
      }
      for (q = 2; q <= parent -> eTimeSize; q++) {
        (parent -> localCIF)[j][q] += (parent -> localCIF)[j][q-1];
      }
    }
    //      if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
    //        RF_nativePrint("\nNode specific local CIF:  [RF_eventTypeSize] x [nodeEventTimeSize] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
    //        RF_nativePrint("              mTimIdx       time ");
    //        for (j=1; j <= RF_eventTypeSize; j++) {
    //          RF_nativePrint("%10d ", j);
    //        }
    //        RF_nativePrint("\n");
    //        for (q=1; q <= parent -> eTimeSize; q++) {
    //          RF_nativePrint("%10d %10d %10.4f ", q, (parent -> eventTimeIndex)[q], RF_masterTime[(parent -> eventTimeIndex)[q]]);
    //          for (j=1; j <= RF_eventTypeSize; j++) {
    //            RF_nativePrint("%10.4f ", (parent -> localCIF)[j][q]);
    //          }
    //          RF_nativePrint("\n");
    //        }
    //      }
  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetLocalCIF() EXIT ...\n");
  //  }
}
void mapLocalToTimeInterest(uint      treeID,
                            Terminal *parent,
                            void     *genericLocal,
                            void     *genericGlobal) {
  uint itIndex, etIndex, lookAheadIndex;
  char mapFlag, transitFlag;
  uint j;
  //  if (getTraceFlag(treeID) & SUMM_HGH_TRACE) {
  //    RF_nativePrint("\nmapTimeInterest() ENTRY ...\n");
  //  }
  if (!(RF_opt & OPT_COMP_RISK)) {
    if ((parent -> eTimeSize) > 0) {
      itIndex = 1;
      etIndex = 1;
      mapFlag = TRUE;
      //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
      //    RF_nativePrint("\n Map:  (treeID, nodeID) = (%10d, %10d)", treeID, parent -> nodeID);
      //    RF_nativePrint("\n Size: (itSize, etSize) = (%10d, %10d)", RF_sortedTimeInterestSize, parent -> eTimeSize);
      //  }
      while(mapFlag) {
        if (RF_timeInterest[itIndex] < RF_masterTime[(parent -> eventTimeIndex)[etIndex]] ) {
          if (itIndex > 1) {
            //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
            //  RF_nativePrint("\n Flat-Line index:   (%10d, %10d) (%10.4f, %10.4f)", itIndex, etIndex, RF_timeInterest[itIndex], RF_masterTime[(parent -> eventTimeIndex)[etIndex]]);
            //  }
            ((double *) genericGlobal)[itIndex] = ((double *) genericGlobal)[itIndex-1];
          }
          itIndex++;
        }
        else {
          lookAheadIndex = etIndex;
          transitFlag = TRUE;
          while (transitFlag) {
            if (RF_timeInterest[itIndex] >= RF_masterTime[(parent -> eventTimeIndex)[lookAheadIndex]] ) {
              //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
              //  RF_nativePrint("\n Transition index:  (%10d, %10d) (%10.4f, %10.4f)", itIndex, lookAheadIndex, RF_timeInterest[itIndex], RF_masterTime[(parent -> eventTimeIndex)[lookAheadIndex]]);
              //  }
              ((double *) genericGlobal)[itIndex] = ((double *) genericLocal)[lookAheadIndex];
              lookAheadIndex++;
              if (lookAheadIndex > (parent -> eTimeSize)) {
                transitFlag = FALSE;
              }
            }
            else {
              transitFlag = FALSE;
            }
          }
          itIndex++;
          etIndex = lookAheadIndex;
        }
        if(etIndex > (parent -> eTimeSize)) {
          while(itIndex <= RF_sortedTimeInterestSize) {
            //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
            //  RF_nativePrint("\n Tail index:        (%10d,           ) (%10.4f,           )", itIndex, RF_timeInterest[itIndex]);
            //  }
            ((double *) genericGlobal)[itIndex] = ((double *) genericGlobal)[itIndex-1];
            itIndex++;
          }
        }
        if(itIndex > RF_sortedTimeInterestSize) {
          mapFlag = FALSE;
        }
      }
    }
  }  
  else {
    if ((parent -> eTimeSize) > 0) {
      itIndex = 1;
      etIndex = 1;
      mapFlag = TRUE;
      //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
      //    RF_nativePrint("\n Map:  (treeID, nodeID) = (%10d, %10d)", treeID, parent -> nodeID);
      //    RF_nativePrint("\n Size: (itSize, etSize) = (%10d, %10d)", RF_sortedTimeInterestSize, parent -> eTimeSize);
      //  }
      while(mapFlag) {
        if (RF_timeInterest[itIndex] < RF_masterTime[(parent -> eventTimeIndex)[etIndex]] ) {
          if (itIndex > 1) {
            //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
            //  RF_nativePrint("\n Flat-Line index:   (%10d, %10d) (%10.4f, %10.4f)", itIndex, etIndex, RF_timeInterest[itIndex], RF_masterTime[(parent -> eventTimeIndex)[etIndex]]);
            //  }
            for (j = 1; j <= RF_eventTypeSize; j++) {
              ((double **) genericGlobal)[j][itIndex] = ((double **) genericGlobal)[j][itIndex-1];
            }
          }
          itIndex++;
        }
        else {
          lookAheadIndex = etIndex;
          transitFlag = TRUE;
          while (transitFlag) {
            if (RF_timeInterest[itIndex] >= RF_masterTime[(parent -> eventTimeIndex)[lookAheadIndex]] ) {
              //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
              //  RF_nativePrint("\n Transition index:  (%10d, %10d) (%10.4f, %10.4f)", itIndex, lookAheadIndex, RF_timeInterest[itIndex], RF_masterTime[(parent -> eventTimeIndex)[lookAheadIndex]]);
              //  }
              for (j = 1; j <= RF_eventTypeSize; j++) {
                ((double **) genericGlobal)[j][itIndex] = ((double **) genericLocal)[j][lookAheadIndex];
              }
              lookAheadIndex++;
              if (lookAheadIndex > (parent -> eTimeSize)) {
                transitFlag = FALSE;
              }
            }
            else {
              transitFlag = FALSE;
            }
          }
          itIndex++;
          etIndex = lookAheadIndex;
        }
        if(etIndex > (parent -> eTimeSize)) {
          while(itIndex <= RF_sortedTimeInterestSize) {
            //  if (getTraceFlag(treeID) & ENSB_HGH_TRACE) {
            //  RF_nativePrint("\n Tail index:        (%10d,           ) (%10.4f,           )", itIndex, RF_timeInterest[itIndex]);
            //  }
              for (j = 1; j <= RF_eventTypeSize; j++) {
                ((double **) genericGlobal)[j][itIndex] = ((double **) genericGlobal)[j][itIndex-1];
              }
              itIndex++;
          }
        }
        if(itIndex > RF_sortedTimeInterestSize) {
          mapFlag = FALSE;
        }
      }
    }    
  }  
  //  if (getTraceFlag(treeID) & SUMM_HGH_TRACE) {
  //    RF_nativePrint("\nmapTimeInterest() EXIT ...\n");
  //  }
}
void getSurvival(uint treeID, Terminal *parent) {
  uint k;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetSurvival() ENTRY ...\n");
  //  }
  if (!(RF_optHigh & OPT_TERM_INCG)) {
    stackSurvival(parent, RF_sortedTimeInterestSize);
    for (k = 1; k <= RF_sortedTimeInterestSize; k++) {
      (parent -> survival)[k] = 1.0;
    }
    mapLocalToTimeInterest(treeID,
                           parent,
                           parent -> localSurvival,
                           parent -> survival);
  }
  else {
    stackSurvival(parent, RF_sortedTimeInterestSize);
    for (k = 1; k <= RF_sortedTimeInterestSize; k++) {
      (parent -> survival)[k] = RF_TN_SURV_ptr[treeID][parent -> nodeID][k];
    }
  }
  //      if (getTraceFlag(treeID) & ENSB_LOW_TRACE) {
  //      if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //        RF_nativePrint("\nNode specific non-local survival function [RF_sortedTimeInterestSize] x [RF_tLeafCount[treeID]] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
  //        RF_nativePrint("                 time ");
  //        RF_nativePrint("\n");
  //        for (k=1; k <= RF_sortedTimeInterestSize; k++) {
  //          RF_nativePrint("%10d %10.4f %10.4f", k, RF_timeInterest[k], (parent -> survival)[k]);
  //          RF_nativePrint("\n");
  //        }
  //      }
  //      }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetSurvival() EXIT ...\n");
  //  }
}
void getNelsonAalen(uint treeID, Terminal *parent) {
  uint k;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetNelsonAalen() ENTRY ...\n");
  //  }
  if (!(RF_optHigh & OPT_TERM_INCG)) {
    stackNelsonAalen(parent, RF_sortedTimeInterestSize);
    for (k = 1; k <= RF_sortedTimeInterestSize; k++) {
      (parent -> nelsonAalen)[k] = 0.0;
    }
    mapLocalToTimeInterest(treeID,
                           parent,
                           parent -> localNelsonAalen,
                           parent -> nelsonAalen);
  }
  else {
    stackNelsonAalen(parent, RF_sortedTimeInterestSize);
    for (k = 1; k <= RF_sortedTimeInterestSize; k++) {
      (parent -> nelsonAalen)[k] = RF_TN_NLSN_ptr[treeID][parent -> nodeID][k];
    }
  }
  //  if (getTraceFlag(treeID) & ENSB_LOW_TRACE) {
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //      RF_nativePrint("\nNelson-Aalen estimator matrix for (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
  //      for (k=1; k <= RF_sortedTimeInterestSize; k++) {
  //        RF_nativePrint("%10d", k);
  //      }
  //      RF_nativePrint("\n");  
  //      for (k=1; k <= RF_sortedTimeInterestSize; k++) {
  //          RF_nativePrint("%10.4f", parent -> nelsonAalen[k]);
  //      }
  //      RF_nativePrint("\n");
  //    }
  //  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetNelsonAalen() EXIT ...\n");
  //  }
}
void getCSH(uint treeID, Terminal *parent) {
  uint j, k;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetCSH() ENTRY ...\n");
  //  }
  if (!(RF_optHigh & OPT_TERM_INCG)) {
    stackCSH(parent, RF_eventTypeSize, RF_sortedTimeInterestSize);
    for (j=1; j <= RF_eventTypeSize; j++) {
      for (k=1; k <= RF_sortedTimeInterestSize; k++) {
        (parent -> CSH)[j][k] = 0.0;
      }
    }
    mapLocalToTimeInterest(treeID,
                           parent,
                           parent -> localCSH,
                           parent -> CSH);
  }
  else {
    stackCSH(parent, RF_eventTypeSize, RF_sortedTimeInterestSize);
    for (j = 1; j <= RF_eventTypeSize; j++) {
      for (k = 1; k <= RF_sortedTimeInterestSize; k++) {
        (parent -> CSH)[j][k] = RF_TN_CSHZ_ptr[treeID][parent -> nodeID][j][k];
      }
    }
  }
  //  if (getTraceFlag(treeID) & ENSB_LOW_TRACE) {
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //        RF_nativePrint("\nNode specific non-local CSH:  [RF_eventTypeSize] x [nodeEventTimeSize] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
  //        RF_nativePrint("                 time ");
  //        for (j=1; j <= RF_eventTypeSize; j++) {
  //          RF_nativePrint("%10d ", j);
  //        }
  //        RF_nativePrint("\n");
  //        for (k=1; k <= RF_sortedTimeInterestSize; k++) {
  //          RF_nativePrint("%10d %10.4f ", k, RF_timeInterest[k]);
  //          for (j=1; j <= RF_eventTypeSize; j++) {
  //            RF_nativePrint("%10.4f ", (parent -> CSH)[j][k]);
  //          }
  //          RF_nativePrint("\n");
  //        }
  //      }
  //      }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetCSH() EXIT ...\n");
  //  }
}
void getCIF(uint treeID, Terminal *parent) {
  uint j, k;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetCIF() ENTRY ...\n");
  //  }
  if (!(RF_optHigh & OPT_TERM_INCG)) {
    stackCIF(parent, RF_eventTypeSize, RF_sortedTimeInterestSize);
    for (j=1; j <= RF_eventTypeSize; j++) {
      for (k=1; k <= RF_sortedTimeInterestSize; k++) {
        (parent -> CIF)[j][k] = 0.0;
      }
    }
    mapLocalToTimeInterest(treeID,
                           parent,
                           parent -> localCIF,
                           parent -> CIF);
  }
  else {
    stackCIF(parent, RF_eventTypeSize, RF_sortedTimeInterestSize);
    for (j = 1; j <= RF_eventTypeSize; j++) {
      for (k = 1; k <= RF_sortedTimeInterestSize; k++) {
        (parent -> CIF)[j][k] = RF_TN_CIFN_ptr[treeID][parent -> nodeID][j][k];
      }
    }
  }
  //  if (getTraceFlag(treeID) & ENSB_LOW_TRACE) {
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //        RF_nativePrint("\nNode specific non-local CIF:  [RF_eventTypeSize] x [nodeEventTimeSize] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
  //        RF_nativePrint("                 time ");
  //        for (j=1; j <= RF_eventTypeSize; j++) {
  //          RF_nativePrint("%10d ", j);
  //        }
  //        RF_nativePrint("\n");
  //        for (k=1; k <= RF_sortedTimeInterestSize; k++) {
  //          RF_nativePrint("%10d %10.4f ", k, RF_timeInterest[k]);
  //          for (j=1; j <= RF_eventTypeSize; j++) {
  //            RF_nativePrint("%10.4f ", (parent -> CIF)[j][k]);
  //          }
  //          RF_nativePrint("\n");
  //        }
  //    }
  //  }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetCIF() EXIT ...\n");
  //  }
}
void getMortality(uint treeID, Terminal *parent) {
  uint j, q;
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetMortality() ENTRY ...\n");
  //  }
  if (!(RF_optHigh & OPT_TERM_INCG)) {
    stackMortality(parent, RF_eventTypeSize);
    for (j = 1; j <= RF_eventTypeSize; j++) {
      (parent -> mortality)[j] = 0.0;
    }
    if (!(RF_opt & OPT_COMP_RISK)) {
      for (q = 1; q <= RF_sortedTimeInterestSize; q++) {
        (parent -> mortality)[1] += (parent -> nelsonAalen)[q];
      }
    }
    else {
      for (j = 1; j <= RF_eventTypeSize; j ++) {
        for (q = 1; q <= RF_sortedTimeInterestSize - 1; q++) {
          (parent -> mortality)[j] += (parent -> CIF)[j][q] * (RF_timeInterest[q+1] - RF_timeInterest[q]);
        }
      }
    }
  }
  else {
    stackMortality(parent, RF_eventTypeSize);
    for (j = 1; j <= RF_eventTypeSize; j++) {
      (parent -> mortality)[j] = RF_TN_MORT_ptr[treeID][parent -> nodeID][j];
    }
  }
  //      if (getTraceFlag(treeID) & ENSB_LOW_TRACE) {
  //      if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //        RF_nativePrint("\nNode specific mortality:  [RF_eventTypeSize] for:  (tree, leaf) = (%10d, %10d)  \n", treeID, parent -> nodeID);
  //        for (j=1; j <= RF_eventTypeSize; j++) {
  //          RF_nativePrint("%10d ", j);
  //        }
  //        RF_nativePrint("\n");
  //        for (j=1; j <= RF_eventTypeSize; j++) {
  //          RF_nativePrint("%10.4f ", (parent -> mortality)[j]);
  //        }
  //        RF_nativePrint("\n");
  //      }
  //      }
  //  if (getTraceFlag(treeID) & SUMM_MED_TRACE) {
  //    RF_nativePrint("\ngetMortality() EXIT ...\n");
  //  }
}
