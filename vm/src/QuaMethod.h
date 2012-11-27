#ifndef QUAMETHOD_H
#define QUAMETHOD_H

extern "C" {
    #include <stdint.h>
}

struct QuaMethod
{
public:

    enum MethodAction {
        INTERPRET = 0,
        COMPILE = 1,
        JUMPTO = 2,
        C_CALL = 3
    } action;
    void* code;
    uint16_t insnCount;

};

#endif // QUAMETHOD_H
