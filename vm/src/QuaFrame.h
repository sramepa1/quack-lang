#ifndef _QUA_FRAME_H_ 
#define _QUA_FRAME_H_

extern "C" {
    #include <stdint.h>
}

#pragma pack(1)

#define EXIT 0
#define METHOD 1
#define EXCEPTION 2
#define FINALLY 3

#define FRAME_TYPE type.frameType
#define INTERPRETED type.interpreted

#define ARG_COUNT typeData.methodFrame.argCount
#define DEST_REG typeData.methodFrame.destReg
#define BP_OFFSET typeData.methodFrame.bpOffset

#define EXCEPTION_TYPE typeData.exceptionType

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
    QuaFrame(void* code, bool interpreted, char argCount, uint16_t destReg, uint32_t bpOffset) : code(code) {
        FRAME_TYPE = METHOD;
        INTERPRETED = interpreted;
        ARG_COUNT = argCount;
        DEST_REG = destReg;
        BP_OFFSET = bpOffset;
    }

    // creates exception handler
    QuaFrame(void* code, bool interpreted, uint16_t exceptionType) : code(code) {
        FRAME_TYPE = EXCEPTION;
        INTERPRETED = interpreted;
        EXCEPTION_TYPE = exceptionType;
    }

    // creates finally or exit handler (it depends on third parameter)
    QuaFrame(void* code, bool interpreted, bool finally) : code(code) {
        FRAME_TYPE = finally ? FINALLY : EXIT;
        INTERPRETED = interpreted;
    }

};

#pragma pack()
#endif

