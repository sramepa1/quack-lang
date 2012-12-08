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


Heap::Heap(size_t volatileSize, size_t permanentSize) : volatileHeap(volatileSize), permanentHeap(permanentSize) {}

const ObjRecord& Heap::dereference(QuaValue ref) {
    if(ref.flags & FLAG_REF_PERMANENT) {
        return permanentHeap.dereference(ref);
    }
    
    return volatileHeap.dereference(ref);
}

QuaValue Heap::allocateNew(uint16_t type, uint32_t fieldCount) {
    if(getClassFromType(type)->isStatic()) {
        return permanentHeap.allocateNew(type, fieldCount);
    }
    
    return volatileHeap.allocateNew(type, fieldCount);
}

/////////////////////////

void allocateHeap(size_t* size, void** heapBase, void** tableBase) {
    
    uint32_t pageCnt = *size / getpagesize() + (*size % getpagesize() == 0 ? 0 : 1);
    *size = pageCnt * getpagesize();
 
    *heapBase = mmap(NULL, *size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    checkMmap(*heapBase, "Could not allocate the heap.");
    
    *tableBase = (void*) ((char*) *heapBase + *size - 1);
    
#ifdef DEBUG
	cout << "Heap of rounded size " << *size << " created at " << *heapBase << " with object table at " << *tableBase << endl;
#endif
    
    //mprotect( (void*) ((char*) *heapBase + *heapAvailableSize + 1), getpagesize(), PROT_NONE);
}

AbstractHeap::AbstractHeap(size_t size) {
    
    allocatedSize = size;
    allocateHeap(&allocatedSize, &heapBase, &tableBase);

    freeHeapPtr = heapBase;
    freeTablePtr = tableBase;
    freeTableIndex = (uint32_t) -1;
    
    heapSize = 0;
    tableSize = 0;
        
}


const ObjRecord& AbstractHeap::dereference(QuaValue ref) {

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


QuaValue AbstractHeap::allocateNew(uint16_t type, uint32_t fieldCount) {
  
    size_t size = fieldCount * sizeof(QuaValue);
    
    if(heapSize + tableSize + size + sizeof(ObjRecord) > allocatedSize) {
        collectGarbage();
    }
    
    ObjRecord record;
    
    // create object
    if(fieldCount == 0) {
        record.field = NULL;
    } else {
        record.field = (QuaObject*) memset(freeHeapPtr, 0, size);  // will be null references to _Null (guaranteed zero)
        freeHeapPtr = (void*) ((char*) freeHeapPtr + size);
        heapSize += size;
    }
    
    record.flags = 0;
    record.fieldCount = fieldCount;
    record.type = type;
    
    // create table entry
    addRecord(&record);

    return QuaValue(freeTableIndex, type, TAG_REF, qValueFlags);
}


void* AbstractHeap::getBase() {
    return heapBase;
}

void* AbstractHeap::getEnd() {
    return (void*) ((char*) heapBase + allocatedSize);
}


////////////////////////////// Baker

BakerHeap::BakerHeap(size_t size) : AbstractHeap::AbstractHeap(size) {
    qValueFlags = 0;
    
    firstGeneration = true;
    
    // null
    ObjRecord null;
    addRecord(&null);
}

void BakerHeap::prepareFreeTableEtry() {
    if(firstGeneration) {
        freeTablePtr = (void*) ((ObjRecord*) freeTablePtr - 1);
        freeTableIndex++;
    }

    // TODO search for free index
}

void BakerHeap::collectGarbage() {
    // TODO implement
}

////////////////////////////// Permanent

PermanentHeap::PermanentHeap(size_t size) : AbstractHeap::AbstractHeap(size) {
    qValueFlags = FLAG_REF_PERMANENT;
    
    // null
    ObjRecord null;
    addRecord(&null);
}

void PermanentHeap::prepareFreeTableEtry() {
    freeTablePtr = (void*) ((ObjRecord*) freeTablePtr - 1);
    freeTableIndex++;
}

void PermanentHeap::collectGarbage() {
    throw runtime_error("Permanent heap run out of memory.");
}

