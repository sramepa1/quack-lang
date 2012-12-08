#ifndef HEAP_H
#define HEAP_H

extern "C" {
    #include <stdint.h>
}

#include <cstring>
#include <cstdlib>
#include <queue>
#include <vector>
#include <stdexcept>

#include "QuaObject.h"


#pragma pack(1)

struct ObjRecord
{
    uint16_t type;
    uint16_t flags;
    uint32_t fieldCount;
    QuaObject* field;
};

#pragma pack()

class AbstractHeap {
public:
    virtual ~AbstractHeap() {}
    AbstractHeap(size_t size);

    void* getBase();
    void* getEnd();

    const ObjRecord& dereference(QuaValue ref);
    QuaValue allocateNew(uint16_t type, uint32_t fieldCount);

protected:
    void* heapBase;
    void* tableBase;
    
    size_t allocatedSize; 
    size_t heapSize;
    size_t tableSize;
    
    uint32_t freeTableIndex;
    
    void* freeHeapPtr;
    void* freeTablePtr;
    
    uint8_t qValueFlags;
    
    virtual void prepareFreeTableEtry() = 0;
    virtual void collectGarbage() = 0;
    
    virtual void addRecord(ObjRecord* record) {
        prepareFreeTableEtry();
        memcpy(freeTablePtr, record, sizeof(ObjRecord));
        tableSize += sizeof(ObjRecord);
    }

    virtual const ObjRecord* getRecord(uint32_t index) {
        return ((const ObjRecord*) tableBase - index - 1);
    }
    
};


class BakerHeap : public AbstractHeap {
public:
    virtual ~BakerHeap() {}
    BakerHeap(size_t size);

protected:
    
    bool firstGeneration;
    
    virtual void prepareFreeTableEtry();
    virtual void collectGarbage();
 
};


class PermanentHeap : public AbstractHeap {
public:
    virtual ~PermanentHeap() {}
    PermanentHeap(size_t size);

protected:
    
    virtual void prepareFreeTableEtry();
    virtual void collectGarbage();
 
};


class Heap {
public:
    Heap(size_t volatileSize, size_t permanentSize);

    void* getVolatileBase() {return volatileHeap.getBase();}
    void* getVolatileEnd() {return volatileHeap.getEnd();}
    void* getPermanentBase() {return permanentHeap.getBase();}
    void* getPermanentEnd() {return permanentHeap.getEnd();}

    const ObjRecord& dereference(QuaValue ref);
    QuaValue allocateNew(uint16_t type, uint32_t fieldCount);
    
private:
    BakerHeap volatileHeap;
    PermanentHeap permanentHeap;          
};


#endif // HEAP_H

