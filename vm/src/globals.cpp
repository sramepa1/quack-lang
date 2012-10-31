#include "globals.h"

#include <cstdlib>
#include <sys/mman.h>

using namespace std;


extern "C" {

    #include <stdint.h>

    void* valStackBase;
    void* SP;
    void* BP;

    void* addrStackBase;
    void* ASP;

}

    Heap* heap;
    Loader* loader;
    QuaClass** typeArray;
    map<string, uint16_t>* linkedTypes;


void initGlobals() {
    valStackBase = mmap(NULL, 1024*1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    SP = valStackBase;
    BP = valStackBase;

    addrStackBase = mmap(NULL, 1024*1024, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    ASP = addrStackBase;

    typeArray = new QuaClass*[65536];
    linkedTypes = new map<string, uint16_t>();

    heap = new Heap();
    loader = new Loader();

}
