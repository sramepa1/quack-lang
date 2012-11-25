;
; ==== PROTOTYPE QUACK RUNTIME CLASS FILE ====
;
;
; Assemble with "nasm -f bin -o rt.qc rt.asm"
;
; Set your editor's TAB to 4 spaces for optimal viewing
;
;
; This is a prototype binary source for a hand-assembled subset of the Quack runtime library.
;
; Everything is little-endian and, where meaningful, aligned to QWORDs




org 0x0							; File offsets start from zero and are NOT counted from magic

; === QUACK HEADER ===

db 0xD,'U',0xC,'K'				; Magic number (must be within first 1024 bytes, aligned to 8B) 

dw 1							; Class file version 1
dw 1							; Minimum VM version 1

dd classtable;					; Class table offset in file
dd constantpool;				; Constant pool offset in file


; === CLASS TABLE ===

classtable:
	dd .classdeftotalsize		; Class def blob size, including alignment bytes
	dw 5						; Class table item count 
								; (number of class references, not definitions)
	alignb 8, db 0
.cls0:
	dw 0						; CP index 0 = "_DataBlob"
	dw .endblob-.blobclass		; Size of Blob's definition (without alignment)
	dd .blobclass				; Blob definition's offset in the file
.cls1:
	dw 1						; CP index 1 = "System"
	dw .endsys-.sysclass
	dd .sysclass
.cls2:
	dw 2						; CP index 2 = "File"
	dw .endfile-.fileclass
	dd .fileclass
.cls3:
	dw 3						; CP index 3 = "OutFile"
	dw .endout-.outclass
	dd .outclass
.cls4:
	dw 4						; CP index 4 = "InFile"
	dw .endin-.inclass
	dd .inclass
.cls5:
	dw 5						; CP index 5 = "String"
	dw .endstring-.stringclass
	dd .sysclass

; === CLASS DEFINITIONS ===

.classdefs:
.blobclass:
	dw 0						; blob is its own ancestor
	dw 0x8000 | 0x4000			; flags: CLS_INCONSTRUCTIBLE | CLS_VARIABLE_LENGTH
	dw 0						; field count must be zero for variable length classes
	dw 0						; no methods
.endblob:
	alignb 8, db 0
	
.sysclass:
	dw 1						; its own ancestor
	dw 1						; CLS_STATIC
	
	dw 3						; field count
	
	dw 0						; no flags
	dw 6						; "in"
	
	dw 0
	dw 7						; "out"
	
	dw 0
	dw 8						; "err"
	
	dw 1						; method count
		
	dw 1						; flags      = native
	dw 9						; init()
	dw 0						; ignored
.endsys:
	alignb 8, db 0

.fileclass:
	dw 2						; its own ancestor
	dw 2						; CLS_DESTRUCTIBLE
	
	dw 3						; field count
	
	dw 0x8000					; flags = hidden
	dw 0						; ignored
	
	dw 0x8000					; flags = hidden
	dw 0						; ignored
	
	dw 0x8000					; flags = hidden
	dw 0						; ignored
	
	dw 8						; method count
	
	dw 0
	dw 10						; init(arg)
	dw 11						; bytecode stub	
	
	dw 1						; flags      = native
	dw 12						; initN(arg)
	dw 0						; ignored	
	
	dw 1						; flags      = native
	dw 13						; readLine()
	dw 0						; ignored
	
	dw 0
	dw 14						; writeLine(arg)
	dw 15						; bytecode stub
	
	dw 1						; flags      = native
	dw 16						; writeLineN()
	dw 0						; ignored
	
	dw 1						; flags      = native
	dw 17						; eof()
	dw 0						; ignored
	
	dw 0
	dw 18						; close()
	dw 19						; bytecode call finalize
	
	dw 1						; flags      = native
	dw 20						; finalize()
	dw 0						; ignored
.endfile:
	alignb 8, db 0
	
.outclass:
	dw 2						; inherits from File
	dw 2						; CLS_DESTRUCTIBLE
	
	dw 0						; field count
	
	dw 3						; method count
	
	dw 0
	dw 10						; init(arg)
	dw 11						; identical stub!
	
	dw 1						; flags      = native
	dw 12						; initN(arg)
	dw 0						; ignored	
	
	dw 0
	dw 13						; readLine()
	dw 21						; bytecode throw ex
.endout:
	alignb 8, db 0
	
.inclass:
	dw 2						; inherits from File
	dw 2						; CLS_DESTRUCTIBLE
	
	dw 0						; field count
	
	dw 3						; method count
	
	dw 0
	dw 10						; init(arg)
	dw 11						; identical stub!
	
	dw 1						; flags      = native
	dw 12						; initN(arg)
	dw 0						; ignored	
	
	dw 0
	dw 14						; writeLine()
	dw 22						; bytecode throw ex
.endin:
	alignb 8, db 0
	
.stringclass:
	dw 5						; its own ancestor
	dw 0
	
	dw 2						; field count
	
	dw 0x8000					; flags = hidden
	dw 0						; ignored
	
	dw 0						; no flags
	dw 23						; "length"
	
	dw 10						; method count
	
	dw 1						; flags      = native
	dw 9						; init()
	dw 0						; ignored
	
	dw 0
	dw 10						; init(arg)
	dw 11						; identical stub!
	
	dw 1						; flags      = native
	dw 12						; initN(arg)
	dw 0						; ignored
	
	dw 0
	dw 24						; _opPlus(arg)
	dw 25						; bytecode stub
	
	dw 1						; flags      = native
	dw 26						; _opPlusN(arg)
	dw 0						; ignored
	
	dw 0
	dw 27						; _opIndex(arg)
	dw 28						; bytecode stub
	
	dw 1						; flags      = native
	dw 29						; _opIndexN(arg)
	dw 0						; ignored
	
	dw 0
	dw 30						; explode(arg)
	dw 31						; bytecode stub
	
	dw 1						; flags      = native
	dw 32						; explodeN(arg)
	dw 0						; ignored
	
	dw 0
	dw 33						; stringValue()
	dw 34						; bytecode return this
	
	
.endstring:
	alignb 8, db 0

.classdeftotalsize:	equ $-.classdefs


; === CONSTANT POOL ===

constantpool:
	dd itemtotalsize			; Size of the constant pool data
	dw 35						; Item count
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
	dd .item8-constantpool
	dd .item9-constantpool
	dd .item10-constantpool
	dd .item11-constantpool
	dd .item12-constantpool
	dd .item13-constantpool
	dd .item14-constantpool
	dd .item15-constantpool
	dd .item16-constantpool
	dd .item17-constantpool
	dd .item18-constantpool
	dd .item19-constantpool
	dd .item20-constantpool
	dd .item21-constantpool
	dd .item22-constantpool
	dd .item23-constantpool
	dd .item24-constantpool
	dd .item25-constantpool
	dd .item26-constantpool
	dd .item27-constantpool
	dd .item28-constantpool
	dd .item29-constantpool
	dd .item30-constantpool
	dd .item31-constantpool
	dd .item32-constantpool
	dd .item33-constantpool
	dd .item34-constantpool
alignb 8, db 0

; == CP contents ==

.items:
.item0:
	db '_DataBlob',0
	alignb 8, db 0
.item1:
	db 'System',0
	alignb 8, db 0
.item2:
	db 'File',0
	alignb 8, db 0
.item3:
	db 'OutFile',0
	alignb 8, db 0
.item4:
	db 'InFile',0
	alignb 8, db 0
.item5:
	db 'String',0
	alignb 8, db 0
.item6:
	db 'in',0
	alignb 8, db 0
.item7:
	db 'out',0
	alignb 8, db 0
.item8:
	db 'err',0
	alignb 8, db 0
.item9:
	db 0,'init',0
	alignb 8, db 0
.item10:
	db 1,'init',0
	alignb 8, db 0
.item11:
	db 0x57	; RETNULL - init(arg) stub
	alignb 8, db 0
.item12:
	db 1,'initN',0
	alignb 8, db 0
.item13:
	db 0,'readLine',0
	alignb 8, db 0
.item14:
	db 1,'writeLine',0
	alignb 8, db 0
.item15:
	db 0x57	; RETNULL - writeLine(arg) stub
	alignb 8, db 0
.item16:
	db 1,'writeLineN',0
	alignb 8, db 0
.item17:
	db 0,'eof',0
	alignb 8, db 0
.item18:
	db 0,'close',0
	alignb 8, db 0
.item19:
	db 0x57	; RETNULL - close()
	alignb 8, db 0
.item20:
	db 0,'finalize',0
	alignb 8, db 0
.item21:
	db 0x57	; RETNULL - readLine throw ex
	alignb 8, db 0
.item22:
	db 0x57	; RETNULL - writeLine throw ex
	alignb 8, db 0
.item23:
	db 'length',0
	alignb 8, db 0
.item24:
	db 1,'_opPlus',0
	alignb 8, db 0
.item25:
	db 0x57	; RETNULL - opPlus stub
	alignb 8, db 0
.item26:
	db 1,'_opPlusN',0
	alignb 8, db 0
.item27:
	db 1,'_opIndex',0
	alignb 8, db 0
.item28:
	db 0x57	; RETNULL - opIndex stub
	alignb 8, db 0
.item29:
	db 1,'_opIndexN',0
	alignb 8, db 0
.item30:
	db 1,'explode',0
	alignb 8, db 0
.item31:
	db 0x57	; RETNULL - explode stub
	alignb 8, db 0
.item32:
	db 1,'explodeN',0
	alignb 8, db 0
.item33:
	db 0,'stringValue',0
	alignb 8, db 0
.item34:
	db 0x57	; RETNULL - stringValue stub
	alignb 8, db 0
	
itemtotalsize equ $-constantpool

;; === END OF CONSTANT POOL ===		(and of the entire file, too)

