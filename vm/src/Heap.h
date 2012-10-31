#ifndef HEAP_H
#define HEAP_H

extern "C" {
    #include <stdint.h>
}

#include <vector>

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
    Heap();


private:
    void* heapBase;
    uint32_t heapSize;

    void* freePtr;

    std::vector<ObjRecord> objTable;

    QuaValue allocateNew(uint16_t type, uint32_t size);
};




#endif // HEAP_H
