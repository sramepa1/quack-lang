#ifndef BYTECODE_H
#define BYTECODE_H

// This is a list of concieved Daisy VM instruction encoding
// Not everything defined here needs to be implemented in v1.0

// One instruction mnemonic is a combination of
// OP_ (opcode, first byte) and SOP_ (sub-opcode, second byte)

// For example, PUSH3 is OP_PUSH, SOP_STACK_3
// and LDCI is OP_LDCT, SOP_TAG_INT


// specials
#define OP_NOP 0x0
#define OP_HLT 0xFF
#define OP_HCF 0xFE


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

// To avoid duplicities, arithmetic instructions have form in higher nybble of the OP_ byte
// and operation in its lower nybble. These two PART_s should be or'd together to form an OP_
// For example, "MULI reg, imm32" is op=(PART_AREGIMM | PART_MUL), subop = SOP_TAG_INT

#define PART_A3REG 0x20
#define PART_AREGIMM 0x30

// Tagged types for the Reg-Immediate variant
#define SOP_TAG_NONE 0x0
#define SOP_TAG_INT 0x1
#define SOP_TAG_FLOAT 0x2
#define SOP_TAG_BOOL 0x3

// basic math
#define PART_ADD 0x0
#define PART_SUB 0x1
#define PART_MUL 0x2
#define PART_DIV 0x3
#define PART_MOD 0x4

// relations
// these are also used in conditional jumps
#define PART_EQ 0x5
#define PART_NEQ 0x6
#define PART_GT 0x7
#define PART_GE 0x8
#define PART_LT 0x9
#define PART_LE 0xA

// logical operators
#define PART_LAND 0xB
#define PART_LOR 0xC

// unary operators
#define OP_NEG 0x40
#define OP_LNOT 0x41

// indexing - related to arithmetics because of "_operator..." call
#define OP_IDX 0x42
#define OP_IDXI 0x43


// CODE FLOW

#define OP_JMP 0x50
#define OP_JCC 0x51

// conditional jump takes a sub-op with values shared with relational operator PART_s
#define SOP_CC_NULL 0x0
#define SOP_CC_NNULL 0x1
#define SOP_CC_TRUE 0x2
#define SOP_CC_FALSE 0x3

#define SOP_CC_EQ PART_EQ
#define SOP_CC_NEQ PART_NEQ
#define SOP_CC_GT PART_GT
#define SOP_CC_GE PART_GE
#define SOP_CC_LT PART_LT
#define SOP_CC_LE PART_LE

#define OP_CALL 0x52
#define OP_CALLMY 0x53
#define OP_NEW 0x54

#define OP_RET 0x55
#define OP_RETT 0x56
#define OP_RETNULL 0x57

#define OP_TRY 0x58
#define OP_CATCH 0x59
#define OP_THROW 0x5A
#define OP_THROWT 0x5B
#define OP_FIN 0x5C


// And that's all, folks.
#endif
