#ifndef _QUA_FRAME_H_ 
#define _QUA_FRAME_H_

#pragma pack(1)

#define EXIT 0
#define METHOD 1
#define EXCEPTION 2
#define FINALLY 3

struct QuaFrame {
	
	void* code;
	struct {
		unsigned frameType : 2;
		unsigned interpreted : 1;
		unsigned : 5;
	} type;
	
	union {
		struct {
			char argCount;
			uint16_t destReg;
			uint32_t bpOffset;
		} methodFrame;
	
		uint16_t exceptionType;
	} typeData;
	
	// creates method frame
	QuaFrame(void* code, bool interpreted, char argCount, uint16_t destReg, uint32_t bpOffset)
		: code(code), type.frameType(METHOD), type.intepreted(interpreted), typeData.methodFrame.argCount(argCount),
		  typeData.methodFrame.destReg(destReg), typeData.methodFrame.bpOffset(bpOffset) {}
	
	// creates exception handler
	QuaFrame(void* code, bool interpreted, uint16_t exceptionType) : code(code), type.frameType(EXCEPTION),
		type.interpreted(interpreted), typeData.exceptionType(exceptionType) {}
	
	// creates finally or exit handler
	QuaFrame(void* code, bool interpreted, bool finally) : code(code), type.typeFrame(finally ? FINALLY : EXIT),
		type.interpreted(interpreted) {}
};

#pragma pack()
#endif

