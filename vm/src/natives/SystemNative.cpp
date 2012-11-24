#include "SystemNative.h"

#include <iostream>

#include "FileNative.h"
#include "helpers.h"

using namespace std;

QuaValue SystemNative::initNativeImpl() {

    // create out stream
    uint16_t type = linkedTypes->at("OutFile");
    QuaValue outRef = heap->allocateNew(type, resolveType(type)->getFieldCount());

    getFieldByIndex(outRef, 0).value = FILE_FLAG_UNCLOSEABLE;
    quaValuesFromPtr(&cout, getFieldByIndex(outRef, 1), getFieldByIndex(outRef, 2));
    getFieldByIndex(*BP, 0) = outRef;

    // TODO: create err and in stream
    return QuaValue();
}
