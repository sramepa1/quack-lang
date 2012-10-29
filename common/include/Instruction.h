#ifndef INSTRUCTION_H
#define INSTRUCTION_H

extern "C" {
    #include <stdint.h>
}

#pragma pack(1)

#define ARG0 args.triword.arg0
#define ARG1 args.triword.arg1
#define ARG2 args.triword.arg2

#define REG0 args.regimm.reg0
#define IMM1 args.regimm.imm1

#define WORD0 args.twotwo.word0;
#define WORD1 args.twotwo.word1;
#define BYTE0 args.twotwo.byte0;
#define BYTE1 args.twotwo.byte1;


struct Instruction {

    unsigned char op;
    unsigned char subop;

    union {

        // Most common format - 3AC. Register arithmetics, comparison jumps, LDF, CALL etc.
        // Leave operands uninitialized if instruction uses less than 3
        struct {
            uint16_t arg0;
            uint16_t arg1;
            uint16_t arg2;
        } triword;

        // Big immediate format. For instructions using a large constant, like reg + tagged immediate ops.
        // Leave reg operand uninitialized if there is only the immediate (JMP, RET<T> etc.)
        struct {
            uint16_t reg0;
            uint32_t imm1;
        } regimm;

        // Special format with 8-bit operands
        // Currently only used by NEW, which leaves byte1 uninitialized
        struct {
            uint16_t word0;
            uint16_t word1;
            unsigned char byte0;
            unsigned char byte1;
        } twotwo;

    } args;
};


#pragma pack()
#endif
