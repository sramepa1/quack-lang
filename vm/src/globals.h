#ifndef GLOBALS_H
#define GLOBALS_H

#define VM_VERSION 1

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
#include "Interpreter.h"
#include "NativeLoader.h"


// These symbols might be needed from assembly sources -> disable mangling

extern "C" {

	#include <stdint.h>

	extern void* valStackLow;
	extern void* valStackHigh;
	extern QuaValue* VMSP;
	extern QuaValue* VMBP;

	extern void* addrStackLow;
	extern void* addrStackHigh;
	extern QuaFrame* ASP;

	extern QuaClass** typeArray;
	extern std::map<std::string, uint16_t>* linkedTypes;

	extern Heap* heap;

}

	extern NativeLoader* nativeLoader;
	extern Loader* loader;
	extern Interpreter* interpreter;

	void initGlobals(bool jit, size_t valStackSize, size_t addrStackSize, size_t heapSize);

	extern sigjmp_buf jmpEnv;

	// Chache for popular types
	// Loader is responsible to fill it
	extern struct TypeCache {
		uint16_t typeNull;
		uint16_t typeDataBlob;
		uint16_t typeBool;
		uint16_t typeInteger;
		uint16_t typeFloat;
		uint16_t typeString;
        uint16_t typeArray;
		uint16_t typeException;
		uint16_t typeNotFoundException;
		uint16_t typeIOException;
	} typeCache;

#endif // GLOBALS_H
