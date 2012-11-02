#include "JITCompiler.h"

#ifdef DEBUG
#include <iostream>
extern "C" {
    #include <unistd.h>
    void* jumpto;
}
#endif

#include <cstring>

extern "C" {
    #include <sys/mman.h>
}

using namespace std;

JITCompiler::JITCompiler()
{
}


void JITCompiler::compile(QuaMethod* method) {

    #ifdef PLEASE_CHANGE_THIS_TO_JUST_DEBUG
        if(method->action != QuaMethod::COMPILE) {
            cerr << "Attempted to compile a method with action=" << method->action << " !" << endl;
        }
    #endif

    vector<unsigned char> buffer(4096);

    list<Instruction> insns = buildObjects(method); //signature may change
    allocateRegisters(insns);
    generate(insns, buffer);

    void* memblob = mmap(NULL, buffer.size(), PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    #ifdef DEBUG
        memset(memblob, 0xCC, (buffer.size() / getpagesize() + 1) * getpagesize());
    #endif
    memcpy(memblob, buffer.data(), buffer.size());
    mprotect(memblob, buffer.size(), PROT_READ | PROT_EXEC);

    // TODO enable!
    //method->code = memblob;

    #ifdef DEBUG
        //test
        jumpto = memblob;

        asm volatile(".intel_syntax noprefix \n\t");
        asm volatile("lea eax, jumpto \n\t");
        asm volatile("call [eax] \n\t");
        asm volatile(".att_syntax noprefix \n\t");

        cout << "Generated machine code executed successfully!" << endl;
    #endif
}


list<Instruction> JITCompiler::buildObjects(QuaMethod* method) {
    //TODO
    return list<Instruction>();
}


void JITCompiler::allocateRegisters(list<Instruction> insns) {
    //TODO
}


void JITCompiler::generate(list<Instruction> insns, vector<unsigned char> & buffer) {
    #ifdef DEBUG
        buffer[0] = 0xC3;
    #endif
    //TODO
}

