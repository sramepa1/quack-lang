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
        loader->registerNativeMethod(name, (QuaSignature*)"\1init",
                          __extension__ (void*)&StringNative::init1NativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_operatorPlus",
                          __extension__ (void*)&StringNative::_operatorPlusNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_operatorIndex",
                          __extension__ (void*)&StringNative::_operatorIndexNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0_stringValue",
                          __extension__ (void*)&StringNative::_stringValueNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1explode",
                          __extension__ (void*)&StringNative::explodeNativeImpl);
    }

    static QuaValue init0NativeImpl();                  // native init()
    static QuaValue init1NativeImpl();                  // native init(string)
    static QuaValue _operatorPlusNativeImpl();          // native fun _operatorPlus(other)
    static QuaValue _operatorIndexNativeImpl();         // native fun _operatorIndex(index)
    static QuaValue _stringValueNativeImpl();           // native fun _stringValue()
    static QuaValue explodeNativeImpl();                // native fun explode(delim)

};

#endif // STRINGNATIVE_H
