#include "BoolNative.h"

#include <stdexcept>
#include <sstream>

#include "helpers.h"
#include "ExceptionNative.h"
#include "StringNative.h"

using namespace std;

bool boolSerializer(QuaValue val) {
    if(val.type != typeCache.typeBool) {
        throw runtime_error("Attempted to serialize a non-Bool as a Bool.");
    }

    if(val.tag == TAG_BOOL) {
        return (bool)val.value;
    } else {
        return (bool)getFieldByIndex(val, 0).value;
    }
}

QuaValue BoolNative::init1NativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue BoolNative::_stringValueNativeImpl() {
    stringstream convert;
    convert << boolalpha << boolSerializer(*VMBP);
    return stringDeserializer(convert.str().c_str());
}


QuaValue BoolNative::_boolValueNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


#define BOOL_OPERATOR_IMPL(_OPER)                                                                       \
try {                                                                                                   \
    bool first = boolSerializer(*VMBP);                                                                 \
    bool second = boolSerializer(*(VMBP + 1));                                                          \
    return createBool(first _OPER second);                                                              \
} catch (runtime_error& ex) {                                                                           \
    throw createException(linkedTypes->at("BadArgumentException"),                                      \
                          "Attempt to call with argument which is not an instance of Bool!");           \
}                                                                                                       \
return QuaValue();


QuaValue BoolNative::_opEqNativeImpl() {
    BOOL_OPERATOR_IMPL(==)
}


QuaValue BoolNative::_opNeqNativeImpl() {
    BOOL_OPERATOR_IMPL(!=)
}


QuaValue BoolNative::_opLAndNativeImpl() {
    BOOL_OPERATOR_IMPL(&&)
}


QuaValue BoolNative::_opLOrNativeImpl() {
    BOOL_OPERATOR_IMPL(||)
}


QuaValue BoolNative::_opLNotNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}
