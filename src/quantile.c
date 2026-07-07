
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "quantile.h"
#include "nrutil.h"
#include "error.h"
QuantileObj *makeQuantileObj(double value) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nmakeQuantileObj() ENTRY ...\n");
  //  }
  QuantileObj *quantileObj = (QuantileObj*) gblock((size_t) sizeof(QuantileObj));
  quantileObj -> fwdLink = NULL;
  quantileObj -> bakLink = NULL;
  quantileObj -> v = value;
  quantileObj -> g = 1;
  quantileObj -> dlt = 0;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nmakeQuantileObj() EXIT ...\n");
  //  }
  return quantileObj;
}
void freeQuantileObj(QuantileObj *obj) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeQuantileObj() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeing Quantile Object at %20x \n", obj);
  //  }
  free_gblock(obj, (size_t) sizeof(QuantileObj));
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeQuantileObj() EXIT ...\n");
  //  }
}
void freeQuantileObjList(QuantileObj *obj) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeQuantileObjList() ENTRY ...\n");
  //  }
  QuantileObj *thisObj, *fwdObj;
  thisObj = obj;
  while (thisObj != NULL) {
    fwdObj = thisObj -> fwdLink;
    freeQuantileObj(thisObj);
    thisObj = fwdObj;
  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeQuantileObjList() EXIT ...\n");
  //  }
}
QuantileObj *insertQuantileObj(uint *qStreamSize, QuantileObj **head, QuantileObj **tail, uint *quantileLinkLength, double value, LookUpInfo **tree) {
  QuantileObj *newObj;
  QuantileObj *insertPtr;
  QuantileObj *thisPtr;
  QuantileObj *segmentHead, *segmentTail, *delPtr, *savPtr;
  uint *band;
  uint p, gStar;
  uint gNew;
  char flag;
  //  uint s = 0;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\ninsertQuantileObj() ENTRY ...\n");
  //  }
  newObj = makeQuantileObj(value);
  if (*head == NULL) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\n  Insert Quantile Object:  null list encountered, creating object, init head and tail for value:  %10.4f \n", value);
  //  }
    *head = *tail = newObj;
    (*quantileLinkLength) ++;
    (*qStreamSize) ++;
  }
  else {
    if ( (((*qStreamSize) % ((uint) floor(RF_inv_2qEpsilon))) == 0) &&
         ((*qStreamSize) > (uint) floor(RF_inv_2qEpsilon)) && 
         ((*qStreamSize) > 2)) {
      p = (*qStreamSize) / (uint) floor(RF_inv_2qEpsilon);
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    s = *quantileLinkLength;
      //  }
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Compress Phase:     qStreamSize = %10d", *qStreamSize);
      //    RF_nativePrint("\n  Compress Phase: 2 x eps x n = p = %10d", p);
      //    RF_nativePrint("\n  Compress Phase:        s (iter) = %10d", s);      
      //  }
      //    if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //      QuantileObj *objPtr;
      //      uint sum;
      //      objPtr = *head;
      //      sum = 0;      
      //      RF_nativePrint("\n  Compress Phase:     incoming fwd parse linked list contents");
      //      RF_nativePrint("\n      index                               g      delta      value");
      //      for (uint k = 1; k <= *quantileLinkLength; k++) {
      //        sum += objPtr -> g;
      //        RF_nativePrint("\n %10d %20x %10d %10d %10.4f ", k, objPtr, objPtr -> g, objPtr -> dlt, objPtr -> v);
      //        objPtr = objPtr -> fwdLink;
      //      }
      //      RF_nativePrint("\n  Compress Phase:     consistency checksum_{i=1}^{n} g_i = %10d", sum);
      //    }
      //    if (getTraceFlag(0) & QUAN_DEF_TRACE) {      
      //    if (getTraceFlag(0) & TURN_OFF_TRACE) {
      //      QuantileObj *objPtr;
      //      objPtr = *tail;
      //      RF_nativePrint("\n  Compress Phase:     incoming bak parse linked list contents");
      //      RF_nativePrint("\n      index                               g      delta      value");
      //      for (uint k = *quantileLinkLength; k >= 1; k--) {
      //        RF_nativePrint("\n %10d %20x %10d %10d %10.4f ", k, objPtr, objPtr -> g, objPtr -> dlt, objPtr -> v);
      //        objPtr = objPtr -> bakLink;
      //      }
      //    }
      //    }
      band = uivector(0, p);
      populateBand(p, band);
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n Mapping of deltas to band identifiers:");
      //    for (uint j = 0; j <= p; j++) {
      //      RF_nativePrint("\n   band [%10d] = %20d", j, band[j]);  
      //    }
      //  }
      thisPtr = *tail;
      while(thisPtr != *head) {
        //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
        //    RF_nativePrint("\n  Compress Quantile Object:  examining thisPtr = s (addr, iter) = (%20x, %10d)", thisPtr, s);
        //  }
        segmentTail = thisPtr -> bakLink; 
        if (segmentTail != *head) {
          if (band[segmentTail -> dlt] <= band[thisPtr -> dlt]) {
            //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
            //    RF_nativePrint("\n  Compress Quantile Object:  adjacent tuples suitable (band[L], band[R]) = (%20d, %20d)", band[segmentTail -> dlt], band[thisPtr -> dlt]);
            //  }
            gStar = 0;
            segmentHead = segmentTail;
            flag = TRUE;
            while (flag && (segmentHead != (*head))) {
              gStar += (segmentHead -> g);
              //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //    RF_nativePrint("\n  Compress Quantile Object:  gStar = %10d", gStar);
              //  }
              segmentHead = segmentHead -> bakLink;
              //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //    s --;
              //  }
              //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //    RF_nativePrint("\n  Compress Quantile Object:  examining segmentHead = s (addr, iter) = (%20x, %10d)", segmentHead, s - 1);
              //  }
              if ((band[segmentHead -> dlt] < band[segmentTail -> dlt]) && segmentHead != (*head)) {
                flag = TRUE;
                //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
                //    RF_nativePrint("\n  Compress Quantile Object:  further descendant found, continue search ...");
                //  }
              }
              else {
                flag = FALSE;
                //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
                //    RF_nativePrint("\n  Compress Quantile Object:  further descendant NOT found, terminating search ...");
                //  }
                //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
                //    s --;
                //  }
              }
            }
            gNew = gStar + (thisPtr -> g);
            if (gNew + (thisPtr -> dlt) <= p) {
              //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //    RF_nativePrint("\n  Compress Quantile Object:  successful epsilon-approx guarantree (gStar + g_i + delta_i, p) = (%10d, %10d)", gNew + (thisPtr -> dlt), p);
              //  }
              delPtr = segmentHead -> fwdLink;
              segmentHead -> fwdLink = thisPtr;
              thisPtr -> bakLink = segmentHead;
              while (delPtr != thisPtr) {
                //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
                //    RF_nativePrint("\n  Compress Quantile Object:  deleting tuple (g, delta, v) = (%10d, %10d, %10.4f)", delPtr -> g, delPtr -> dlt, delPtr -> v);
                //  }
                savPtr = delPtr -> fwdLink;
                freeQuantileObj(delPtr);
                delPtr = savPtr;
                (*quantileLinkLength) --;
              }
              //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //    RF_nativePrint("\n  Compress Quantile Object:  receiving tuple (g, delta, v) = (%10d, %10d, %10.4f)", thisPtr -> g, thisPtr -> dlt, thisPtr -> v);
              //  }
              thisPtr -> g = gNew;
              //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //    RF_nativePrint("\n  Compress Quantile Object:  resultant tuple (g, delta, v) = (%10d, %10d, %10.4f)", thisPtr -> g, thisPtr -> dlt, thisPtr -> v);
              //  }
              //    if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //      QuantileObj *objPtr;
              //      uint sum;
              //      objPtr = *head;
              //      sum = 0;      
              //      for (uint k = 1; k <= *quantileLinkLength; k++) {
              //        sum += objPtr -> g;
              //        objPtr = objPtr -> fwdLink;
              //      }
              //      RF_nativePrint("\n  Compress Quantile Object:  consistency checksum_{i=1}^{n} g_i = %10d", sum);
              //      if (sum != *qStreamSize) {
              //        RF_nativeError("\nRF-SRC:  *** ERROR *** ");
              //        RF_nativeError("\nRF-SRC:  Quantile consistency checksum failed with (actual, expected) = (%10.d, %10.d)", sum, *qStreamSize);
              //        RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
              //        RF_nativeExit();
              //      }
              //    }
              thisPtr = segmentHead;
            }
            else {
              //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
              //    RF_nativePrint("\n  Compress Quantile Object:  failing epsilon-approx guarantree (gNew, p) = (%10d, %10d)", gNew, p);
              //    RF_nativePrint("\n  Compress Quantile Object:  terminating iteration");
              //  }
              thisPtr = segmentHead;
            }
          }
          else {
            thisPtr = thisPtr -> bakLink;
            //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
            //    RF_nativePrint("\n  Compress Quantile Object:  adjacent tuples not capacity compatible");
            //  }
            //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
            //    s --;
            //  }
          }
        }
        else {
          thisPtr = thisPtr -> bakLink;
          //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
          //    RF_nativePrint("\n  Compress Quantile Object:  segment tail starts at head");
          //    RF_nativePrint("\n  Compress Quantile Object:  terminating iteration");
          //  }
        }
      }
      free_uivector(band, 0, p);
      if (*tree != NULL) {
        //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
        //    RF_nativePrint("\n  LookUp Tree:  freeing current tree");
        //  }
        freeLookUpTree(*tree);
        *tree = NULL;
      }
      if (*quantileLinkLength >= 8) {
        //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
        //    RF_nativePrint("\n  LookUp Tree:  creating and populating new search tree.");
        //  }
        *tree = makeLookUpInfo();
        uint depth = ulog2(*quantileLinkLength) - 2;
        makeLookUpTree(*tree, *head, *quantileLinkLength, depth);
      }
      else {
        //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
        //    RF_nativePrint("\n  LookUp Tree:  search tree omitted, link length is %10d", *quantileLinkLength);
        //  }
      }
    }
    else {
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Compress Quantile Object:  OMITTED:     2 x epsilon = %10d, qStreamsize = %10d", (uint) floor(RF_inv_2qEpsilon), *qStreamSize);
      //  }
    }
    if (value <= (*head) -> v) {
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Insert Quantile Object:  creating new head for value:  %10.4f \n", value);
      //  }
      (*head) -> bakLink = newObj;
      newObj -> fwdLink = *head;
      *head = newObj;
      newObj -> g = 1;
      newObj -> dlt = 0;
      (*quantileLinkLength) ++;
      (*qStreamSize) ++;
    }
    else if (value >= (*tail) -> v) {
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Insert Quantile Object:  creating new tail for value:  %10.4f \n", value);
      //  }
      (*tail) -> fwdLink = newObj;
      newObj -> bakLink = *tail;
      *tail = newObj;
      newObj -> g = 1;
      newObj -> dlt = 0;
      (*quantileLinkLength) ++;
      (*qStreamSize) ++;
    }
    else {
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Insert Quantile Object:  finding insertion point for value:  %10.4f \n", value);
      //  }
      insertPtr = findInsertionPoint(*head, value, *tree);
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Insert Quantile Object:  insertion point found at:  %20x, value =  %10.4f \n", insertPtr -> v);
      //  }
      (insertPtr -> bakLink) -> fwdLink = newObj;
      newObj -> bakLink = insertPtr -> bakLink;
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Insert Quantile Object:  backward linking complete");
      //  }
      insertPtr -> bakLink = newObj;
      newObj -> fwdLink = insertPtr;
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  Insert Quantile Object:  forward  linking complete");
      //  }
      newObj -> g = 1;
      if ((double) *qStreamSize <= RF_inv_2qEpsilon) {
        newObj -> dlt = 0;
      }
      else {
        newObj -> dlt = (insertPtr -> g) + (insertPtr -> dlt) - 1;
      }
      (*quantileLinkLength) ++;
      (*qStreamSize) ++;
    }
  }
      //    if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //      QuantileObj *objPtr;
      //      uint sum;
      //      objPtr = *head;
      //      sum = 0;      
      //      RF_nativePrint("\n  Insert Phase:     outgoing fwd parse linked list contents");
      //      RF_nativePrint("\n      index                               g      delta      value");
      //      for (uint k = 1; k <= *quantileLinkLength; k++) {
      //        sum += objPtr -> g;
      //        RF_nativePrint("\n %10d %20x %10d %10d %10.4f ", k, objPtr, objPtr -> g, objPtr -> dlt, objPtr -> v);
      //        objPtr = objPtr -> fwdLink;
      //      }
      //      RF_nativePrint("\n  Insert Quantile Object:     consistency checksum_{i=1}^{n} g_i = %10d", sum);
      //    }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\ninsertQuantileObj() EXIT ...\n");
  //  }
  return newObj;
}
QuantileObj *findInsertionPoint(QuantileObj *head, double value, LookUpInfo *tree) {
  QuantileObj *insertPtr;
  char found;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfindInsertionPoint() ENTRY ...\n");
  //  }
  found = FALSE;
  if (tree == NULL) {
    insertPtr = head;
  }
  else {
    findApproximateInsertionPoint(head, tree, value, &insertPtr);
    //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
    //    RF_nativePrint("\n Approximate insertion point:  obj = %20x, Vapprox = %10.4f, Vactual = %10.4f", insertPtr, insertPtr -> v, value);
    //  }
  }
  while (!found) {
    if (insertPtr != NULL) {
      if (value > insertPtr -> v) {
        insertPtr = insertPtr -> fwdLink;
      }
      else {
        found = TRUE;
      }
    }
    else {
      insertPtr = NULL;
    }
  }   
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfindInsertionPoint() EXIT ...\n");
  //  }
  return insertPtr;
}
double getApproxQuantile(QuantileObj *head, double phi, uint streamSize) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\ngetApproxQuantile() ENTRY ...\n");
  //  }
  double rank, margin;
  QuantileObj *currentObj;
  double rmax, rmin;
  double result;
  char found;
  rank = ceil(phi * streamSize);
  margin = RF_qEpsilon * streamSize;
  found = FALSE;
  currentObj = head;
  result = RF_nativeNaN; 
  rmin = 0;
  while (!found) {
    if (currentObj != NULL) {
      rmin += (double) (currentObj -> g);
      rmax = rmin + (double) (currentObj -> dlt);
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n approximation details: ");
      //    RF_nativePrint("\n    margin = %10.4f", margin);
      //    RF_nativePrint("\n    rank   = %10.4f", rank);
      //    RF_nativePrint("\n    (rmin, rmax) = (%10.4f, %10.4f)", rmin, rmax);
      //    RF_nativePrint("\n    ((rank - rmin), (rmax - rank) = (%10.4f, %10.4f)", rank - rmin, rmax - rank);
      //  }
      if ( (((rank - rmin) <= margin) && ((rmax - rank) <= margin)) ) {
        found = TRUE;
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n coverage found");
      //  }
      }
      else if ((uint) rmin == streamSize) {
        found = TRUE;
        //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
        //    RF_nativePrint("\n rmin = mrax = streamSize, found");
        //  }
      }
      else {
        currentObj = currentObj -> fwdLink;
        //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
        //    RF_nativePrint("\n coverage NOT found, iterating forward");
        //  }
      }
    }
    else {
      RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      RF_nativeError("\nRF-SRC:  Quantile query failed with (epsilon, phi) -> (margin, rank) => (%10.4f, %10.4f) -> (%10.4f, %10d)", RF_qEpsilon, phi, margin, (uint) rank);
      RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      RF_nativeExit();
    }
  }
  result = currentObj -> v;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\n approximation with (epsilon, phi) -> (margin, rank) => (%10.4f, %10.4f) -> (%10.4f, %10d)", RF_qEpsilon, phi, margin, (uint) rank);
  //    RF_nativePrint("\n   quantile approximation = %10.4f", result);
  //  }
  return result;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\ngetApproxQuantile() EXIT ...\n");
  //  }
}
void populateBand(uint p, uint *band) {
  uint alpha, alphaPowLo, alphaPowHi;
  uint j;
  uint lower, upper;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\npopulateBand() ENTRY ...\n");
  //  }
  uint alphaLimit = ulog2(p);
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nBand Maximum alhpa:  %10d", alphaLimit);
  //  }
  band[0]   = INT_MAX;
  band[p] = 0;
  for (alpha = 1; alpha <= alphaLimit; alpha ++) {
    alphaPowLo = 1 << (alpha - 1);
    alphaPowHi = 1 << alpha;
    lower = p - alphaPowHi - (p % alphaPowHi);
    upper = p - alphaPowLo - (p % alphaPowLo);
    for (j = upper; j > lower; j--) {
      band[j] = alpha;
    }
  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\npopulateBand() ENTRY ...\n");
  //  }
}
void makeLookUpTree(LookUpInfo *infoObj, QuantileObj *qObj, uint size, uint depth) {
  QuantileObj *qPtr;
  uint half;
  uint i;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nmakeLookUpTree() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\n  makeLookUpTree() called with:  GKLI = %20x, QO = %20x, size = %10d, depth = %10d", infoObj, qObj, size, depth);
  //  }
  half = (size >> 1);
  qPtr = qObj;
  for (i = 1; i < half; i++) {
    qPtr = qPtr -> fwdLink;
  }
  infoObj -> qPtr = qPtr;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\n  G-K LookUp Object initialization:  half = %10d, QO = %20x, v = %10.4f", half, qPtr, qPtr -> v);
  //  }
  if (half > 1) {
    if (depth > 1) {      
      LookUpInfo *leftPtr = makeLookUpInfo();
      infoObj -> leftPtr = leftPtr;
      leftPtr -> rootPtr = infoObj;
      makeLookUpTree(leftPtr, qObj, half, depth - 1);
      LookUpInfo *rghtPtr = makeLookUpInfo();
      infoObj -> rghtPtr = rghtPtr;
      rghtPtr -> rootPtr = infoObj;
      makeLookUpTree(rghtPtr, qPtr, size - half, depth - 1);
    }
    else {
    //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
    //    RF_nativePrint("\n  G-K LookUp Object tree depth limit reached:  depth = %10d", depth);
    //  }
    }
  }
  else {
    //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
    //    RF_nativePrint("\n  G-K LookUp Object segment limit reached:  half = %10d", half);
    //  }
  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nmakeLookUpTree() EXIT ...\n");
  //  }
}
LookUpInfo *makeLookUpInfo(void) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nmakeLookUpInfo() ENTRY ...\n");
  //  }
  LookUpInfo *obj = (LookUpInfo*) gblock((size_t) sizeof(LookUpInfo));
  obj -> qPtr    = NULL;
  obj -> rootPtr = NULL;
  obj -> leftPtr = NULL;
  obj -> rghtPtr = NULL;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nmakeLookUpInfo() EXIT ...\n");
  //  }
  return obj;
}
void freeLookUpInfo(LookUpInfo *obj) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeLookUpInfo() ENTRY ...\n");
  //  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeing LookUpInfo object at %20x \n", obj);
  //  }
  free_gblock(obj, (size_t) sizeof(LookUpInfo));
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeLookUpInfo() EXIT ...\n");
  //  }
}
void freeLookUpTree(LookUpInfo *obj) {
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeLookUpTree() ENTRY ...\n");
  //  }
  if (obj != NULL) {
    if ((obj -> leftPtr != NULL) && (obj -> rghtPtr != NULL)) {
      freeLookUpTree(obj -> leftPtr);
      freeLookUpTree(obj -> rghtPtr);
    }
    freeLookUpInfo(obj);
  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfreeLookUpTree() EXIT ...\n");
  //  }
}
void findApproximateInsertionPoint(QuantileObj *head, LookUpInfo *tree, double value, QuantileObj **insertPtr) {
  char foundFlag;
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfindApproximateInsertionPoint() ENTRY ...\n");
  //  }
  if (value < (tree -> qPtr) -> v) {
    if (tree -> leftPtr != NULL) {
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  finding approximate insertion:  parsing left with (value, mid)  = (%10.4f, %10.4f)", value, (tree -> qPtr) -> v);
      //  }
      findApproximateInsertionPoint(head, tree -> leftPtr, value, insertPtr);
    }
    else {
      foundFlag = FALSE;
      while(!foundFlag) {
        tree = tree -> rootPtr;
        if (tree != NULL) {
          if (value < ((tree -> qPtr) -> v)) {
            //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
            //    RF_nativePrint("\n  finding approximate insertion:  parsing upward with (value, mid)  = (%10.4f, %10.4f)", value, (tree -> qPtr) -> v);
            //  }
          }
          else {
            foundFlag = TRUE;
            *insertPtr = tree -> qPtr;
            //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
            //    RF_nativePrint("\n  finding approximate insertion:  parsing stopped nominal with (value, mid)  = (%10.4f, %10.4f)", value, (tree -> qPtr) -> v);
            //  }
          }
        }
        else {
          foundFlag = TRUE;
          *insertPtr = head;
          //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
          //    RF_nativePrint("\n  finding approximate insertion:  parsing stopped root with (value, mid)  = (%10.4f, %10.4f)", value, head -> v);
          //  }
        }
      }
    }
  }
  else if (value > (tree -> qPtr) -> v) {
    if (tree -> rghtPtr != NULL) {
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  finding approximate insertion:  parsing right with (value, mid)  = (%10.4f, %10.4f)", value, (tree -> qPtr) -> v);
      //  }
      findApproximateInsertionPoint(head, tree -> rghtPtr, value, insertPtr);
    }
    else {
      *insertPtr = tree -> qPtr;
      //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
      //    RF_nativePrint("\n  finding approximate insertion:  parsing stopped terminal with (value, mid)  = (%10.4f, %10.4f)", value, (tree -> qPtr) -> v);
      //  }
    }
  }
  else {
    *insertPtr = tree -> qPtr;
    //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
    //    RF_nativePrint("\n  finding approximate insertion:  parsing stopped exact with (value, mid)  = (%10.4f, %10.4f)", value, (tree -> qPtr) -> v);
    //  }
  }
  //  if (getTraceFlag(0) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\nfindApproximateInsertionPoint() EXIT ...\n");
  //  }
}
void testQuantile(uint treeID) {
  QuantileObj *head, *tail;
  uint streamSize;
  uint quantileLinkLength;
  LookUpInfo *ghiPtr;
  //  if (getTraceFlag(treeID) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\ntestQuantile(%10d) ENTRY ...\n", treeID);
  //  }
  head = tail = NULL;
  streamSize = 0;
  quantileLinkLength = 0;
  uint size = RF_observationSize;
  ghiPtr = NULL;
  for (uint i = 1; i <= size; i++) {
    insertQuantileObj(&streamSize, &head, &tail, &quantileLinkLength,  RF_response[treeID][1][i], &ghiPtr); 
  }
  //    if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
  //      QuantileObj *objPtr = head;
  //      RF_nativePrint("\nFinal Quantile Linked List:  ");
  //      RF_nativePrint("\n      index          g      delta      value");
  //      for (uint k = 1; k <= quantileLinkLength; k++) {
  //        RF_nativePrint("\n %10d %10d %10d %10.4f ", k, objPtr -> g, objPtr -> dlt, objPtr -> v);
  //        objPtr = objPtr -> fwdLink;
  //      }
  //    }
  if (!FALSE) {
    for (uint i = 1; i <= RF_quantileSize; i++) {
      getApproxQuantile(head, RF_quantile[i], streamSize);
    }
  }
  if (ghiPtr != NULL) {
    freeLookUpTree(ghiPtr);
  }
  freeQuantileObjList(head);
  //  if (getTraceFlag(treeID) & QUAN_DEF_TRACE) {
  //    RF_nativePrint("\ntestQuantile(%10d) EXIT ...\n", treeID);
  //  }
}
