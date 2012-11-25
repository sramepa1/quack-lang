#ifndef STRINGNATIVE_H
#define STRINGNATIVE_H

#include "NativeLoader.h"

QuaValue stringDeserializer(const char* data);

class StringNative : protected NativeLoader
{
public:
    StringNative() : NativeLoader("String", 2) {

        *parent = NULL;
        *flags = CLS_STATIC;
        *deserializer = &stringDeserializer;

        // first hidden field is reference to DataBlobNative which contains the string
        fields->insert(std::make_pair("length", (uint16_t)1));

        createMethod((QuaSignature*)"\0init", __extension__ (void*)&StringNative::init0NativeImpl);
        createMethod((QuaSignature*)"\1init", __extension__ (void*)&StringNative::init1NativeImpl);
        createMethod((QuaSignature*)"\1_operatorPlus", __extension__ (void*)&StringNative::_operatorPlusNativeImpl);
        createMethod((QuaSignature*)"\1_operatorIndex", __extension__ (void*)&StringNative::_operatorIndexNativeImpl);
        createMethod((QuaSignature*)"\0_stringValue", __extension__ (void*)&StringNative::_stringValueNativeImpl);
        createMethod((QuaSignature*)"\1explode", __extension__ (void*)&StringNative::explodeNativeImpl);

    }

    static QuaValue init0NativeImpl();                  // native init()
    static QuaValue init1NativeImpl();                  // native init(string)
    static QuaValue _operatorPlusNativeImpl();          // native fun _operatorPlus(other)
    static QuaValue _operatorIndexNativeImpl();         // native fun _operatorIndex(index)
    static QuaValue _stringValueNativeImpl();           // native fun _stringValue()
    static QuaValue explodeNativeImpl();                // native fun explode(delim)

};

#endif // STRINGNATIVE_H
