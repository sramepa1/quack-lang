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
        loader->registerNativeMethod(name, (QuaSignature*)"\2setElemN",
                          __extension__ (void*)&ArrayNative::setElemNativeImpl);
    }

    static QuaValue init1NativeImpl();                  // native initN(size)
    static QuaValue _opIndexNativeImpl();               // native fun _opIndexN(index)
    static QuaValue setElemNativeImpl();                // native fun setElemN(index, elem)

};

#endif // ARRAYNATIVE_H
