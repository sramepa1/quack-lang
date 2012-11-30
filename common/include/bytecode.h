#ifndef BYTECODE_H
#define BYTECODE_H

// This is a list of concieved Daisy VM instruction encoding
// Not everything defined here needs to be implemented in v1.0

// One instruction mnemonic is a combination of
// OP_ (opcode, first byte) and SOP_ (sub-opcode, second byte)

// For example, PUSH3 is OP_PUSH, SOP_STACK_3
// and LDCI is OP_LDCT, SOP_TAG_INT

#define NO_SOP 0x0

// specials
#define OP_NOP 0x0
#define OP_HLT 0xFF
#define OP_HCF 0xFE

// Tagged type sub-ops for <T> instructions
#define SOP_TAG_NONE 0x0
#define SOP_TAG_INT 0x1
#define SOP_TAG_FLOAT 0x2
#define SOP_TAG_BOOL 0x3


// DATA MANIPULATION

#define OP_LDC 0x1
#define OP_LDCT 0x2
// supports tags in sub-opcode
#define OP_LDNULL 0x3

#define OP_LDF 0x6
#define OP_STF 0x7
#define OP_LDMYF 0x8
#define OP_STMYF 0x9

#define OP_LDSTAT 0xA

#define OP_MOV 0xD
#define OP_XCHG 0xE


// VALUE STACK ACCESS

#define OP_PUSH 0x10
#define OP_POP 0x11
#define OP_PUSHC 0x12
#define OP_PUSHCT 0x13
#define OP_LDS 0x14
#define OP_STS 0x15

// these sub-opcodes differentiate PUSH2, PUSH3, POP2, POP3, ...
#define SOP_STACK_1 0x0
#define SOP_STACK_2 0x1
#define SOP_STACK_3 0x2
// for a possible future PUSHA where registers mean from .. to
#define SOP_STACK_RANGE 0x10


// ARITHMETICS

// binary operator in 3AC
#define OP_A3REG 0x20

// basic math
#define SOP_ADD 0x0
#define SOP_SUB 0x1
#define SOP_MUL 0x2
#define SOP_DIV 0x3
#define SOP_MOD 0x4

// relations
// these are also used in conditional jumps
#define SOP_EQ 0x5
#define SOP_NEQ 0x6
#define SOP_GT 0x7
#define SOP_GE 0x8
#define SOP_LT 0x9
#define SOP_LE 0xA

// logical operators
#define SOP_LAND 0xB
#define SOP_LOR 0xC

// unary operators (no need for subops here)
#define OP_NEG 0x40
#define OP_LNOT 0x41

// indexing - related to arithmetics because of "_operator..." call
#define OP_IDX 0x42
#define OP_IDXI 0x43


// CODE FLOW

#define OP_JMP 0x50
#define OP_JCC 0x51

#define SOP_CC_NULL 0x0
#define SOP_CC_NNULL 0x1

// these require a TAGGED bool (use CNVT if unsure)
#define SOP_CC_TRUE 0x2
#define SOP_CC_FALSE 0x3


#define OP_CALL 0x52
#define OP_CALLMY 0x53
#define OP_NEW 0x54

#define OP_RET 0x55
#define OP_RETT 0x56
#define OP_RETNULL 0x57

#define OP_TRY 0x59
#define OP_CATCH 0x5A
#define OP_THROW 0x5B
#define OP_THROWT 0x5C
#define OP_FIN 0x5D


// TYPE MANIPULATION

// "classic" instanceof
#define OP_INSTOF 0x60
// "exact" instanceof
#define OP_ISTYPE 0x61

// conversion (required for conditional jumps if the compiler is not sure, uses SOP_ tags)
#define OP_CNVT 0x62


// And that's all, folks.
#endif
