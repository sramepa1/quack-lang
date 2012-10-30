#ifndef INSTRUCTION_H
#define INSTRUCTION_H

extern "C" {
    #include <stdint.h>
}

#pragma pack(1)

#define ARG0 args.triword.arg0
#define ARG1 args.triword.arg1
#define ARG2 args.triword.arg2

#define REG args.regimm.reg
#define IMM args.regimm.imm


struct Instruction {

    unsigned char op;       // main operation code (OP_)
    unsigned char subop;    // sub-operation code (SOP_, only for some instructions)

    union {

        // Most common format - 3AC. Register arithmetics, comparison jumps, LDF, CALL etc.
        // Leave operands uninitialized or zero if instruction uses less than 3
        struct {
            uint16_t arg0;
            uint16_t arg1;
            uint16_t arg2;
        } triword;

        // Big immediate format. For instructions using a large constant, like reg + tagged immediate ops.
        // Leave reg operand uninitialized if there is only the immediate (JMP, RET<T> etc.)
        struct {
            uint16_t reg;
            uint32_t imm;
        } regimm;

    } args;



    // Creates a 0/1/2/3 - word Instruction
    Instruction(unsigned char op,
                unsigned char subop,
                uint16_t _arg0 = 0,
                uint16_t _arg1 = 0,
                uint16_t _arg2 = 0) : op(op), subop(subop) {

        ARG0 = _arg0;
        ARG1 = _arg1;
        ARG2 = _arg2;
    }


    // Creates a big-immediate Instruction
    Instruction(unsigned char op,
                unsigned char subop,
                uint32_t _imm,
                uint16_t _reg = 0) : op(op), subop(subop) {

        IMM = _imm;
        REG = _reg;
    }
};


#pragma pack()
#endif
