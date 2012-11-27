#include "StringNative.h"

#include <stdexcept>
#include <cstring>

#include "helpers.h"

using namespace std;

QuaValue stringDeserializer(const char* data) {
    QuaValue strRef = newRawInstance(typeCache.typeString);
    uint32_t length = strlen(data);
    QuaValue blobRef = heap->allocateNew(typeCache.typeDataBlob, length);

    for(uint32_t i = 0; i < length; i++) {
        getFieldByIndex(blobRef, i).flags = TAG_INT;
        getFieldByIndex(blobRef, i).value = (unsigned char)data[i];
    }

    getFieldByIndex(strRef, 0) = blobRef;
    getFieldByIndex(strRef, 1).flags = TAG_INT;
    getFieldByIndex(strRef, 1).value = length;

    return strRef;
}


QuaValue StringNative::init0NativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::init1NativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_opPlusNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_opIndexNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_stringValueNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::explodeNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}
