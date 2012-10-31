#ifndef QUAMETHOD_H
#define QUAMETHOD_H

struct QuaMethod
{
public:

    enum MethodAction {
        INTERPRET = 0,
        COMPILE = 1,
        JUMPTO = 2,
        C_VOID_CALL = 3
    };

    MethodAction action;
    void* code;
};

#endif // QUAMETHOD_H
