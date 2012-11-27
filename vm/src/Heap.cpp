#include "Heap.h"

#include <cstdlib>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

#include "globals.h"
#include "helpers.h"

using namespace std;

Heap::Heap(size_t size) : temporaryObjTableFreeIndex(1)
{
    heapSize = size;
    heapBase = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    checkMmap(heapBase, "Could not allocate the heap.");
    // guard page
    mprotect( (void*)((char*)heapBase + size - getpagesize()), getpagesize(), PROT_NONE);
    freePtr = heapBase;
    objTable.push_back(ObjRecord()); // "null"
}

QuaValue Heap::allocateNew(uint16_t type, uint32_t size) {

    // TODO replace this prototype with an actual allocator
    uint32_t id = temporaryObjTableFreeIndex++;

    ObjRecord rec;
    if(size == 0) {
        rec.instance = NULL;
    } else {
        size_t bytes = size*sizeof(QuaValue);
        rec.instance = (QuaObject*)memset(freePtr, 0, bytes);  // will be null references to _Null
        freePtr = (void*)((char*)freePtr + bytes);
    }
    rec.flags = 0;
    rec.size = size;
    rec.type = type;
    objTable.push_back(rec);

    return QuaValue(id, type, 0);
}


void* Heap::getBase() {
    return heapBase;
}

void* Heap::getEnd() {
    return (void*)((char*)heapBase + heapSize);
}
