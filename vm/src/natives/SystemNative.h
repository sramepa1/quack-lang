#ifndef SYSTEMNATIVE_H
#define SYSTEMNATIVE_H

#include "NativeLoader.h"

class SystemNative : protected NativeLoader {

public:
    SystemNative() : NativeLoader("System", 3) {

        *parent = NULL;
        *flags = CLS_STATIC;

        fields->insert(std::make_pair("out", (uint16_t)0));
        fields->insert(std::make_pair("in", (uint16_t)1));
        fields->insert(std::make_pair("err", (uint16_t)2));

        createMethod((QuaSignature*)"\0init", __extension__ (void*)&SystemNative::initNativeImpl);

    }

    static QuaValue initNativeImpl();       // native init()

};


#endif // SYSTEMNATIVE_H
