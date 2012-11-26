#ifndef STRINGNATIVE_H
#define STRINGNATIVE_H

#include "NativeLoader.h"

QuaValue stringDeserializer(const char* data);

class StringNative
{
    const char* const name;

public:
    StringNative(NativeLoader* loader) : name("String") {
        loader->registerClassDeserializer(name, &stringDeserializer);

        loader->registerNativeMethod(name, (QuaSignature*)"\0init",
                          __extension__ (void*)&StringNative::init0NativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                          __extension__ (void*)&StringNative::init1NativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opPlusN",
                          __extension__ (void*)&StringNative::_opPlusNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opIndexN",
                          __extension__ (void*)&StringNative::_opIndexNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0_stringValue",
                          __extension__ (void*)&StringNative::_stringValueNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1explodeN",
                          __extension__ (void*)&StringNative::explodeNativeImpl);
    }

    static QuaValue init0NativeImpl();                  // native init()
    static QuaValue init1NativeImpl();                  // native init(string)
    static QuaValue _opPlusNativeImpl();                // native fun _opPlusN(other)
    static QuaValue _opIndexNativeImpl();               // native fun _opIndexN(index)
    static QuaValue _stringValueNativeImpl();           // native fun _stringValue()
    static QuaValue explodeNativeImpl();                // native fun explode(delim)

};

#endif // STRINGNATIVE_H
