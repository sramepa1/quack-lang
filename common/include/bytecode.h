#ifndef BYTECODE_H
#define BYTECODE_H

// This is a list of concieved Daisy VM instruction encoding
// Not everything defined here needs to be implemented in v1.0

// One instruction mnemonic is a combination of
// OP_ (opcode, first byte) and SOP_ (sub-opcode, second byte)

// For example, PUSH3 is OP_PUSH, SOP_STACK_3
// and LDCI is OP_LDCT, SOP_TAG_INT

/**
 *	Empty sub-opcode for instructions that do not use them
 */
#define NO_SOP 0x0



// specials


/**
 *	No operation. No subop, no args.
 *	NOP
 */
#define OP_NOP 0x0

/**
 *	Halt. No subop, no args.
 *	HLT
 */
#define OP_HLT 0xFF

/**
 *	Halt and catch fire (see Wikipedia). No subop, no args.
 *	HCF
 */
#define OP_HCF 0xFE



// Tagged type sub-ops for <T> instructions


#define SOP_TAG_NONE 0x0
#define SOP_TAG_INT 0x1
#define SOP_TAG_FLOAT 0x2
#define SOP_TAG_BOOL 0x3



// DATA MANIPULATION


/**
 *	Load constant from CP. No subop.
 *	LDC reg dest, imm16 constant type, imm16 CP index (constant data)
 */
#define OP_LDC 0x1

/**
 *	Load immediate tagged constant. Subop specifies tag, VM determines type.
 *	LDC<T> reg dest, imm32 value
 */
#define OP_LDCT 0x2

/**
 *	Load a NULL reference. No subop, no args.
 *	LDNULL reg dest
 */
#define OP_LDNULL 0x3


/**
 *	Load field contents. No subop.
 *	LDF reg dest, reg that, imm16 CP index (field name)
 */
#define OP_LDF 0x6

/**
 *	Store field contents. No subop.
 *	STF reg that, imm16 CP index (field name), reg src
 */
#define OP_STF 0x7

/**
 *	Load field contents from *this. No subop.
 *	LDMYF reg dest, imm16 field index
 */
#define OP_LDMYF 0x8

/**
 *	Store field contents for *this. No subop.
 *	STMYF imm16 field index, reg src
 */
#define OP_STMYF 0x9


/**
 *	Load static class instance. No subop.
 *	LDSTAT reg dest, imm16 class type
 */
#define OP_LDSTAT 0xA


/**
 *	Copy register contents. No subop.
 *	MOV reg dest, reg src
 */
#define OP_MOV 0xD

/**
 *	Exchange register contents. No subop.
 *	XCHG reg 1, reg 2
 */
#define OP_XCHG 0xE



// VALUE STACK ACCESS


/**
 *	Push or pop registers to/from stack. Subop specifies how many.
 */
#define OP_PUSH 0x10
#define OP_POP 0x11

// these sub-opcodes differentiate PUSH, PUSH2, PUSH3, PUSHA, POP, POP2, POP3, POPA
// For 2 or more registers, argument order is push order, and remains the same for popping them back
// (VM reverses the order internally so that PUSH2 r1, r2 is complemented by POP2 r1, r2 and values are NOT exchanged)

/**
 *	PUSH/POP reg
 */
#define SOP_STACK_1 0x0

/**
 *	PUSH2/POP2 reg 1, reg 2
 */
#define SOP_STACK_2 0x1

/**
 *	PUSH3/POP3 reg 1, reg 2, reg 3
 */
#define SOP_STACK_3 0x2

/**
 *	Push or pop all registers from start to end, end included.
 *	PUSHA/POPA reg start, reg end
 */
#define SOP_STACK_RANGE 0x10



/**
 *	Push a constant from CP. No subop.
 *	PUSHC imm16 constant type, imm16 CP index (constant data)
 */
#define OP_PUSHC 0x12

/**
 *	Push immediate tagged constant. Subop specifies tag, VM determines type.
 *	PUSHC<T> imm32 value
 */
#define OP_PUSHCT 0x13

/**
 *	Load value from stack, offset relative to BP. No subop.
 *	LDS reg dest, imm16 (signed) BP offset
 */
#define OP_LDS 0x14

/**
 *	Store value to stack, offset relative to BP. No subop.
 *	STS imm16 (signed) BP offset, reg src
 */
#define OP_STS 0x15




// ARITHMETICS


/**
 *	3AC binary operator. Subop defines operation used.
 *	[SOP] reg dest, reg left operand, reg right operand
 */
#define OP_A3REG 0x20

// basic math ( + - * / % )
#define SOP_ADD 0x0
#define SOP_SUB 0x1
#define SOP_MUL 0x2
#define SOP_DIV 0x3
#define SOP_MOD 0x4

// comparison ( == != > >= < <= )
#define SOP_EQ 0x5
#define SOP_NEQ 0x6
#define SOP_GT 0x7
#define SOP_GE 0x8
#define SOP_LT 0x9
#define SOP_LE 0xA

// logical operators ( && || )
#define SOP_LAND 0xB
#define SOP_LOR 0xC


// unary operators (no need for subops here)

/**
 *	Unary - (arithmetic negation). No subop.
 *	NEG reg dest, reg src
 */
#define OP_NEG 0x40

/**
 *	Unary ! (logical negation). No subop.
 *	LNOT reg dest, reg src
 */
#define OP_LNOT 0x41


// indexing - related to arithmetics because of "_op..." call

/**
 *	Get indexed value i.e. *that[index]. No subop.
 *	IDX	reg dest, reg that, reg index
 */
#define OP_IDX 0x42

/**
 *	Get indexed value with immediate i.e. *that[imm]. No subop.
 *	IDXI	reg dest, reg that, imm16 index (unsigned)
 */
#define OP_IDXI 0x43



// CODE FLOW

// All destination addresses are always signed and relative to next instruction.
// -> JMP 0 is a nop and continues, JMP -1 is an infinite loop and JMP 1 jumps over 1 instruction

/**
 *	Jump. Subop defines condition.
 */
#define OP_JMP 0x50

/**
 *	Unconditional jump
 *	JMP imm16 offset
 */
#define SOP_UNCONDITIONAL 0x0


/**
 *	Jump if a value is NULL
 *	JNULL	imm16 offset, reg cond
 */
#define SOP_CC_NULL 0x1

/**
 *	Jump if a value is not NULL
 *	JNULL	imm16 offset, reg cond
 */
#define SOP_CC_NNULL 0x2


// These jumps operate on tagged boolean values only.
// Unless using the result of a native method call guarenteed to return one,
//  compillers must prepend a CNVT<B> instruction to extract boolValue

/**
 *	Jump if a value is a tagged TRUE
 *	JTRUE	imm16 offset, reg cond
 */
#define SOP_CC_TRUE 0x3

/**
 *	Jump if a value is a tagged FALSE
 *	JTRUE	imm16 offset, reg cond
 */
#define SOP_CC_FALSE 0x4


// Calls should be preceded by PUSHing arguments (count must match the signature)
// Use REG_DEV_NULL as destination if you want to throw away the return value
#define REG_DEV_NULL 0xFFFF

/**
 *	Call that->method(...). No subop.
 *	CALL reg dest, reg that, imm16 CP index (signature)
 */
#define OP_CALL 0x52

/**
 *	Call this->method(...). No subop.
 *	CALL reg dest, imm16 CP index (signature)
 */
#define OP_CALLMY 0x53

/**
 *	Create a new instance using a type and constructor signature. No subop.
 *	NEW reg dest, imm16_type, imm16 CP index (constructor signature)
 */
#define OP_NEW 0x54


/**
 *	Return a value. No subop.
 *	RET reg src
 */
#define OP_RET 0x55

/**
 *	Return an immediate constant. Subop specifies tag, VM determines type.
 *	RET<T>	imm32 value
 */
#define OP_RETT 0x56

/**
 *	Return NULL. No subop.
 */
#define OP_RETNULL 0x57


/**
 *	Enter guarded section (push finally-handler i.e. what is after the '}' of the last catch).
 *	Should be immediately followed by at least one CATCH. No subop.
 *	TRY	imm16 handler offset
 */
#define OP_TRY 0x59

/**
 *	Push an exception handler. No subop.
 *	CATCH imm16 exception type, imm16 handler offset
 */
#define OP_CATCH 0x5A

/**
 *	Throw a value as an exception. No subop.
 *	THROW reg src
 */
#define OP_THROW 0x5B

/**
 *	Throw an immediate constant as an exception. Subop specifies tag, VM determines type.
 *	THROW<T>	imm32 value
 */
#define OP_THROWT 0x5C

/**
 *	Leave guarded section (discard exception handlers and jump to finally-handler).
 *	Must be present at the end of the main codepath (TRY - CATCH - CATCH - ... - FIN)
 *	 and at the end of all exception handlers as well (unless they RET or THROW).
 *	No subop, no arguments.
 *	FIN
 */
#define OP_FIN 0x5D



// TYPE MANIPULATION

/**
 *	Liskov-aware instanceof (descendants are considered valid instances of a type).
 *	Returns a tagged bool. No subop.
 *	INSTOF	reg dest, reg what, imm16 what type
 */
#define OP_INSTOF 0x60

/**
 *	Exact instanceof (Class must be the same and descendants are NOT considered valid instances of a type).
 *	Returns a tagged bool. No subop.
 *	ISTYPE	reg dest, reg what, imm16 what type
 */
#define OP_ISTYPE 0x61

/**
 *	Conversion to a tagged value (via repeated bool/int/floatValue calls). Subop specifies target tag.
 *	Using CNVTB is pretty much mandatory before JTRUE or JFALSE. Also, TAG_REF subop makes this a nop.
 *	CNVT<T> reg dest, reg src
 */
#define OP_CNVT 0x62


// And that's all, folks.
#endif
