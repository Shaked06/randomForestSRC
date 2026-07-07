
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "splitMahalanobis.h"
#include "splitUtil.h"
#include "regression.h"
#include "svdUtil.h"
#include "nrutil.h"
// #include "error.h"
char mahalanobis (uint       treeID,
                  Node      *parent,
                  SplitInfoMax *splitInfoMax,
                  GreedyObj    *greedyMembr,
                  char       multImpFlag) {
  //  if (getTraceFlag(treeID) & SPLT_LOW_TRACE) {
  //    RF_nativePrint("\nmahalanobis(%10d) ENTRY ...\n", treeID);
  //  }
  uint     covariate;
  uint     covariateCount;
  double  *splitVector;
  uint     splitVectorSize;
  uint   *indxx;
  uint priorMembrIter, currentMembrIter;
  uint leftSize, rghtSize;
  char *localSplitIndicator;
  uint splitLength;
  void *splitVectorPtr;
  double *observation;
  char factorFlag;
  uint mwcpSizeAbsolute;
  char deterministicSplitFlag;
  char preliminaryResult, result;
  double delta;
  double deltaPartial;
  double partialLeft;
  double partialRght;
  uint i, j, r, r1, r2;
  localSplitIndicator    = NULL;  
  splitVector            = NULL;  
  splitVectorSize        = 0;     
  preliminaryResult = getPreSplitResult(treeID,
                                        parent,
                                        multImpFlag,
                                        TRUE);
  if (preliminaryResult) {
    uint  repMembrSize = parent -> repMembrSize;
    uint *repMembrIndx = parent -> repMembrIndx;
    char   *impurity   = cvector(1, RF_ySize);
    double *mean       = dvector(1, RF_ySize);
    double *variance   = dvector(1, RF_ySize);
    char impuritySummary;
    impuritySummary = FALSE;
    for (r = 1; r <= RF_ySize; r++)  {
      impurity[r] = getVarianceDoublePass(repMembrSize,
                                          repMembrIndx,
                                          0,
                                          NULL,
                                          RF_response[treeID][r],
                                          &mean[r],
                                          &variance[r]);
      impuritySummary = impuritySummary | impurity[r];
    }
    if (impuritySummary) {
      stackSplitPreliminary(repMembrSize,
                            & localSplitIndicator,
                            & splitVector);
      DistributionObj *distributionObj = stackRandomCovariates(treeID, parent);
      uint *impureIdx;
      uint  impureIdxCount;
      double **elStar;
      double **elStarTranspose;
      double **qStar;
      double **qStarPlus;
      double **u, *w, **v;      
      double *leftMean;
      double *rghtMean;
      double **leftCentered;
      double **rghtCentered;
      double **leftCenteredT;
      double **rghtCenteredT;
      double **tempResult;
      double **tempResult2;
      double *sumLeft         = dvector(1, RF_ySize);
      double *sumRght         = dvector(1, RF_ySize);
      impureIdx = uivector(1, RF_ySize);
      impureIdxCount = 0;
      for (r = 1; r <= RF_ySize; r++) {
        if (impurity[r]) {
          impureIdx[++impureIdxCount] = r;
        }  
      }  
      leftMean      = dvector(1, impureIdxCount);
      rghtMean      = dvector(1, impureIdxCount);
      leftCentered  = dmatrix(1, impureIdxCount, 1, 1);
      rghtCentered  = dmatrix(1, impureIdxCount, 1, 1);
      leftCenteredT = dmatrix(1, 1, 1, impureIdxCount);
      rghtCenteredT = dmatrix(1, 1, 1, impureIdxCount);
      elStar = dmatrix(1, repMembrSize, 1, impureIdxCount);
      //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
      //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
      //              RF_nativePrint("\nMean of responses:       index   response");
      //              for (uint rr = 1; rr <= impureIdxCount; rr++) {
      //                RF_nativePrint("\n                    %10d %10.4f", impureIdx[rr], mean[impureIdx[rr]]);
      //              }
      //            }
      //          }
      for (i = 1; i <= repMembrSize; i++) {
        for (uint rr = 1; rr <= impureIdxCount; rr++) {
          r = impureIdx[rr];
          elStar[i][rr] = RF_response[treeID][r][ repMembrIndx[i] ] - mean[r];
        }
      }
      qStarPlus = NULL;
      if (RF_qStarPlus != NULL) {
        qStarPlus = dmatrix(1, impureIdxCount, 1, impureIdxCount);
        for (uint rr1 = 1; rr1 <= impureIdxCount; rr1++) {
          r1 = impureIdx[rr1];
          for (uint rr2 = 1; rr2 <= impureIdxCount; rr2++) {
            r2 = impureIdx[rr2];
            qStarPlus[rr1][rr2] = RF_qStarPlus[r1][r2];
          }
        }
        elStarTranspose = NULL;
        qStar = NULL;
      }
      else {
        //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
        //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
        //              RF_nativePrint("\n elStar (of dim n x r'):  ");
        //              matrixPrint(elStar, repMembrSize, impureIdxCount);
        //            }
        //          }
        elStarTranspose = matrixTrans(elStar, repMembrSize, impureIdxCount);
        //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
        //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
        //              RF_nativePrint("\n elStarTranspose (of dim r' x n):  ");
        //              matrixPrint(elStarTranspose, impureIdxCount, repMembrSize);
        //            }
        //          }
        qStar = matrixMult(elStarTranspose, elStar, impureIdxCount, repMembrSize, impureIdxCount);
        //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
        //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
        //              RF_nativePrint("\n qStar (of dim r' x r'):  ");
        //              matrixPrint(qStar, impureIdxCount, impureIdxCount);
        //            }
        //          }
        svdcmp(qStar, impureIdxCount, impureIdxCount, &u, &w, &v);
        //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
        //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
        //              RF_nativePrint("\n SVD Check of qStar (qStar =? u x w x v):  ");
        //              svdchk(qStar, impureIdxCount, impureIdxCount, u, w, v);
        //            }
        //          }
        qStarPlus = svdinv(u, w, v, impureIdxCount, impureIdxCount, impureIdxCount);
        //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
        //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
        //            if (qStarPlus != NULL) {
        //              RF_nativePrint("\n qStar x qStarPlus (of dim r' x r'):  ");
        //              double **qStarChk = matrixMult(qStar, qStarPlus, impureIdxCount, impureIdxCount, impureIdxCount);
        //              matrixPrint(qStarChk, impureIdxCount, impureIdxCount);
        //              RF_nativePrint("\n qStarPlus x qStar (of dim r' x r'):  ");
        //              double **qStarChk2 = matrixMult(qStarPlus, qStar, impureIdxCount, impureIdxCount, impureIdxCount);
        //              matrixPrint(qStarChk2, impureIdxCount, impureIdxCount);
        //              free_dmatrix(qStarChk, 1, impureIdxCount, 1, impureIdxCount);
        //              free_dmatrix(qStarChk2, 1, impureIdxCount, 1, impureIdxCount);
        //            }
        //            }
        //          }
      }
      //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
      //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
      //            if (qStarPlus != NULL) {
      //              RF_nativePrint("\n qStarPlus (of dim r' x r'):  ");
      //              matrixPrint(qStarPlus, impureIdxCount, impureIdxCount);
      //            }
      //            }
      //          }
      double deltaMax;
      uint   indexMax;
      covariateCount = 0;
      while (selectRandomCovariates(treeID,
                                    parent,
                                    distributionObj,
                                    & factorFlag,
                                    & covariate,
                                    & covariateCount)) {
        splitVectorSize = stackAndConstructSplitVectorGenericPhase1(treeID,
                                                                    parent,
                                                                    covariate,
                                                                    splitVector,
                                                                    & indxx,
                                                                    multImpFlag);
        if (splitVectorSize >= 2) {
          splitLength = stackAndConstructSplitVectorGenericPhase2(treeID,
                                                                  parent,
                                                                  covariate,
                                                                  splitVector,
                                                                  splitVectorSize,
                                                                  & factorFlag,
                                                                  & deterministicSplitFlag,
                                                                  & mwcpSizeAbsolute,
                                                                  & splitVectorPtr);
          observation = RF_observation[treeID][covariate];
          leftSize = 0;
          priorMembrIter = 0;
          if (factorFlag == FALSE) {
            for (j = 1; j <= repMembrSize; j++) {
              localSplitIndicator[j] = RIGHT;
            }
          }
          deltaMax =  RF_nativeNaN;
          indexMax =  0;
          for (j = 1; j < splitLength; j++) {
            if (factorFlag == TRUE) {
              priorMembrIter = 0;
              leftSize = 0;
            }
            virtuallySplitNode(treeID,
                               parent,
                               factorFlag,
                               mwcpSizeAbsolute,
                               observation,
                               indxx,
                               splitVectorPtr,
                               j,
                               localSplitIndicator,
                               & leftSize,
                               priorMembrIter,
                               & currentMembrIter);
            rghtSize = repMembrSize - leftSize;
            //   if (getTraceFlag(treeID) & SPLT_MED_TRACE) {
            //     RF_nativePrint("\nNon-miss Node Size:  %10d, Non-miss Left Size:  %10d, Non-miss Right Size:  %10d", repMembrSize, leftSize, rghtSize);
            //   }
            //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
            //            if (getTraceFlag(treeID) & TURN_OFF_TRACE) {
            //              RF_nativePrint("\n PriorIter:     %10d  CurrentIter:   %10d", priorMembrIter, currentMembrIter);
            //            }
            //          }
            if ((leftSize != 0) && (rghtSize != 0)) {
              delta        = 0.0;
              deltaPartial = 0.0;
              if (qStarPlus != NULL) {
                for (uint rr = 1; rr <= impureIdxCount; rr++) {
                  leftMean[rr] = rghtMean[rr] = 0.0;
                }
                for (i = 1; i <= repMembrSize; i++) {
                  if (localSplitIndicator[i] == LEFT) {
                    for (uint rr = 1; rr <= impureIdxCount; rr++) {
                      r = impureIdx[rr];
                      leftMean[rr] += elStar[i][rr];
                    }
                  }
                  else {
                    for (uint rr = 1; rr <= impureIdxCount; rr++) {
                      r = impureIdx[rr];
                      rghtMean[rr] += elStar[i][rr];
                    }
                  }
                }
                //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
                //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
                //              RF_nativePrint("\nLeft and Right Response Means:       index   response       left      right");
                //            }
                //          }
                for (uint rr = 1; rr <= impureIdxCount; rr++) {
                  leftMean[rr] = leftMean[rr] / leftSize;
                  rghtMean[rr] = rghtMean[rr] / rghtSize;
                }
                //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
                //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
                //              for (uint rr = 1; rr <= impureIdxCount; rr++) {
                //                RF_nativePrint("\n                                %10d %10d %10.4f %10.4f",
                //                rr,
                //                impureIdx[rr],
                //                leftMean[rr],
                //                rghtMean[rr]);
                //              }
                //            }
                //          }
                partialLeft = partialRght = 0.0;
                for (i = 1; i <= repMembrSize; i++) {
                  if (localSplitIndicator[i] == LEFT) {
                    for (uint rr = 1; rr <= impureIdxCount; rr++) {
                      r = impureIdx[rr];
                      leftCentered[rr][1]  = elStar[i][rr] - leftMean[rr];  
                      leftCenteredT[1][rr] = leftCentered[rr][1];
                    }
                    tempResult = matrixMult(leftCenteredT, qStarPlus, 1, impureIdxCount, impureIdxCount);
                    tempResult2 = matrixMult(tempResult, leftCentered, 1, impureIdxCount, 1);
                    partialLeft += tempResult2[1][1];
                  }
                  else {
                    for (uint rr = 1; rr <= impureIdxCount; rr++) {
                      r = impureIdx[rr];
                      rghtCentered[rr][1]  = elStar[i][rr] - rghtMean[rr];  
                      rghtCenteredT[1][rr] = rghtCentered[rr][1];
                    }
                    tempResult = matrixMult(rghtCenteredT, qStarPlus, 1, impureIdxCount, impureIdxCount);
                    tempResult2 = matrixMult(tempResult, rghtCentered, 1, impureIdxCount, 1);
                    partialRght += tempResult2[1][1];
                  }
                  free_dmatrix(tempResult,  1, 1, 1, impureIdxCount);
                  free_dmatrix(tempResult2, 1, 1, 1, 1);
                }  
                deltaPartial = ( ((double) leftSize / repMembrSize) * partialLeft) + ( ((double) rghtSize / repMembrSize) * partialRght);
                //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
                //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
                //              RF_nativePrint("\nMahalanobis Internals:                left                right                  sum");
                //              RF_nativePrint("\n                      %20.8f %20.8f %20.8f", ( ((double) leftSize / repMembrSize) * partialLeft), ( ((double) rghtSize / repMembrSize) * partialRght), deltaPartial);
                //            }
                //          }
                delta = 1.0 - (deltaPartial / impureIdxCount);
              }
              else {
                for (uint rr = 1; rr <= impureIdxCount; rr++) {
                  r = impureIdx[rr];
                  sumLeft[r] = sumRght[r] = 0.0;
                }
                for (i = 1; i <= repMembrSize; i++) {
                  if (localSplitIndicator[i] == LEFT) {
                    for (uint rr = 1; rr <= impureIdxCount; rr++) {
                      r = impureIdx[rr];
                      sumLeft[r] += RF_response[treeID][r][ repMembrIndx[i] ] - mean[r];
                    }
                  }
                  else {
                    for (uint rr = 1; rr <= impureIdxCount; rr++) {
                      r = impureIdx[rr];
                      sumRght[r] += RF_response[treeID][r][ repMembrIndx[i] ] - mean[r];
                    }
                  }
                }
                for (uint rr = 1; rr <= impureIdxCount; rr++) {
                  r = impureIdx[rr];
                  partialLeft = pow (sumLeft[r], 2.0) / (leftSize * variance[r]);
                  partialRght = pow (sumRght[r], 2.0) / (rghtSize * variance[r]);
                  //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
                  //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
                  //              RF_nativePrint("\nPartial Delta (Regr): leftSize   rghtSize       mean   variance     sumLFT     sumRGT partialLFT partialRGT        sum");
                  //              RF_nativePrint("\n                    %10d %10d %10.4f %10.4f %10.4f %10.4f %10.4f %10.4f %10.4f",
                  //                       leftSize,
                  //                       rghtSize,
                  //                       mean[r],
                  //                       variance[r],
                  //                       sumLeft[r],
                  //                       sumRght[r],
                  //                       partialLeft,
                  //                       partialRght,
                  //                       partialLeft+partialRght);
                  //            }
                  //          }
                  delta = partialLeft + partialRght;
                }
              }
            }
            else {
              delta = RF_nativeNaN;
            }
            if (!RF_nativeIsNaN(delta)) {
              if(RF_nativeIsNaN(deltaMax)) {
                deltaMax = delta;
                indexMax = j;
              }
              else {
                if ((delta - deltaMax) > EPSILON) {
                  deltaMax = delta;
                  indexMax = j;
                }
              }
            }
            //          if (getTraceFlag(treeID) & SPLT_HGH_TRACE) {
            //            if (getTraceFlag(treeID) & !TURN_OFF_TRACE) {
            //              RF_nativePrint("\nVirtual (non-miss) Membership:  ");
            //              for (uint k = 1; k <= repMembrSize; k++) {
            //                if (localSplitIndicator[ indxx[k] ] == LEFT) {
            //                  RF_nativePrint("\n %10d %10d %10d %12.4f %12.4f --> LEFT ", k, indxx[k], repMembrIndx[indxx[k]], observation[ repMembrIndx[indxx[k]] ], RF_response[treeID][1][  repMembrIndx[indxx[k]]  ]);
            //                }
            //                else {
            //                  RF_nativePrint("\n %10d %10d %10d %12.4f %12.4f --> RGHT ", k, indxx[k], repMembrIndx[indxx[k]], observation[ repMembrIndx[indxx[k]] ], RF_response[treeID][1][  repMembrIndx[indxx[k]]  ]);
            //                }
            //              }
            //            }
            //          }
            if (factorFlag == FALSE) {
              priorMembrIter = currentMembrIter - 1;
            }
          }  
          updateMaximumSplit(treeID,
                             parent,
                             deltaMax,
                             covariate,
                             indexMax,
                             factorFlag,
                             mwcpSizeAbsolute,
                             repMembrSize,
                             & localSplitIndicator,
                             splitVectorPtr,
                             splitInfoMax);
          unstackSplitVector(treeID,
                                parent,
                                splitLength,
                                factorFlag,            
                                splitVectorSize,
                                mwcpSizeAbsolute,
                                deterministicSplitFlag,
                                splitVectorPtr,
                                multImpFlag,
                                indxx);
        }  
      }  
      if (qStarPlus != NULL)  {
        free_dmatrix(qStarPlus, 1, impureIdxCount, 1, impureIdxCount);
      }
      if (RF_qStarPlus == NULL) {
        free_svdcmp(qStar, impureIdxCount, impureIdxCount, u, w, v);
        free_dmatrix(elStarTranspose, 1, impureIdxCount, 1, repMembrSize);
      }
      free_dmatrix(elStar, 1, repMembrSize, 1, impureIdxCount);      
      free_dvector(leftMean, 1, impureIdxCount);
      free_dvector(rghtMean, 1, impureIdxCount);
      free_dmatrix(leftCentered, 1, impureIdxCount, 1, 1);
      free_dmatrix(rghtCentered, 1, impureIdxCount, 1, 1);
      free_dmatrix(leftCenteredT, 1, 1, 1, impureIdxCount);
      free_dmatrix(rghtCenteredT, 1, 1, 1, impureIdxCount);
      free_uivector(impureIdx, 1, RF_ySize);
      free_dvector(sumLeft,     1, RF_ySize);
      free_dvector(sumRght,     1, RF_ySize);
      unstackRandomCovariates(treeID, distributionObj);
      unstackSplitPreliminary(repMembrSize,
                              localSplitIndicator,
                              splitVector);
    }  
    free_cvector(impurity,   1, RF_ySize);
    free_dvector(mean,       1, RF_ySize);
    free_dvector(variance,   1, RF_ySize);
  }  
  unstackPreSplit(preliminaryResult,
                  parent,
                  multImpFlag,
                  TRUE);  
  result = summarizeSplitResult(splitInfoMax);
  //  if (getTraceFlag(treeID) & SPLT_LOW_TRACE) {
  //    RF_nativePrint("\nmahalanobis(%10d) result:  %10d", treeID, result);
  //    RF_nativePrint("\nmahalanobis(%10d) EXIT ...\n", treeID);
  //  }
  return result;
}
