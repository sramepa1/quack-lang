#include "IntegerNative.h"

#include <stdexcept>
#include <sstream>

#include "helpers.h"
#include "StringNative.h"
#include "ExceptionNative.h"

using namespace std;

int32_t integerSerializer(QuaValue val) {
    if(val.type != typeCache.typeInteger) {
        throw runtime_error("Attempted to serialize a non-Integer as a Integer.");
    }

    if(val.tag == TAG_INT) {
        return val.value;
    } else {
        return getFieldByIndex(val, 0).value;
    }
}

QuaValue IntegerNative::init1NativeImpl() {
    try {
        getFieldByIndex(*VMBP, 0) = createInteger(integerSerializer(*(VMBP + 1)));
    } catch (runtime_error& ex) {
        throw createException(linkedTypes->at("BadArgumentException"),
                              "Attempt to call with argument which is not an instance of Integer!");
    }
    return QuaValue();
}


QuaValue IntegerNative::_intValueNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue IntegerNative::_floatValueNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue IntegerNative::_stringValueNativeImpl() {
    stringstream convert;
    convert << (*VMBP).value;
    //convert << getFieldByIndex(*VMBP, 0).value;
    return stringDeserializer(convert.str().c_str());
}


QuaValue IntegerNative::_boolValueNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}

// This macro is helper for generating source code for binary operators

#define OPERATOR_IMPL(_RESULT)                                                                          \
try {                                                                                                   \
    int32_t first = getFieldByIndex(*VMBP, 0).value;                                                    \
    int32_t second = integerSerializer(*(VMBP + 1));                                                    \
    _RESULT                                                                                             \
} catch (runtime_error& ex) {                                                                           \
    throw createException(linkedTypes->at("BadArgumentException"),                                      \
                          "Attempt to call with argument which is not an instance of Integer!");        \
}                                                                                                       \
return QuaValue();                                                                                      \


/////////////////////////////////////////////////////////////
//// Arithmetic operators

#define ARR_RESULT(_OPER)                                                                           \
QuaValue resultRef = newRawInstance(typeCache.typeInteger);                                         \
getFieldByIndex(resultRef, 0).value = first _OPER second;                                           \
return resultRef;                                                                                   \



QuaValue IntegerNative::_opPlusNativeImpl() {
    OPERATOR_IMPL(ARR_RESULT(+))
}


QuaValue IntegerNative::_opMinusNativeImpl() {
    OPERATOR_IMPL(ARR_RESULT(-))
}


QuaValue IntegerNative::_opMulNativeImpl() {
    OPERATOR_IMPL(ARR_RESULT(*))
}


QuaValue IntegerNative::_opDivNativeImpl() {
    OPERATOR_IMPL(ARR_RESULT(/))
}


QuaValue IntegerNative::_opModNativeImpl() {
    OPERATOR_IMPL(ARR_RESULT(%))
}


QuaValue IntegerNative::_opUnMinusNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


/////////////////////////////////////////////////////////////
//// Relation operators

#define REL_RESULT(_OPER)                                                                           \
return createBool(first _OPER second);                                                              \


QuaValue IntegerNative::_opEqNativeImpl() {
    OPERATOR_IMPL(REL_RESULT(==))
}


QuaValue IntegerNative::_opNeqNativeImpl() {
    OPERATOR_IMPL(REL_RESULT(!=))
}


QuaValue IntegerNative::_opGtNativeImpl() {
    OPERATOR_IMPL(REL_RESULT(>))
}


QuaValue IntegerNative::_opLtNativeImpl() {
    OPERATOR_IMPL(REL_RESULT(<))
}


QuaValue IntegerNative::_opGeNativeImpl() {
    OPERATOR_IMPL(REL_RESULT(>=))
}


QuaValue IntegerNative::_opLeNativeImpl() {
    OPERATOR_IMPL(REL_RESULT(<=))
}
