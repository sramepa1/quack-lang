#ifndef EXCEPTIONNATIVE_H
#define EXCEPTIONNATIVE_H

#include "NativeLoader.h"

class ExceptionNative
{
    const char* const name;

public:
    ExceptionNative(NativeLoader* loader) : name("Exception") {

        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                          __extension__ (void*)&ExceptionNative::init1NativeImpl);

    }

    static QuaValue init1NativeImpl();                  // native initN(cause)
};

QuaValue createException(uint16_t type, const char*msg);

#endif // EXCEPTIONNATIVE_H
