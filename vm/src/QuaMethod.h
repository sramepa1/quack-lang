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
		INTERPRET = 0,
		COMPILE = 1,
		JUMPTO = 2,
		C_CALL = 3,
		ALWAYS_INTERPRET = 4
	} action;

	void* code;
	QuaSignature* sig;
	uint16_t regCount;
	uint16_t insnCount;

};

#endif // QUAMETHOD_H
