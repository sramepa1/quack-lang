#include "ArrayNative.h"

#include <stdexcept>

#include "helpers.h"
#include "ExceptionNative.h"
#include "IntegerNative.h"

using namespace std;


void checkIndex(int32_t index) {
    int32_t arraySize = heap->dereference(getFieldByIndex(*VMBP, 0)).instanceCount;

    if(index >= arraySize) {
        stringstream errStr;
        errStr << "Attempt to acces index number " << index << " but max index is " << (arraySize - 1);
        throw createException(linkedTypes->at("ArrayIndexOfBoundException"), errStr.str().c_str());
    }
}

QuaValue ArrayNative::init1NativeImpl() {

    int32_t arrayLength = integerSerializer(*(VMBP + 1));
    QuaValue blobRef = heap->allocateNew(typeCache.typeDataBlob, arrayLength);
    getFieldByIndex(*VMBP, 0) = blobRef;
    getFieldByIndex(*VMBP, 1) = createInteger(arrayLength);

    return QuaValue();
}


QuaValue ArrayNative::_opIndexNativeImpl() {
    int32_t index = integerSerializer(*(VMBP + 1));
    checkIndex(index);
    return heap->dereference(getFieldByIndex(*VMBP, 0)).instance->fields[index];
}


QuaValue ArrayNative::_opIndexWNativeImpl() {
    int32_t index = integerSerializer(*(VMBP + 1));
    checkIndex(index);
    heap->dereference(getFieldByIndex(*VMBP, 0)).instance->fields[index] = *(VMBP + 2);
    return QuaValue();
}
