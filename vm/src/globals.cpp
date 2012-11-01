#include "globals.h"

#include <stdexcept>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <cstdlib>

using namespace std;

extern "C" {
    #include <unistd.h>
    #include <sys/mman.h>

    #include <stdint.h>

    void* valStackLow;
    void* valStackHigh;
    QuaValue* SP;
    QuaValue* BP;

    void* addrStackLow;
    void* addrStackHigh;
    QuaFrame* ASP;

    Heap* heap;

    QuaClass** typeArray;
    map<string, uint16_t>* linkedTypes;
}

    Loader* loader;
    sigjmp_buf jmpEnv;


void initGlobals(size_t valStackSize, size_t addrStackSize, size_t heapSize) {
    valStackLow = mmap(NULL, valStackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    checkMmap(valStackLow, "Could not allocate value stack." );
    valStackHigh = (void*)((char*)valStackLow + valStackSize); // sigh, pedantic mode doesn't like void pointers...
    SP = (QuaValue*) valStackHigh;
    BP = (QuaValue*) valStackHigh;
    // guard page
    mprotect(valStackLow, getpagesize(), PROT_NONE);

    // SIGSEGV test
    *((char*)valStackLow + getpagesize() - 1) = 'a';

    addrStackLow = mmap(NULL, addrStackSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN, -1, 0);
    checkMmap(valStackLow, "Could not allocate address stack." );
    addrStackHigh = (void*)((char*)addrStackLow + addrStackSize); // sigh again...
    // guard page
    mprotect(addrStackLow, getpagesize(), PROT_NONE);
    ASP = (QuaFrame*) addrStackHigh;

    typeArray = new QuaClass*[65536];
    linkedTypes = new map<string, uint16_t>();

    heap = new Heap(heapSize);
    loader = new Loader();
}


void checkMmap(void* ptr, const char* errMsg) {
    if(ptr == MAP_FAILED) {
        int err = errno;
        ostringstream os;
        os << errMsg << endl;
        os << "Memory mapping failed with error code " << err << ", that is \"" << strerror(err) << "\"." << endl;
        throw runtime_error(os.str());
    }
}
