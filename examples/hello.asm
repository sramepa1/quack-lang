;
; ==== EXAMPLE QUACK CLASS FILE ====
;
;
; Assemble with "nasm -f bin -o hello.qc hello.asm"
;
; Set your editor's TAB to 4 spaces for optimal viewing
;
;
; This is a commented binary source for a hand-assembled "Hello, world" program:
;
;	statclass Main {
;		fun main(args) {
;			@System.out->writeLine("Hello, world!");
;		}
;	}
;
; Everything is little-endian and, where meaningful, aligned to QWORDs




org 0x0							; File offsets start from zero and are NOT counted from magic

; === UNIX PROLOG === (optional)

db '#!/usr/bin/daisy',0xA		; Why not be compatible with unix interpreter notation?
alignb 8, db 0					; Alignment padding


; === QUACK HEADER ===

db 0xD,'U',0xC,'K'				; Magic number (must be within first 1024 bytes, aligned to 8B) 

dw 1							; Class file version 1
dw 1							; Minimum VM version 1

dd classtable;					; Class table offset in file
dd constantpool;				; Constant pool offset in file


; === CLASS TABLE ===

classtable:
	dd .classdeftotalsize		; Class def blob size, including alignment bytes
	dw 3						; Class table item count 
								; (number of class references, not definitions)
	alignb 8, db 0
.cls0:
	dw 0						; CP index 0 = "Main"
	dw .endmain-.mainclass		; Size of Main's definition (without alignment)
	dd .mainclass-classtable	; Main definition's offset from start of class table
.cls1:
	dw 2						; CP index 2 = "System"
	dw 0						; external
	dd 0
.cls2:
	dw 5						; CP index 5 = "String"
	dw 0						; external
	dd 0

; === CLASS DEFINITIONS === 	(only one here)

.classdefs:
.mainclass:
	dw 0						; Main is its own ancestor
								; = Parent index in CT, must be <= current index
	dw 1						; Flags: CLS_STATIC
	dw 0						; 0 fields
	; field name indices array would be here if Main had fields
	dw 1						; 1 method

	dw 0						; flags      = empty (normal, fully accessible method)
	dw 1						; CP index 1 = signature of main(args)
	dw 7						; CP index 6 = bytecode of main(args)
	dw 5						; Bytecode instruction count
	dw 1						; method uses exactly one register, r0
.endmain:
	alignb 8, db 0
.classdeftotalsize:	equ $-.classdefs	; classdef size calculation (nasm pseudoinstruction)


; === CONSTANT POOL ===

constantpool:
	dd itemtotalsize			; Size of the constant pool data
	dw 8						; Item count
alignb 8, db 0

.offsets:						; Offset array, CP offsets are relative to CP's start
	dd .item0-constantpool
	dd .item1-constantpool
	dd .item2-constantpool
	dd .item3-constantpool
	dd .item4-constantpool
	dd .item5-constantpool
	dd .item6-constantpool
	dd .item7-constantpool	
alignb 8, db 0

; == CP contents ==

.items:
.item0:
	db 'Main',0
	alignb 8, db 0
.item1:
	db 1,'main',0
	alignb 8, db 0
.item2:
	db 'System',0
	alignb 8, db 0
.item3:
	db 'out',0
	alignb 8, db 0
.item4:
	db 1,'writeLineN',0		; bypass stub
	alignb 8, db 0
.item5:
	db 'String',0
	alignb 8, db 0
.item6:
	db 'Hello, world!',0
	alignb 8, db 0
.item7:
	
;; ==== BYTECODE OF MAIN(args) ==== ;;
;   opcodes are subject to change
;   uncommmented bytes are padding

	; LDSTAT r0, 1
	
	db 0xA			; LDSTAT
	db 0
	dw 0			; Destination r0
	dw 1 | 0x8000	; CT index 1 = System. High bit = unresolved
	dw 0
	
	
	; LDF r0, r0, 3
	
	db 0x6			; LDF
	db 0
	dw 0			; Destination r0
	dw 0			; 'That' is in r0
	dw 3			; CP index 3 = "out"
	
	
	; PUSHC 2, 6
	
	db 0x12			; PUSHC
	db 0
	dw 2 | 0x8000	; CT index 2 = String. High bit = unresolved
	dw 6			; CP index 6 = "Hello, world!"
	dw 0
	
	
	; CALL r1, r0, 4
	
	db 0x52			; CALL
	db 0
	dw 0xFFFF		; Destination REG_DEV_NULL (throw away return value)
	dw 0			; 'That' is in r0
	dw 4			; CP index 4 = signature of writeLine(str)

;
;	; HLT			(temporary substitution until returning from Main was implemented)
;	
;	db 0xFF			; HLT
;	times 7 db 0
;
	
	; RETNULL
	
	db 0x57	; RETNULL
	times 7 db 0

;; ================================ ;;

	
	alignb 8, db 0					; Align end of CP (not necessary here)
	
itemtotalsize equ $-constantpool			; CP contents size calculation (nasm pseudoinstruction)

;; === END OF CONSTANT POOL ===		(and of the entire file, too)

