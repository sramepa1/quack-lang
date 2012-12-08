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


#define F_NORMAL 0x0000
#define F_STATIC 0x0001



#pragma pack(1)

struct ObjRecord
{
    uint16_t type;
    uint16_t flags;
    uint32_t instanceCount;
    QuaObject* instance;
};

#pragma pack()



class Heap
{
public:
    Heap(size_t size);
    Heap(size_t statSize, size_t dynSize);

    void* getBase();
    void* getEnd();

    const ObjRecord& dereference(QuaValue ref);

    QuaValue allocateNew(uint16_t type, uint32_t fieldCount);
    QuaValue allocateNewStatic(uint16_t type, uint32_t fieldCount);

private:
    void* heapBase;
    void* tableBase;
    
    size_t totalSize; 
    size_t heapSize;
    size_t tableSize;
    
    uint32_t freeTableIndex;
    
    void* freeHeapPtr;
    void* freeTablePtr;
    
    bool firstGeneration;
    
    
    void* statHeapBase;
    uint32_t statHeapTotalSize;
    uint32_t statHeapAvailableSize;
    void* statFreePtr;

    void* objectTableBase;
    uint32_t objectTableSize;
    
    //uint32_t freeIndex;
    
    void testFreeSpaceForNew(uint32_t fieldCount);
    void findFreeTablePointer() {
        if(firstGeneration) {
            freeTablePtr = (void*) ((ObjRecord*) freeTablePtr - 1);
            freeTableIndex++;
        }

        // TODO search for free index
    }
    
    void addRecord(ObjRecord* record){
        findFreeTablePointer();
        memcpy(freeTablePtr, record, sizeof(ObjRecord));
    }
    
    const ObjRecord* getRecord(uint32_t index) {
        return ((const ObjRecord*) tableBase - index - 1);
    }
    
    /*
    std::vector<ObjRecord*> objectTable;
    
    std::vector<int> objectTable;
    std::map<int, ObjRecord*> staticCache;

     */
};




#endif // HEAP_H
