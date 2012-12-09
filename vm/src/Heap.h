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


#define COLLECTION_MASK            0x0001

#define FLAG_COLLECTION_ODD        0x0000
#define FLAG_COLLECTION_EVEN       0x0001


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
    uint16_t currentRecordFlags;
    
    virtual void prepareFreeTableEtry() = 0;
    virtual void collectGarbage() = 0;
    
    void addRecord(ObjRecord* record) {
        prepareFreeTableEtry();
        memcpy(freeTablePtr, record, sizeof(ObjRecord));
    }

    ObjRecord* getRecord(uint32_t index) {
        return ((ObjRecord*) tableBase - index - 1);
    }
    
};


class BakerHeap : public AbstractHeap {
public:
    virtual ~BakerHeap() {}
    BakerHeap(size_t size);

protected:
    
    bool firstGeneration;
    
    void* _heapBase;
    void* _tableBase;
    
    size_t _allocatedSize; 
    size_t _heapSize;
    void* _freeHeapPtr;
  
    void* topTablePtr;
    uint32_t topTableIndex;
    uint32_t maxUsedTableIndex;

    virtual void prepareFreeTableEtry();
    virtual void collectGarbage();
 
    ObjRecord* _getRecord(uint32_t index) {
        return ((ObjRecord*) _tableBase - index - 1);
    }
    
    void saveReachableObject(uint32_t index);
    void trySaveFields(const ObjRecord* source);
    void trySaveObject(QuaValue qval);
};


class PermanentHeap : public AbstractHeap {
public:
    virtual ~PermanentHeap() {}
    PermanentHeap(size_t size);

protected:
    
    virtual void prepareFreeTableEtry();
    virtual void collectGarbage();
 
    friend class BakerHeap;
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

