
// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "leafLink.h"
#include "termOps.h"
#include "nrutil.h"
// #include "error.h"
LeafLinkedObj *makeLeafLinkedObj(void) {
  LeafLinkedObj *obj = (LeafLinkedObj*) gblock((size_t) sizeof(LeafLinkedObj));
  obj -> fwdLink = NULL;
  obj -> bakLink = NULL;
  obj -> nodePtr = NULL;
  obj -> termPtr = NULL;
  obj -> termPtrAux = NULL;
  obj -> nodeID = 0;
  obj -> ibgMembrCount = 0;
  obj -> allMembrCount = 0;
  obj -> oobMembrCount = 0;
  return obj;
}
LeafLinkedObjSimple *makeLeafLinkedObjSimple(void) {
  LeafLinkedObjSimple *obj = (LeafLinkedObjSimple*) gblock((size_t) sizeof(LeafLinkedObjSimple));
  obj -> fwdLink = NULL;
  obj -> bakLink = NULL;
  obj -> nodePtr = NULL;
  return obj;
}
LeafLinkedObj *makeAndSpliceLeafLinkedObj(LeafLinkedObj *tail,
                                          Node *nodePtr,
                                          uint ibgCount,
                                          uint allCount) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    RF_nativePrint("\nmakeAndSpliceLeafLinkedObj() ENTRY ...\n");
  //  }
  LeafLinkedObj *obj = makeLeafLinkedObj();
  tail -> fwdLink = obj;
  obj -> bakLink = tail;
  obj -> nodePtr = nodePtr;
  obj -> termPtr = makeTerminal();
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    RF_nativePrint("\nLeaf Linked Object spliced for terminal nodeID:  %10d", nodePtr -> nodeID);
  //  }
  (obj -> termPtr) -> mate = obj -> nodePtr;
  (obj -> nodePtr) -> mate = obj -> termPtr;
  (obj -> termPtr) -> nodeID = obj -> nodeID = nodePtr -> nodeID;
  obj -> ibgMembrCount = ibgCount;
  obj -> allMembrCount = allCount;
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    RF_nativePrint("\nmakeAndSpliceLeafLinkedObj() EXIT ...\n");
  //  }
  return obj;
}
LeafLinkedObjSimple *makeAndSpliceLeafLinkedObjSimple(LeafLinkedObjSimple *tail,
                                                      Node *nodePtr) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    RF_nativePrint("\nmakeAndSpliceLeafLinkedObjSimple() ENTRY ...\n");
  //  }
  LeafLinkedObjSimple *obj = makeLeafLinkedObjSimple();
  tail -> fwdLink = obj;
  obj -> bakLink = tail;
  obj -> nodePtr = nodePtr;
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    RF_nativePrint("\nLeaf Linked Object Simple spliced for node nodeID:  %10d", nodePtr -> nodeID);
  //  }
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    RF_nativePrint("\nmakeAndSpliceLeafLinkedObjSimple() EXIT ...\n");
  //  }
  return obj;
}
void freeLeafLinkedObj(LeafLinkedObj *obj) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {
  //    RF_nativePrint("\nfreeLeafLinkedObj() ENTRY ...\n");
  //  }
  //  }
  if (obj -> termPtr != NULL) {
    //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
    //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {
    //      RF_nativePrint("\nFreeing linked object and terminal node pointer at:  %10x, %10x", obj, obj -> termPtr);
    //    }
    //  }
    freeTerminal(obj -> termPtr);
    obj -> termPtr = NULL;
  }
  free_gblock(obj, (size_t) sizeof(LeafLinkedObj));
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {  
  //    RF_nativePrint("\nfreeLeafLinkedObj() EXIT ...\n");
  //  }
  //  }
}
void freeLeafLinkedObjSimple(LeafLinkedObjSimple *obj) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //  if (getTraceFlag(0) & !TURN_OFF_TRACE) {
  //    RF_nativePrint("\nfreeLeafLinkedObjSimple() ENTRY ...\n");
  //  }
  //  }
  free_gblock(obj, (size_t) sizeof(LeafLinkedObjSimple));
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //  if (getTraceFlag(0) & !TURN_OFF_TRACE) {  
  //    RF_nativePrint("\nfreeLeafLinkedObjSimple() EXIT ...\n");
  //  }
  //  }
}
void freeLeafLinkedObjList(LeafLinkedObj *obj) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {    
  //    RF_nativePrint("\nfreeLeafLinkedObjList() ENTRY ...\n");
  //  }
  //  }
  if (obj -> fwdLink != NULL) {
    freeLeafLinkedObjList(obj -> fwdLink);
  }
  freeLeafLinkedObj(obj);
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & !TURN_OFF_TRACE) {  
  //    RF_nativePrint("\nfreeLeafLinkedObjList() EXIT ...\n");
  //  }
  //  }
}
void freeLeafLinkedObjListRev(LeafLinkedObj *obj) {
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & TURN_OFF_TRACE) {    
  //    RF_nativePrint("\nfreeLeafLinkedObjListRev() ENTRY ...\n");
  //  }
  //  }
  if (obj -> bakLink != NULL) {
    freeLeafLinkedObjListRev(obj -> bakLink);
  }
  freeLeafLinkedObj(obj);
  //  if (getTraceFlag(0) & NODE_DEF_TRACE) {
  //    if (getTraceFlag(0) & TURN_OFF_TRACE) {  
  //    RF_nativePrint("\nfreeLeafLinkedObjListRev() EXIT ...\n");
  //  }
  //  }
}
