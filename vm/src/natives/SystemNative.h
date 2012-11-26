#ifndef SYSTEMNATIVE_H
#define SYSTEMNATIVE_H

#include "NativeLoader.h"

class SystemNative {

    const char* const name;

public:
    SystemNative(NativeLoader* loader) : name("System") {
        loader->registerNativeMethod(name, (QuaSignature*)"\0init",
                          __extension__ (void*)&SystemNative::initNativeImpl);
    }

    static QuaValue initNativeImpl();       // native init()

};


#endif // SYSTEMNATIVE_H
