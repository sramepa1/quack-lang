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

static PermanentHeap* permHeap;

template< typename T>
void mySwap(T* a, T* b) {
    T c = *a;
    *a = *b;
    *b = c;
}

Heap::Heap(size_t volatileSize, size_t permanentSize) : volatileHeap(volatileSize), permanentHeap(permanentSize) {
    permHeap = &permanentHeap;
}

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
	cout << "A heap of rounded size " << *size << "B allocated at " << *heapBase << " with object table at " << *tableBase << endl;
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
        cout << "error at " << ref.value << endl;
        
        throw NullReferenceException(string("Dereferenced a null value of class ") + getClassFromValue(ref)->getName());
    }

    return *getRecord(ref.value);
}


QuaValue AbstractHeap::allocateNew(uint16_t type, uint32_t fieldCount) {

    size_t size = fieldCount * sizeof(QuaValue);
    
    if(heapSize + tableSize + size + sizeof(ObjRecord) > allocatedSize) {
        collectGarbage();
        
        // control test
        if(heapSize + tableSize + size + sizeof(ObjRecord) > allocatedSize) {
            throw runtime_error("Volatile heap run out of memory and can not be freed.");
        }
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

BakerHeap::BakerHeap(size_t size) : AbstractHeap::AbstractHeap(size / 2) {
    qValueFlags = FLAG_REF_VOLATILE;
    currentRecordFlags = FLAG_COLLECTION_ODD;
    
    firstGeneration = true;
    
    // epty second part of the heap
    _heapBase = NULL;
    _tableBase = NULL;
    
    // null
    ObjRecord null;
    addRecord(&null);
}

void BakerHeap::prepareFreeTableEtry() {
    if(firstGeneration) {
        freeTablePtr = (void*) ((ObjRecord*) freeTablePtr - 1);
        topTablePtr = freeTablePtr; // remember start(end) of the table
        topTableIndex = ++freeTableIndex;
        
        tableSize += sizeof(ObjRecord);
        
        return;
    }
    
    // search for free index and reuse
    for(uint32_t i = maxUsedTableIndex; i <= topTableIndex; ++i) {
        if(getRecord(i)->flags != currentRecordFlags) {
            freeTableIndex = i;
            freeTablePtr = (void*) getRecord(i);
            return;
        }
    }
    
    freeTablePtr = (void*) ((ObjRecord*) topTablePtr - 1);
    topTablePtr = freeTablePtr;
    freeTableIndex = ++topTableIndex;
    
    tableSize += sizeof(ObjRecord);
}



void BakerHeap::collectGarbage() {
    
#ifdef DEBUG
    cout << "Insufficient heap space, initiating garbage collection" << endl;
#endif
    
    firstGeneration = false;
    currentRecordFlags = (currentRecordFlags + 1) & COLLECTION_MASK;
    maxUsedTableIndex = 0;
    
    // create the second heap part
    if(_heapBase == NULL) {
        _allocatedSize = allocatedSize;
        allocateHeap(&_allocatedSize, &_heapBase, &_tableBase);
    } else {
#ifdef DEBUG
	cout << "Using heap number " << currentRecordFlags << " of size " << allocatedSize << " at " << _heapBase << " with object table at " << _tableBase << endl;
#endif
    }

    _freeHeapPtr = _heapBase;
    
    _heapSize = 0;
 
    // copy class table
    void* src = topTablePtr;
    void* dest = ((ObjRecord*) _tableBase - topTableIndex - 1);
    size_t size = sizeof(ObjRecord) * (topTableIndex + 1);
    topTablePtr = dest;
    
#ifdef DEBUG
    cout << "Copying object table of size " << size << " from " << src << " to " << dest << endl;
    cout << "Table base position check is " << (void*) ((char*) dest + size) << endl;
#endif
    
    memcpy(src, dest, size);
    
    // manualy save null
    ObjRecord* _null = _getRecord(0);
    _null->flags = currentRecordFlags;
    
#ifdef DEBUG
    cout << "Searching permanent heap root set" << endl;
#endif
    
    // copy objects from the root set  
    for(uint32_t i = 1; i <= permHeap->freeTableIndex; ++i) {
        const ObjRecord* record = permHeap->getRecord(i);
        trySaveFields(record);
    }
    
#ifdef DEBUG
    cout << "Searching stack root set" << endl;
#endif
    
    for(QuaValue* qvptr = VMSP; (void*) qvptr < (void*) valStackHigh ; ++qvptr) {
        trySaveObject(*qvptr);
    }
    
#ifdef DEBUG
    cout << "Searching registry root set" << endl;
#endif
 
    /* BUG caused by interpreter->getMaxRegCount()
    for(uint16_t i = 0; i < interpreter->getMaxRegCount(); ++i) {
        QuaValue qval = interpreter->readRegister(i);
        trySaveObject(qval);
    }
    */
    
    for(uint16_t i = 0; i < 0xFFFF; ++i) {
        QuaValue qval = interpreter->readRegister(i);
        trySaveObject(qval);
    }
    
    
    // free unused end of object table
    // TODO buggy and not needed
    /*
    _tableSize = maxUsedTableIndex * sizeof(ObjRecord);
    topTablePtr = (void*) ((ObjRecord*) topTablePtr + (topTableIndex - maxUsedTableIndex));
    topTableIndex = maxUsedTableIndex;
    
#ifdef DEBUG
    cout << "Freeing " << (topTableIndex - maxUsedTableIndex) << " unused recods from object table" << endl;
#endif
    */
     
    maxUsedTableIndex = 1; // 1 is for null
    
    // swaping space pointers
    void* swap;
    swap = heapBase;
    heapBase = _heapBase;
    _heapBase = swap;
    
    swap = tableBase;
    tableBase = _tableBase;
    _tableBase = swap;
    
    mySwap(&heapSize, &_heapSize);
    mySwap(&freeHeapPtr, &_freeHeapPtr);
    
#ifdef DEBUG
    cout << "Garbage collection complete, heap is running on part " << currentRecordFlags << endl;
#endif
    
}

void BakerHeap::saveReachableObject(uint32_t index) {
    
#ifdef DEBUG
    cout << "GC saving object of index " << index << endl;
#endif
    
    ObjRecord* record = getRecord(index);
    ObjRecord* _record = _getRecord(index);
    
    size_t size = record->fieldCount * sizeof(QuaValue);
    
    // copy data and update record
    _record->field = (QuaObject*) memcpy(_freeHeapPtr, record->field->fields, size);
    _record->flags = currentRecordFlags;
        
    _freeHeapPtr = (void*) ((char*) _freeHeapPtr + size);
    _heapSize += size;
    
    maxUsedTableIndex = maxUsedTableIndex < index ? index : maxUsedTableIndex;
    
    //recursion
    trySaveFields(record);

}

void BakerHeap::trySaveFields(const ObjRecord* source) {
    for(uint32_t i = 0; i < source->fieldCount; i++) {
        
        QuaValue qval = source->field->fields[i];
        
        trySaveObject(qval);
        
    }
}

void BakerHeap::trySaveObject(QuaValue qval) {
    if(qval.tag == TAG_REF && qval.flags == FLAG_REF_VOLATILE) {
        ObjRecord* target = _getRecord(qval.value);

        if(target->flags != currentRecordFlags) {
            saveReachableObject(qval.value);
        }  
    }
}


////////////////////////////// Permanent

PermanentHeap::PermanentHeap(size_t size) : AbstractHeap::AbstractHeap(size) {
    qValueFlags = FLAG_REF_PERMANENT;
    currentRecordFlags = FLAG_COLLECTION_ODD;
    
    // null
    ObjRecord null;
    addRecord(&null);
}

void PermanentHeap::prepareFreeTableEtry() {
    freeTablePtr = (void*) ((ObjRecord*) freeTablePtr - 1);
    freeTableIndex++;
    tableSize += sizeof(ObjRecord);
}

void PermanentHeap::collectGarbage() {
    throw runtime_error("Permanent heap run out of memory.");
}

