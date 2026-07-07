
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "sampling.h"
#include "nrutil.h"
// #include "error.h"
DistributionObj *makeDistributionObjRaw(void) {
  DistributionObj *obj = (DistributionObj*) gblock((size_t) sizeof(DistributionObj));
  return obj;
}
DistributionObj *makeDistributionObjFull(void) {
  DistributionObj *obj = (DistributionObj*) gblock((size_t) sizeof(DistributionObj));
  obj -> permissibleIndex  = NULL;
  obj -> permissible       = NULL;
  obj -> permissibleSize   = 0;
  obj -> augmentationSize    = NULL;
  obj -> weightType       = 0;
  obj -> weight           = NULL;
  obj -> weightSorted     = NULL;
  obj -> cdf     = NULL;
  obj -> cdfSize = 0;
  obj -> cdfSort = NULL;
  obj -> density          = NULL;
  obj -> densityAllocSize = 0;
  obj -> densitySize      = 0;
  obj -> densitySwap      = NULL;
  obj -> index           = NULL;
  obj -> indexSize       = 0;
  obj -> uIndexAllocSize = 0;
  obj -> slot            = 0;
  return obj;
}
void freeDistributionObjRaw(DistributionObj *obj) {
  free_gblock(obj, (size_t) sizeof(DistributionObj));
}
void initializeCDFNew(uint treeID, DistributionObj *obj) {
  char validElement;
  uint i, j, k, kk;
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\ninitializeCDFNew() ENTRY ...\n");
  //  }
  switch (obj -> weightType) {
  case RF_WGHT_UNIFORM:
    if (obj -> permissible != NULL) {
      if (obj -> augmentationSize != NULL) {
        obj -> uIndexAllocSize = obj -> permissibleSize + 
          obj -> augmentationSize[1] +
          obj -> augmentationSize[2] +
          (RF_xSize * (obj -> augmentationSize[2])) +
          ((obj -> augmentationSize[1]) * (obj -> augmentationSize[2]));
      }
      else {
        obj -> uIndexAllocSize = obj -> permissibleSize;
      }
      obj -> index = uivector(1, obj -> uIndexAllocSize);
      obj -> indexSize = 0;
      for (k = 1; k <= obj -> permissibleSize; k++) {
        if (obj -> permissible[k]) {
          obj -> index[++(obj -> indexSize)] = k;
        }
      }
    }
    else {
      obj -> index = uivector(1, obj -> permissibleSize);
      obj -> indexSize = obj -> uIndexAllocSize = obj -> permissibleSize;
      for (k=1; k <= obj -> permissibleSize; k++) {
        obj -> index[k] = obj -> permissibleIndex[k];
      }
    }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nUniform of Size: %10d", obj -> uIndexAllocSize);
    //      }
    //      }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nVector Weights:  Uniform");
    //        RF_nativePrint("\n     index      absolute");
    //        for (k=1; k <= obj -> uIndexAllocSize; k++) {
    //          RF_nativePrint("\n%10d  %10d", k, obj -> index[k]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //      }
    break;
  case RF_WGHT_INTEGER:
    obj -> density = uivector(1, obj -> densityAllocSize);
    obj -> densitySize = 0;
    obj -> densitySwap = (uint **) new_vvector(1, obj -> permissibleSize, NRUTIL_UPTR);
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //        RF_nativePrint("\nDensity Swap Vector:  ");
    //      }
    for (k = obj -> permissibleSize; k >= 1; k--) {
      kk = obj -> weightSorted[k];
      validElement = TRUE;
      if (obj -> permissible != NULL) {
        if (obj -> permissible[kk] == FALSE) {
          validElement = FALSE;
        }
      }
      if (validElement) {
        j = (uint) (obj -> weight)[kk];
        if (j > 0) {
          (obj -> densitySwap)[kk] = uivector(1, j);
          for (i = 1; i <= j; i++) {
            (obj -> density)[++(obj -> densitySize)] = kk;
            (obj -> densitySwap)[kk][i] = obj -> densitySize;
          }
          //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
          //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
          //        RF_nativePrint("\nSwap Vector for (Weight Index, of Length):  (%10d, %10d)", kk, j);
          //        RF_nativePrint("\n     index          swap");
          //        for (i=1; i <= j; i++) {
          //          RF_nativePrint("\n%10d  %12d", i, (obj -> densitySwap)[kk][i]);
          //        }
          //        RF_nativePrint("\n");
          //      }
          //      }
        }
        else {
          (obj -> densitySwap)[kk] = NULL;
          //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
          //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
          //        RF_nativePrint("\nSwap Vector for (Weight Index, of Length):  (%10d, %10d)", kk, 0);
          //      }
          //      }
        }
      }
      else {
        (obj -> densitySwap)[kk] = NULL;
          //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
          //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
          //        RF_nativePrint("\nSwap Vector for (Weight Index, of Length):  (%10d, %10d)", kk, 0);
          //      }
          //      }
      }
    }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //        RF_nativePrint("\nVector Weights Integer Size: %10d", obj -> densitySize);
    //      }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nVector Weights:  Integer");
    //        RF_nativePrint("\n     index          CWDV");
    //        for (i=1; i <= (obj -> densitySize); i++) {
    //          RF_nativePrint("\n%10d  %12d", i, (obj -> density)[i]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //      }
    break;
  case RF_WGHT_GENERIC:
    obj -> index = uivector(1, obj -> permissibleSize);
    obj -> cdf     =  dvector(1, obj -> permissibleSize);
    obj -> cdfSize = 0;
    i = 0;
    for (k = 1; k <= obj -> permissibleSize; k++) {
      kk = obj -> weightSorted[k];
      validElement = TRUE;
      if (obj -> permissible != NULL) {
        if (obj -> permissible[kk] == FALSE) {
          validElement = FALSE;
        }
      }
      if (validElement) {
        if (obj -> weight[kk] > 0) {
          (obj -> index)[++i] = kk;
          (obj -> cdfSize) ++;
          (obj -> cdf)[obj -> cdfSize] = obj -> weight[kk];
        }
      }
    }
    for (k = 2; k <= obj -> cdfSize; k++) {
      (obj -> cdf)[k] += (obj -> cdf)[k-1];
    }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //        RF_nativePrint("\nIncoming Weights Generic Size:  %10d", obj -> cdfSize);
    //      }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nIncoming Weights:  Generic");
    //        RF_nativePrint("\n  (note that sorting is ignored)");
    //        RF_nativePrint("\n       slot      index                            CDF");
    //        for (k=1; k <= obj -> cdfSize; k++) {
    //          RF_nativePrint("\n %10d %10d %30.24e", k, obj -> index[k], obj -> cdf[k]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //      }
    break;
  }
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\ninitializeCDFNew() EXIT ...\n");
  //  }
}
uint sampleFromCDFNew (float (*genericGenerator) (uint), uint treeID, DistributionObj *obj) {
  double randomValue;
  double midValue;
  char flag;
  uint low, mid, high, value;
  uint p;
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\nsampleFromCDFNew() ENTRY ...\n");
  //  }
  // p = 0;
  value = 0;  
  switch (obj -> weightType) {
  case RF_WGHT_UNIFORM:
    if (obj -> indexSize > 0) {
      obj -> slot = (uint) ceil(genericGenerator(treeID) * (obj -> indexSize * 1.0));
      value = obj -> index[obj -> slot];
    }
    else {
      value = obj -> slot = 0;
    }
    break;
  case RF_WGHT_INTEGER:
    if (obj -> densitySize > 0) {
      p = (uint) ceil(genericGenerator(treeID) * (obj -> densitySize * 1.0));
      value = obj -> slot = obj -> density[p];
    }
    else {
      value = obj -> slot = 0;
    }
    break;
  case RF_WGHT_GENERIC:
    if (obj -> cdf[obj -> cdfSize] > 0) {
      randomValue = genericGenerator(treeID) * (obj -> cdf)[obj -> cdfSize];
      //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
      //    RF_nativePrint("\nRandom Value:  %30.24e", randomValue);
      //  }
      low  = mid = 1;
      high = obj -> cdfSize;
      while (low < high) {
        mid  = (low + high) >> 1;
        //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
        //    RF_nativePrint("\nBinary Search (low, mid, high) = (%10d, %10d, %10d)", low, mid, high);
        //  }
        if (randomValue > obj -> cdf[mid]) {
          //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
          //    RF_nativePrint("\nBinary Search:  Upper with (random, cdf[mid] = (%30.24e %30.24e)", randomValue, obj -> cdf[mid]);
          //  }
          if (low == mid) {
            low = high;
            mid = high;
            if (obj -> cdf[mid] == 0) {
              mid ++;
              //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
              //    RF_nativePrint("\nZero level CDF adjustment:  %10d", mid);
              //  }
            }
          }
          else {
            low = mid;
          }
        }
        else {
          //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
          //    RF_nativePrint("\nBinary Search:  Lower with (random, cdf[mid]) = (%30.24e %30.24e)", randomValue, obj -> cdf[mid]);
          //  }
          if (low == mid) {
            low = high;
            if (obj -> cdf[mid] == 0) {
              mid ++;
              //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
              //    RF_nativePrint("\nZero level CDF adjustment:  %10d", mid);
              //  }
            }
          }
          else {
            high = mid;
          }
        }
      }
      midValue = obj -> cdf[mid];
      flag = TRUE;
      while (flag) {
        if (mid > 1) {
          if (midValue == obj -> cdf[mid-1]) {
            mid --;
            //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
            //    RF_nativePrint("\nFlat-line mid adjustment:  %10d", mid);
            //  }
          }
          else {
            flag = FALSE;
          }
        }
        else {
          flag = FALSE;
        }
      }
      obj -> slot = mid;
      value = obj -> index[obj -> slot];
    }
    else {
      value = obj -> slot = 0;
    }
    break;
  }
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\nSelected element:  %10d", value);
  //  }
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\nsampleFromCDFNew() EXIT ...\n");
  //  }
  return value;
}
void updateCDFNew(uint    treeID, DistributionObj *obj) {
  uint sourcePt;
  uint stepIndex;
  uint currCov, nextCov;
  double oldStepValue, newStepValue;
  char flag;
  uint   i, j, k;
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\nupdateCDF() ENTRY ...\n");
  //  }
  switch (obj -> weightType) {
  case RF_WGHT_UNIFORM:
    obj -> index[obj -> slot] = obj -> index[obj -> indexSize];
    (obj -> indexSize) --;
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nUpdated Incoming Weights Uniform Size:  %10d ", obj -> indexSize);
    //      }
    //      }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nUpdated Incoming Weights:  Uniform");
    //        RF_nativePrint("\n     index      absolute");
    //        for (k=1; k <= obj -> indexSize; k++) {
    //          RF_nativePrint("\n%10d  %10d", k, obj -> index[k]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //      }
    break;
  case RF_WGHT_INTEGER:
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //        RF_nativePrint("\nIncoming Weights Integer Size (before):  %10d", obj -> densitySize);
    //      }
    currCov = nextCov = obj -> density[obj -> densitySize];
    i = 0;
    j = (uint) (obj -> weight)[currCov];
    k = (uint) (obj -> weight)[obj -> slot];
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //        if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //          RF_nativePrint("\nLength of end element weight:       %10d", j);
    //          RF_nativePrint("\nLength of selected element weight:  %10d", k);
    //        }
    //      }
    while(i < k) {
      if (obj -> density[obj -> densitySize] == obj -> slot) {
        //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
        //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
        //        RF_nativePrint("\nWeight iterator while found:  %10d", i);
        //        RF_nativePrint("\nTo be deleted element found at end of density vector at end position:    %10d", obj -> densitySize);
        //        RF_nativePrint("\nTo be deleted element found at end of density vector at swap index:      %10d", k);
        //      }
        //      }
        obj -> density[obj -> densitySize] = 0;
        (obj -> densitySize) --;
        (obj -> densitySwap)[obj -> slot][k] = 0;
        k--;
        if (obj -> densitySize > 0) {
          currCov = nextCov = obj -> density[obj -> densitySize];
          j = (uint) (obj -> weight)[currCov];
        }
      }
      else {
        i++;
        sourcePt = obj -> densitySwap[obj -> slot][i];
        obj -> density[sourcePt] = obj -> density[obj -> densitySize];
        //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
        //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
        //        RF_nativePrint("\nWeight iterator while not found:  %10d", i);
        //        RF_nativePrint("\nTo be deleted element found in density vector at position:      %10d", sourcePt);
        //        RF_nativePrint("\nTo be deleted element found in density vector at swap index:    %10d", i);
        //        RF_nativePrint("\nTo be swapped element found in density vector at end position:  %10d", obj -> densitySize);
        //        RF_nativePrint("\nTo be swapped element found in density vector at swap index:    %10d", j);
        //      }
        //      }
        obj -> density[obj -> densitySize] = 0;
        (obj -> densitySize) --;
        obj -> densitySwap[currCov][j] = obj -> densitySwap[obj -> slot][i];
        obj -> densitySwap[obj -> slot][i] = 0;
        nextCov = obj -> density[obj -> densitySize];
        if (nextCov == currCov) {
          j--;
        }
        else {
          //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
          //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
          //        RF_nativePrint("\nDone swapping element:  ");
          //        RF_nativePrint("\nOld x-var:   %10d", currCov);
          //        RF_nativePrint("\nOld weight:  %10d", (uint) (obj -> weight)[currCov]);
          //        RF_nativePrint("\nNew x-var:   %10d", nextCov);
          //        RF_nativePrint("\nNew weight:  %10d", (uint) (obj -> weight)[nextCov]);
          //      }
          //      }
          hpsortui(obj -> densitySwap[currCov], (uint) (obj -> weight)[currCov]);
          currCov = nextCov = obj -> density[obj -> densitySize];
          j = (uint) (obj -> weight)[currCov];
        }
      }
    }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nWeight Density Vector Integer Size (after):   %10d", obj -> densitySize);
    //      }
    //      }
    if (obj -> densitySize > 0) {
      if (nextCov == currCov) {
        hpsortui(obj -> densitySwap[currCov], (uint) (obj -> weight)[currCov]);
      }
    }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nUpdated Incoming Weights:  Integer");
    //        RF_nativePrint("\n     index          CWDV");
    //        for (i=1; i <= obj -> densitySize; i++) {
    //          RF_nativePrint("\n%10d  %12d", i, obj -> density[i]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //      }
    break;
  case RF_WGHT_GENERIC:
    stepIndex = obj -> slot;
    if (stepIndex == 1) {
      newStepValue = 0;
    }
    else {
      newStepValue = obj -> cdf[stepIndex - 1];
    }
    oldStepValue = obj -> cdf[stepIndex];
    k = stepIndex;
    flag = TRUE;
    while (flag) {
      if ((obj -> cdf)[k] == oldStepValue) {
        (obj -> cdf)[k] = newStepValue;
        k ++;
      }
      else {
        flag = FALSE;
      }
      if (k > obj -> cdfSize) {
        flag = FALSE;
      }
    }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nUpdated Incoming Weights Generic Step:  %10d", stepIndex);
    //      }
    //      }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //        RF_nativePrint("\nUpdated Outgoing Weights:   ");
    //        RF_nativePrint("\n     index                            CDF");
    //        for (k=1; k <= obj -> cdfSize; k++) {
    //          RF_nativePrint("\n%10d %30.24e", k, obj -> cdf[k]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    break;
  }
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\nupdateCDF() EXIT ...\n");
  //  }
}
void discardCDFNew(uint treeID, DistributionObj *obj) {
  uint k;
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\ndiscardCDFNew() ENTRY ...\n");
  //  }
  switch (obj -> weightType) {
  case RF_WGHT_UNIFORM:
    free_uivector(obj -> index, 1, obj -> uIndexAllocSize);
    break;
  case RF_WGHT_INTEGER:
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //        RF_nativePrint("\nVector Weights Integer Size: %10d", obj -> permissibleSize);
    //      }
    //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
    //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
    //        RF_nativePrint("\nVector Weights:  Integer");
    //        RF_nativePrint("\n     index          CWDV");
    //        for (uint i=1; i <= obj -> densityAllocSize; i++) {
    //          RF_nativePrint("\n%10d  %12d", i, obj -> density[i]);
    //        }
    //        RF_nativePrint("\n");
    //      }
    //      }
    free_uivector(obj -> density, 1, obj -> densityAllocSize);
    for (k = 1; k <= obj -> permissibleSize; k++) {
      if (obj -> densitySwap[k] != NULL) {
        //      if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
        //      if (getTraceFlag(treeID) | TURN_OFF_TRACE) {
        //        RF_nativePrint("\nSwap Vector for (Weight Index, of Length):  (%10d, %10d)", k, (uint) (obj -> weight)[k]);
        //        RF_nativePrint("\n     index          swap");
        //        for (int i=1; i <= (uint) (obj -> weight)[k]; i++) {
        //          RF_nativePrint("\n%10d  %12d", i, (obj -> densitySwap)[k][i]);
        //        }
        //        RF_nativePrint("\n");
        //      }
        //      }
        free_uivector(obj -> densitySwap[k], 1, (uint) (obj -> weight)[k]);
        obj -> densitySwap[k] = NULL;
      }
    }
    free_new_vvector(obj -> densitySwap, 1, obj -> permissibleSize, NRUTIL_UPTR);
    break;
  case RF_WGHT_GENERIC:
    free_uivector(obj -> index, 1, obj -> permissibleSize);
    free_dvector(obj -> cdf, 1, obj -> permissibleSize);
    break;
  }
  //  if (getTraceFlag(treeID) & SAMP_DEF_TRACE) {
  //    RF_nativePrint("\ndiscardCDFNew() EXIT ...\n");
  //  }
}
uint sampleUniformlyFromVector (uint    treeID,
                                uint   *index,
                                uint    size,
                                uint   *sampleSlot) {
  uint result;
  if (size > 0) {
    (*sampleSlot) = (uint) ceil(ran1B(treeID) * (size * 1.0));
    result = index[*sampleSlot];
  }
  else {
    result = 0;
  }
  return result;
}
