#include "Heap.h"

#include <cstdlib>
#include <sys/mman.h>

using namespace std;

Heap::Heap()
{
    heapSize = 1024*1024; // initial, for testing
    heapBase = mmap(NULL, heapSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

QuaValue Heap::allocateNew(uint16_t type, uint32_t size) {
    //TODO
    return QuaValue(0,0,0);
}
