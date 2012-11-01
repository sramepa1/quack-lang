#include "Heap.h"

#include <cstdlib>
#include <sys/mman.h>
#include <unistd.h>

#include "globals.h"

using namespace std;

Heap::Heap(size_t size)
{
    heapSize = size;
    heapBase = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    checkMmap(heapBase, "Could not allocate the heap.");
    // guard page
    mprotect( (void*)((char*)heapBase + size - getpagesize()), getpagesize(), PROT_NONE);
}

QuaValue Heap::allocateNew(uint16_t type, uint32_t size) {
    //TODO
    return QuaValue(0,0,0);
}


void* Heap::getBase() {
    return heapBase;
}

void* Heap::getEnd() {
    return (void*)((char*)heapBase + heapSize);
}
