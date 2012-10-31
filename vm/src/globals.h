#ifndef GLOBALS_H
#define GLOBALS_H

/* Assembly-access-friendly VM state globals */

// but is it even needed?

#include <map>
#include <string>

#include "QuaClass.h"
#include "Heap.h"
#include "Loader.h"


extern "C" {

    #include <stdint.h>

    extern void* valStackBase;
    extern uint64_t SPoffset;
    extern uint64_t BPoffset;

    extern void* addrStackBase;
    extern uint64_t ASPoffset;

}

    extern QuaClass** typeArray;
    extern std::map<std::string, uint16_t>* linkedTypes;

    extern Loader* loader;
    extern Heap* heap;


    void initGlobals();

    // TODO signal handler?


#endif // GLOBALS_H
