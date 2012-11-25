#ifndef JITCOMPILER_H
#define JITCOMPILER_H

#include <vector>
#include <list>

#include "Instruction.h"
#include "QuaMethod.h"

class JITCompiler
{
public:
    JITCompiler();

    void compile(QuaMethod* method);

private:
    std::list<Instruction> buildObjects(QuaMethod* method);
    void allocateRegisters(std::list<Instruction> insns);
    void generate(std::list<Instruction> insns, std::vector<unsigned char> & buffer);
};

#endif // JITCOMPILER_H