#ifndef INTEGERNATIVE_H
#define INTEGERNATIVE_H

#include "NativeLoader.h"

class IntegerNative
{
    const char* const name;

public:
    IntegerNative(NativeLoader* loader) : name("Integer") {

        loader->registerNativeMethod(name, (QuaSignature*)"\1initN",
                          __extension__ (void*)&IntegerNative::init1NativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\0_intValue",
                          __extension__ (void*)&IntegerNative::_intValueNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0_floatValue",
                          __extension__ (void*)&IntegerNative::_floatValueNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0_stringValue",
                          __extension__ (void*)&IntegerNative::_stringValueNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\0_boolValue",
                          __extension__ (void*)&IntegerNative::_boolValueNativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\1_opPlusN",
                          __extension__ (void*)&IntegerNative::_opPlusNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opMinusN",
                          __extension__ (void*)&IntegerNative::_opMinusNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opMulN",
                          __extension__ (void*)&IntegerNative::_opMulNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opDivN",
                          __extension__ (void*)&IntegerNative::_opDivNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opModN",
                          __extension__ (void*)&IntegerNative::_opModNativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\0_opUnMinus",
                          __extension__ (void*)&IntegerNative::_opUnMinusNativeImpl);

        loader->registerNativeMethod(name, (QuaSignature*)"\1_opEqN",
                          __extension__ (void*)&IntegerNative::_opEqNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opNeqN",
                          __extension__ (void*)&IntegerNative::_opNeqNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opGtN",
                          __extension__ (void*)&IntegerNative::_opGtNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opLtN",
                          __extension__ (void*)&IntegerNative::_opLtNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opGeN",
                          __extension__ (void*)&IntegerNative::_opGeNativeImpl);
        loader->registerNativeMethod(name, (QuaSignature*)"\1_opLeN",
                          __extension__ (void*)&IntegerNative::_opLeNativeImpl);

    }


    static QuaValue init1NativeImpl();                  // native fun initN(value)

    static QuaValue _intValueNativeImpl();              // native fun _intValue()
    static QuaValue _floatValueNativeImpl();            // native fun _floatValue()
    static QuaValue _stringValueNativeImpl();           // native fun _stringValue()
    static QuaValue _boolValueNativeImpl();             // native fun _boolValue()

    static QuaValue _opPlusNativeImpl();                // native fun _opPlusN(secOperand)
    static QuaValue _opMinusNativeImpl();               // native fun _opMinusN(secOperand)
    static QuaValue _opMulNativeImpl();                 // native fun _opMulN(secOperand)
    static QuaValue _opDivNativeImpl();                 // native fun _opDivN(secOperand)
    static QuaValue _opModNativeImpl();                 // native fun _opModN(secOperand)

    static QuaValue _opUnMinusNativeImpl();             // native fun _opUnMinus()

    static QuaValue _opEqNativeImpl();                  // native fun _opEqN(secOperand)
    static QuaValue _opNeqNativeImpl();                 // native fun _opNeqN(secOperand)
    static QuaValue _opGtNativeImpl();                  // native fun _opGtN(secOperand)
    static QuaValue _opLtNativeImpl();                  // native fun _opLtN(secOperand)
    static QuaValue _opGeNativeImpl();                  // native fun _opGeN(secOperand)
    static QuaValue _opLeNativeImpl();                  // native fun _opLeN(secOperand)

};

#endif // INTEGERNATIVE_H
