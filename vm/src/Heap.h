#ifndef HEAP_H
#define HEAP_H

extern "C" {
    #include <stdint.h>
}

#include <cstdlib>
#include <vector>
#include <stdexcept>

#include "QuaObject.h"


struct ObjRecord
{
    uint16_t type;
    uint16_t flags;
    uint32_t size;
    QuaObject* instance;
};


class Heap
{
public:
    Heap(size_t size);

    void* getBase();
    void* getEnd();

    const ObjRecord& dereference(QuaValue ref);

    QuaValue allocateNew(uint16_t type, uint32_t size);

private:
    void* heapBase;
    uint32_t heapSize;

    void* freePtr;

    uint32_t temporaryObjTableFreeIndex;

    std::vector<ObjRecord> objTable;

};




#endif // HEAP_H
