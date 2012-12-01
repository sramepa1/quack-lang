#include "globals.h"
#include "helpers.h"

#include <stdexcept>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <cstdlib>

#ifdef DEBUG
#include <iostream>
#endif

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

	NativeLoader* nativeLoader;
	Loader* loader;
	Interpreter* interpreter;

	sigjmp_buf jmpEnv;
	TypeCache typeCache;


void initGlobals(bool jit, size_t valStackSize, size_t addrStackSize, size_t heapSize) {

#ifdef DEBUG
	cout << "=== Initializing core VM structures... ===" << endl;
#endif

	valStackLow = mmap(NULL, valStackSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
	checkMmap(valStackLow, "Could not allocate value stack." );
	valStackHigh = (void*)((char*)valStackLow + valStackSize);
	SP = (QuaValue*) valStackHigh;
	BP = (QuaValue*) valStackHigh;
	// guard page
	mprotect(valStackLow, getpagesize(), PROT_NONE);

#ifdef DEBUG
	cout << "Value stack initialized successfully" << endl;
#endif

	addrStackLow = mmap(NULL, addrStackSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
	checkMmap(valStackLow, "Could not allocate address stack." );
	addrStackHigh = (void*)((char*)addrStackLow + addrStackSize);
	// guard page
	mprotect(addrStackLow, getpagesize(), PROT_NONE);
	ASP = (QuaFrame*) addrStackHigh;

#ifdef DEBUG
	cout << "Address stack initialized successfully" << endl;
#endif

	typeArray = new QuaClass*[65536];
	linkedTypes = new map<string, uint16_t>();

	heap = new Heap(heapSize);

#ifdef DEBUG
	cout << "Heap initialized successfully" << endl;
#endif

	nativeLoader = new NativeLoader();
	loader = new Loader();

	interpreter = new Interpreter(jit);

#ifdef DEBUG
	cout << "=== All core VM structures initialized. ===" << endl;
#endif
}
