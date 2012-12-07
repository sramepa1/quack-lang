#include "ExceptionNative.h"

#include "StringNative.h"
#include "helpers.h"

QuaValue ExceptionNative::init1NativeImpl() {
    getFieldByIndex(*VMBP, 0) = *(VMBP + 1);
    return QuaValue();
}


QuaValue createException(uint16_t type, const char* msg) {
    QuaValue msgRef = stringDeserializer(msg);
    *(--VMSP) = msgRef;                               // Push argument for constructor
    QuaValue exRef = newRawInstance(type);
    nativeCall(exRef, (QuaSignature*)"\1initN");    // Call native constructor
    return exRef;
}
