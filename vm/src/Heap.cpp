#include "Heap.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include "Exceptions.h"
#include "globals.h"
#include "helpers.h"

using namespace std;


void allocateHeap(size_t* size, void** heapBase, void** tableBase) {
    
    uint32_t pageCnt = *size / getpagesize() + (*size % getpagesize() == 0 ? 0 : 1);
    *size = pageCnt * getpagesize();
   // uint32_t heapAvailableSize = (pageCnt - 1) * getpagesize()
 
    *heapBase = mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    checkMmap(*heapBase, "Could not allocate the heap.");
    
    *tableBase = (void*) ((char*) *heapBase + *size - 1);
    
#ifdef DEBUG
	cout << "Heap of rounded size " << *size << " created at " << *heapBase << " with object table at " << *tableBase << endl;
#endif
    
    //mprotect( (void*) ((char*) *heapBase + *heapAvailableSize + 1), getpagesize(), PROT_NONE);
}




Heap::Heap(size_t size) {
    
    totalSize = size;
    allocateHeap(&totalSize, &heapBase, &tableBase);

    freeHeapPtr = heapBase;
    freeTablePtr = tableBase;
    freeTableIndex = (uint32_t) -1;
    
    firstGeneration = true;
    
    // null
    ObjRecord null;
    addRecord(&null);
    
    /*
    heapSize = size;
    heapBase = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    checkMmap(heapBase, "Could not allocate the heap.");
    
    //TODO je to korektní? Tu stránku jsme si nevyžádali, může twm být kdoví co
    
    // guard page
    mprotect( (void*)((char*)heapBase + size - getpagesize()), getpagesize(), PROT_NONE);
    freePtr = heapBase;
    objTable.push_back(ObjRecord()); // "null"
     */
}

/*
Heap::Heap(size_t statSize, size_t dynSize) {
    
    statHeapSize = statSize;
        
    allocateHeap(&heapTotalSize, &heapAvailableSize, &statHeapBase);
    
    statFreePtr = heapBase;
   
    // TODO why?
    //objTable.push_back(ObjRecord()); // "null"
}

*/


const ObjRecord& Heap::dereference(QuaValue ref) {

    switch(ref.tag) {   // TODO: Autobox code (needs runtime class layouts or constructors)
        case TAG_BOOL: throw runtime_error("Autoboxing Bool is not yet supported.");
        case TAG_INT: throw runtime_error("Autoboxing Integer is not yet supported.");
        case TAG_FLOAT: throw runtime_error("Autoboxing Float is not yet supported.");
        case TAG_REF: break;
        default: errorUnknownTag(ref.tag);
    }

    if(ref.type == typeCache.typeNull || ref.value == 0) {
        throw NullReferenceException(string("Dereferenced a null value of class ") + getClassFromValue(ref)->getName());
    }

    return *getRecord(ref.value);
}





/*

QuaValue Heap::allocateNewStatic(uint16_t type, uint32_t fieldCount) {
    
    size_t size = fieldCount * sizeof(QuaValue);
    
    if(statFreePtr + size >= statHeapAvailableSize) {
        throw runtime_error("Out of static heap memory.");
    }
    
    uint32_t id = statObjTable.size();
    ObjRecord record;
    
    if(fieldCount == 0) {
        record.instance = NULL;
    } else {
        record.instance = (QuaObject*) memset(statFreePtr, 0, size);  // will be null references to _Null (guaranteed zero)
        freePtr = (void*) ((char*)freePtr + size);
    }
    
    record.flags = 0;
    record.instanceCount = fieldCount;
    record.type = type;
    
    objectTable.push_back(record);

    return QuaValue(id, type, TAG_REF);
}

*/


void Heap::testFreeSpaceForNew(uint32_t fieldCount) {
     //TODO
}




QuaValue Heap::allocateNew(uint16_t type, uint32_t fieldCount) {
  
    size_t size = fieldCount * sizeof(QuaValue);
    
    testFreeSpaceForNew(fieldCount);
    
    ObjRecord record;
    
    // create object
    if(fieldCount == 0) {
        record.instance = NULL;
    } else {
        record.instance = (QuaObject*) memset(freeHeapPtr, 0, size);  // will be null references to _Null (guaranteed zero)
        freeHeapPtr = (void*) ((char*) freeHeapPtr + size);
    }
    
    record.flags = 0;
    record.instanceCount = fieldCount;
    record.type = type;
    
    // create table entry
    addRecord(&record);

    return QuaValue(freeTableIndex, type, TAG_REF);
}


void* Heap::getBase() {
    return heapBase;
}

void* Heap::getEnd() {
    return (void*)((char*)heapBase + heapSize);
}
