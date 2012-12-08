#ifndef ARRAYNATIVE_H
#define ARRAYNATIVE_H

#include "NativeLoader.h"

class ArrayNative
{
    const char* const name;

public:
    ArrayNative(NativeLoader* loader) : name("Array") {

        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                          __extension__ (void*)&ArrayNative::init1NativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opIndexN",
                          __extension__ (void*)&ArrayNative::_opIndexNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\2_opIndexWN",
                          __extension__ (void*)&ArrayNative::_opIndexWNativeImpl);
    }

    static QuaValue init1NativeImpl();                  // native initN(size)
    static QuaValue _opIndexNativeImpl();               // native fun _opIndexN(index)
    static QuaValue _opIndexWNativeImpl();              // native fun _opIndexWN(index, elem)

};

#endif // ARRAYNATIVE_H
