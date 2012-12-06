#ifndef JITCOMPILER_H
#define JITCOMPILER_H

#include <vector>
#include <list>

extern "C" {
#include "stdint.h"
}

#include "Instruction.h"
#include "QuaMethod.h"

class GiveUpException {};

class JITCompiler
{
public:
	JITCompiler(bool enabled); // Defined in interpreter.cpp

	bool compile(QuaMethod* method);

private:

	bool enabled;

	static void* callLabel;
	static void* returnLabel;
	static void* throwLabel;

	static void* whatLabel;

	std::list<Instruction*> buildObjects(QuaMethod* method);

	void updateMax(uint16_t& maxReg, uint16_t insnReg);
	void allocateRegisters(std::list<Instruction*> insns);
	void generate(std::list<Instruction*> insns, std::vector<unsigned char> & buffer);
};

#endif // JITCOMPILER_H
