#ifndef BOOLNATIVE_H
#define BOOLNATIVE_H

#include "NativeLoader.h"

class BoolNative
{
    const char* const name;

public:
    BoolNative(NativeLoader* loader) : name("Bool") {

        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                                     __extension__ (void*)&BoolNative::init1NativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\0_stringValue",
                                     __extension__ (void*)&BoolNative::_stringValueNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0_boolValue",
                                     __extension__ (void*)&BoolNative::_boolValueNativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\1_opEqN",
                                     __extension__ (void*)&BoolNative::_opEqNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opNeqN",
                                     __extension__ (void*)&BoolNative::_opNeqNativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\1_opLAndN",
                                     __extension__ (void*)&BoolNative::_opLAndNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opLOrN",
                                     __extension__ (void*)&BoolNative::_opLOrNativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\0_opLNot",
                                     __extension__ (void*)&BoolNative::_opLNotNativeImpl);
    }

    static QuaValue init1NativeImpl();                  // native fun initN(value)

    static QuaValue _stringValueNativeImpl();           // native fun _stringValue()
    static QuaValue _boolValueNativeImpl();             // native fun _boolValue()

    static QuaValue _opEqNativeImpl();                  // native fun _opEqN(secOperand)
    static QuaValue _opNeqNativeImpl();                 // native fun _opNeqN(secOperand)

    static QuaValue _opLAndNativeImpl();                // native fun _opLAndN(secOperand)
    static QuaValue _opLOrNativeImpl();                 // native fun _opLOrN(secOperand)

    static QuaValue _opLNotNativeImpl();                // native fun _opLNot()
};

#endif // BOOLNATIVE_H
