
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "sortedLink.h"
#include "nrutil.h"
// #include "error.h"
SortedLinkedObj *makeSortedLinkedObj(void) {
  SortedLinkedObj *obj = (SortedLinkedObj*) gblock((size_t) sizeof(SortedLinkedObj));
  obj -> fwdLink = NULL;
  obj -> bakLink = NULL;
  obj -> rank = 0;
  obj -> indx = 0;
  return obj;
}
void makeAndSpliceSortedLinkedObj(uint treeID,
                                  SortedLinkedObj **headPtr,
                                  SortedLinkedObj **tailPtr,
                                  uint *listLength,
                                  uint rank, uint indx) {
  char flag;
  uint lowIndx, highIndx, halfIndx;
  SortedLinkedObj *objIterator;
  uint i;
  //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
  //    RF_nativePrint("\nmakeAndSpliceSortedLinkedObj() ENTRY ...\n");
  //  }
  SortedLinkedObj *head = headPtr[treeID];
  SortedLinkedObj *tail = tailPtr[treeID];
  SortedLinkedObj *obj = makeSortedLinkedObj();
  obj -> rank = rank;
  obj -> indx = indx;
  obj -> fwdLink = obj -> bakLink = NULL;
  //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
  //    RF_nativePrint("\nIncoming Sorted Linked Obj:  (head, tail, length, rank, index) = (%10x %10x %10d %10d %10d)", head, tail, *listLength, rank, indx);
  //  }
  flag = TRUE;
  if (*listLength == 0) {
    head = tail = obj;
    flag = FALSE;
    //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
    //    RF_nativePrint("\nFirst Sorted Linked Obj initialized:  %10x", head);
    //  }
  }
  else if (rank >= tail -> rank) {
    tail -> fwdLink = obj;
    obj -> bakLink = tail;
    tail = obj;
    flag = FALSE;
    //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
    //    RF_nativePrint("\nHighest Sorted Linked Obj initialized:  %10x", tail);
    //  }
  }
  else if (rank <= head -> rank) {
    head -> bakLink = obj;
    obj -> fwdLink = head;
    head = obj;
    flag = FALSE;
    //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
    //    RF_nativePrint("\nLowest Sorted Linked Obj initialized:  %10x", head);
    //  }
  }
  else {
    lowIndx  = 1;
    highIndx = *listLength;
    while (flag) {
      halfIndx = (uint) ((double) (highIndx + lowIndx) / 2.0); 
      objIterator = head;
      for (i = lowIndx; i < halfIndx; i++) {
        objIterator = objIterator -> fwdLink;
      }
      //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
      //    RF_nativePrint("\nSorted Linked Obj iterating forward to half: (low, half, high) = (%10d, %10d, %10d)", lowIndx, halfIndx, highIndx);
      //  }
      if (rank == head -> rank) {
        obj -> fwdLink = head;
        obj -> bakLink = (head -> bakLink);
        (head -> bakLink) -> fwdLink = obj;
        head -> bakLink = obj;
        flag = FALSE;
        //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
        //    RF_nativePrint("\nSorted Linked Obj insert before (internal) head:  %10x with ranks (%10d, %10d, %10d)", obj, (obj -> bakLink) -> rank, obj -> rank, (obj -> fwdLink) -> rank);
        //  }
      }
      else if (rank == tail -> rank) {
        obj -> fwdLink = tail;
        obj -> bakLink = (tail -> bakLink);
        (tail -> bakLink) -> fwdLink = obj;
        tail -> bakLink = obj;
        flag = FALSE;
        //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
        //    RF_nativePrint("\nSorted Linked Obj insert before (internal) tail:  %10x with ranks (%10d, %10d, %10d)", obj, (obj -> bakLink) -> rank, obj -> rank, (obj -> fwdLink) -> rank);
        //  }
      }
      else if (rank == objIterator -> rank) {
        obj -> fwdLink = objIterator;
        obj -> bakLink = (objIterator -> bakLink);
        (objIterator -> bakLink) -> fwdLink = obj;
        objIterator -> bakLink = obj;
        flag = FALSE;
        //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
        //    RF_nativePrint("\nSorted Linked Obj insert before half:  %10x with ranks (%10d, %10d, %10d)", obj, (obj -> bakLink) -> rank, obj -> rank, (obj -> fwdLink) -> rank);
        //  }
      }
      else if (halfIndx == lowIndx) {
        obj -> fwdLink = tail;
        obj -> bakLink = (tail -> bakLink);
        (tail -> bakLink) -> fwdLink = obj;
        tail -> bakLink = obj;
        flag = FALSE;
        //  if (getTraceFlag(0) & FORK_DEF_TRACE) {
        //    RF_nativePrint("\nSorted Linked Obj insert worst case at (lowIndx, highIndx):  %10x with ranks (%10d, %10d, %10d)", obj, (obj -> bakLink) -> rank, obj -> rank, (obj -> fwdLink) -> rank);
        //  }
      }
      else if (rank < objIterator -> rank) {
        tail = objIterator;
        highIndx = halfIndx;
      }
      else {
        head = objIterator;
        lowIndx = halfIndx;
      }
    }
  }
  (*listLength) ++;
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    RF_nativePrint("\nmakeAndSpliceSortedLinkedObj() EXIT ...\n");
  //  }
}
void freeSortedLinkedObjList(SortedLinkedObj *obj) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {    
  //    RF_nativePrint("\nfreeSortedLinkedObjList() ENTRY ...\n");
  //  }
  //  }
  if (obj -> fwdLink != NULL) {
    freeSortedLinkedObjList(obj -> fwdLink);
  }
  freeSortedLinkedObj(obj);
  obj = NULL;
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {  
  //    RF_nativePrint("\nfreeSortedLinkedObjList() EXIT ...\n");
  //  }
  //  }
}
void freeSortedLinkedObj(SortedLinkedObj *obj) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {
  //    RF_nativePrint("\nfreeSortedLinkedObj() ENTRY ...\n");
  //  }
  //  }
  free_gblock(obj, (size_t) sizeof(SortedLinkedObj));
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {  
  //    RF_nativePrint("\nfreeSortedLinkedObj() EXIT ...\n");
  //  }
  //  }
}
