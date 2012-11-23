#include "Interpreter.h"
#include "bytecode.h"
#include "globals.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

Interpreter::Interpreter() : compiler(new JITCompiler())
{
}

void Interpreter::start(Instruction* entryPoint) {
    Instruction* pc = entryPoint;

#ifdef DEBUG
    cout << "Entering interpreter loop, entry point address is " << entryPoint << endl;
#endif

    while(1) {
        pc = processInstruction(pc);
    }
}



Instruction* Interpreter::processInstruction(Instruction* insn) {


    /*

               *************
               *    ON     *
               *           *
               *  [=====]  *
               *   |   |   *
               *   |   |   *
               *   +   +   *
               *           *
               *           *
               *           *
               *           *
               *    OFF    *
               *************

        +--------------------------+
        | THE BIG SWITCH (TM) No.1 |
        +--------------------------+


    (this monstrosity compiles into a jump table,
     which means very fast interpretation dispatch)

    */


    switch(insn->op) {

        case OP_NOP:    return ++insn;

        case OP_LDC:    return handleLDC(insn);
        case OP_LDCT:   throw runtime_error("Instruction not yet implemented.");
        case OP_LDNULL:	throw runtime_error("Instruction not yet implemented.");

        case OP_LDF:	throw runtime_error("Instruction not yet implemented.");
        case OP_STF:	throw runtime_error("Instruction not yet implemented.");
        case OP_LDMYF:	throw runtime_error("Instruction not yet implemented.");
        case OP_STMYF:	throw runtime_error("Instruction not yet implemented.");

        case OP_LDSTAT:	throw runtime_error("Instruction not yet implemented.");

        case OP_MOV:	throw runtime_error("Instruction not yet implemented.");
        case OP_XCHG:	throw runtime_error("Instruction not yet implemented.");
        case OP_PUSH:	throw runtime_error("Instruction not yet implemented.");
        case OP_POP:	throw runtime_error("Instruction not yet implemented.");
        case OP_PUSHC:	throw runtime_error("Instruction not yet implemented.");
        case OP_PUSHCT:	throw runtime_error("Instruction not yet implemented.");
        case OP_LDS:	throw runtime_error("Instruction not yet implemented.");
        case OP_STS:	throw runtime_error("Instruction not yet implemented.");

        case PART_A3REG|PART_ADD:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_SUB:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_MUL:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_DIV:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_MOD:	throw runtime_error("Instruction not yet implemented.");

        case PART_A3REG|PART_EQ:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_NEQ:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_GT:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_GE:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_LT:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_LE:	throw runtime_error("Instruction not yet implemented.");

        case PART_A3REG|PART_LAND:	throw runtime_error("Instruction not yet implemented.");
        case PART_A3REG|PART_LOR:	throw runtime_error("Instruction not yet implemented.");

        case PART_AREGIMM|PART_ADD:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_SUB:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_MUL:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_DIV:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_MOD:	throw runtime_error("Instruction not yet implemented.");

        case PART_AREGIMM|PART_EQ:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_NEQ:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_GT:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_GE:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_LT:	throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_LE:	throw runtime_error("Instruction not yet implemented.");

        case PART_AREGIMM|PART_LAND:throw runtime_error("Instruction not yet implemented.");
        case PART_AREGIMM|PART_LOR:	throw runtime_error("Instruction not yet implemented.");

        case OP_NEG:	throw runtime_error("Instruction not yet implemented.");
        case OP_LNOT:	throw runtime_error("Instruction not yet implemented.");

        case OP_IDX:	throw runtime_error("Instruction not yet implemented.");
        case OP_IDXI:	throw runtime_error("Instruction not yet implemented.");

        case OP_JMP:	throw runtime_error("Instruction not yet implemented.");
        case OP_JCC:	throw runtime_error("Instruction not yet implemented.");

        case OP_CALL:	throw runtime_error("Instruction not yet implemented.");
        case OP_CALLMY:	throw runtime_error("Instruction not yet implemented.");
        case OP_NEW:	throw runtime_error("Instruction not yet implemented.");

        case OP_RET:	throw runtime_error("Instruction not yet implemented.");
        case OP_RETT:	throw runtime_error("Instruction not yet implemented.");
        case OP_RETNULL:throw runtime_error("Instruction not yet implemented.");

        case OP_TRY:	throw runtime_error("Instruction not yet implemented.");
        case OP_CATCH:	throw runtime_error("Instruction not yet implemented.");
        case OP_THROW:	throw runtime_error("Instruction not yet implemented.");
        case OP_THROWT:	throw runtime_error("Instruction not yet implemented.");
        case OP_FIN:	throw runtime_error("Instruction not yet implemented.");

        case OP_HCF:
        case OP_HLT:   throw ExitException();

        default:
            ostringstream os;
            os << "Illegal instruction opcode 0x" << setw(2) << setfill('0') << uppercase << hex << (int)insn->op << " encountered at " << insn << "." << endl;
            // TODO improve this error message - print current class and method names
            throw runtime_error(os.str());
    }

    // SIGSEGV tests
    //*((char*)valStackLow + getpagesize() - 1) = 'a';
    //*((char*)addrStackLow + getpagesize() - 1) = 'a';
    //*((char*)heap->getEnd() - 1) = 'a';
    //*((char*)NULL) = 'a';

    //compiler->compile(NULL);
}


Instruction* Interpreter::handleLDC(Instruction* insn) {
    return ++insn; //TODO
}
