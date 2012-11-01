#ifndef GLOBALS_H
#define GLOBALS_H

/* Assembly-access-friendly VM state globals */

// but is it even needed?

#include <map>
#include <string>
#include <csetjmp>

#include "QuaValue.h"
#include "QuaClass.h"
#include "QuaFrame.h"
#include "Heap.h"
#include "Loader.h"


// These symbols might be needed from assembly sources -> disable mangling

extern "C" {

    #include <stdint.h>

    extern void* valStackLow;
    extern void* valStackHigh;
    extern QuaValue* SP;
    extern QuaValue* BP;

    extern void* addrStackLow;
    extern void* addrStackHigh;
    extern QuaFrame* ASP;

    extern QuaClass** typeArray;
    extern std::map<std::string, uint16_t>* linkedTypes;

    extern Heap* heap;

}

    extern Loader* loader;

    void initGlobals(size_t valStackSize, size_t addrStackSize, size_t heapSize);
    void checkMmap(void* ptr, const char* errMsg);

    extern sigjmp_buf jmpEnv;


#endif // GLOBALS_H
