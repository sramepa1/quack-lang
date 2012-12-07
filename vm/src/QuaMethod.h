#ifndef QUAMETHOD_H
#define QUAMETHOD_H

#include "QuaSignature.h"

extern "C" {
	#include <stdint.h>
}

struct QuaMethod
{
public:

	enum MethodAction {
		ALWAYS_INTERPRET = 0,
		INTERPRET = 1,
		COMPILE = 2,
		JUMPTO = 3,
		C_CALL = 4
	} action;

	void* code;
	QuaSignature* sig;
	uint16_t regCount;
	uint16_t insnCount;

};

#endif // QUAMETHOD_H
