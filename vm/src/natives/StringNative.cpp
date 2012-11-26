#include "StringNative.h"

#include <stdexcept>
#include <cstring>

#include "helpers.h"

using namespace std;

QuaValue stringDeserializer(const char* data) {
    uint16_t type = linkedTypes->at("String");
    QuaValue strRef = newInstance(type);

    uint32_t length = strlen(data);
    QuaValue blobRef = heap->allocateNew(1, length); // TODO: Resolve _DataBlob and cache its type

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
