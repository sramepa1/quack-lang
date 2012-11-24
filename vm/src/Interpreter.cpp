#include "Interpreter.h"
#include "bytecode.h"
#include "globals.h"
#include "helpers.h"

#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>

#ifdef DEBUG
#include <iostream>
#endif

using namespace std;

Interpreter::Interpreter() : regs(vector<QuaValue>(65536)), compiler(new JITCompiler())
{
}

void Interpreter::start(uint16_t mainClassType) {

#ifdef DEBUG
    cout << "Initializing interpreter environment, Main class has type " << mainClassType << endl;
#endif

    QuaClass* mainClass = resolveType(mainClassType);

    // push This pointer
    *(--BP) = heap->allocateNew(mainClassType, mainClass->getFieldCount());
    --SP;

    // push exit handler
    *(--ASP) = QuaFrame(NULL, false, false);

    // get entry point
    Instruction* pc = (Instruction*)(mainClass->lookupMethod((QuaSignature*)"\1main")->code);

#ifdef DEBUG
    cout << "Entering interpreter loop, entry point address is " << pc << endl;
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

        case OP_LDF:	return handleLDF(insn);
        case OP_STF:	throw runtime_error("Instruction not yet implemented.");
        case OP_LDMYF:	throw runtime_error("Instruction not yet implemented.");
        case OP_STMYF:	throw runtime_error("Instruction not yet implemented.");

        case OP_LDSTAT:	return handleLDSTAT(insn);

        case OP_MOV:	throw runtime_error("Instruction not yet implemented.");
        case OP_XCHG:	throw runtime_error("Instruction not yet implemented.");
        case OP_PUSH:	throw runtime_error("Instruction not yet implemented.");
        case OP_POP:	throw runtime_error("Instruction not yet implemented.");
        case OP_PUSHC:	return handlePUSHC(insn);
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

        case OP_CALL:	return handleCALL(insn);
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
        case OP_HLT:    throw ExitException();

        default:        return handleIllegalInstruction(insn);
    }

    // SIGSEGV tests
    //*((char*)valStackLow + getpagesize() - 1) = 'a';
    //*((char*)addrStackLow + getpagesize() - 1) = 'a';
    //*((char*)heap->getEnd() - 1) = 'a';
    //*((char*)NULL) = 'a';

    //compiler->compile(NULL);
}

// may need ASM implementation
Instruction* Interpreter::performCall(QuaMethod* method) {
    switch(method->action) {
        case QuaMethod::INTERPRET:  return (Instruction*) method->code; //TODO set flag to compile
        case QuaMethod::COMPILE:    compiler->compile(method); // and fall-through
        case QuaMethod::JUMPTO:     throw runtime_error("Jumping to compiled code blobs is not yet implemented.");
        case QuaMethod::C_CALL:     return performReturn( ( (QuaValue (*)())method->code )() );
    }
}

Instruction* Interpreter::performReturn(QuaValue retVal) {

    // discard adress stack junk
    while(ASP->FRAME_TYPE == EXCEPTION || ASP->FRAME_TYPE == FINALLY) {
        ++ASP;
    }
    if(ASP->FRAME_TYPE == EXIT) {
        throw ExitException(); // TODO Is this safe if code had passed through ASM?
    }

    // The frame is a correct return address (frameType == METHOD)
    SP = BP + (ASP->ARG_COUNT + 1);
    BP = (QuaValue*)valStackHigh - ASP->BP_OFFSET;
    if(ASP->INTERPRETED) {
        regs[ASP->DEST_REG] = retVal;
        return (Instruction*)((ASP++)->code);
    } else {
        ++ASP;
        // TODO where to put return value?
        throw runtime_error("Returning to machine code is not yet implemented");
    }
}


Instruction* Interpreter::handleIllegalInstruction(Instruction* insn) {
    ostringstream os;
    os << "Illegal instruction opcode 0x" << setw(2) << setfill('0') << uppercase << hex << (int)insn->op
       << " encountered at " << insn << "." << endl;
    // TODO improve this error message - print current class and method names
    throw runtime_error(os.str());
}


Instruction* Interpreter::handleLDC(Instruction* insn) {

    regs[insn->ARG0] = loadConstant(insn->ARG1, insn->ARG2);
    return ++insn;
}


Instruction* Interpreter::handleLDF(Instruction* insn) {

    regs[insn->ARG0] = getFieldByName(regs[insn->ARG1], getCurrentCPEntry(insn->ARG2));
    return ++insn;
}


Instruction* Interpreter::handleLDSTAT(Instruction* insn) {

    QuaClass* statclass = resolveType(insn->ARG1);
    QuaValue instance = statclass->getInstance();
    if(instance.value == 0) {
        instance = heap->allocateNew(insn->ARG1, statclass->getFieldCount());
        statclass->setInstance(instance);
        functionPrologue(instance, insn, true, 0, insn->ARG0);
        return performCall(statclass->lookupMethod((QuaSignature*)"\0init"));
    }
    regs[insn->ARG0] = instance;
    return ++insn;
}



Instruction* Interpreter::handlePUSHC(Instruction* insn) {

    *(--SP) = loadConstant(insn->ARG0, insn->ARG1);
    return ++insn;
}



Instruction* Interpreter::handleCALL(Instruction* insn) {

    QuaSignature* methSig = (QuaSignature*)getCurrentCPEntry(insn->ARG2);
    functionPrologue(regs[insn->ARG1], insn + 1, true, methSig->argCnt, insn->ARG0);
    return performCall(getClass(regs[insn->ARG1])->lookupMethod(methSig));
}



