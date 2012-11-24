#include "SystemNative.h"

#include <iostream>

#include "FileNative.h"
#include "helpers.h"

using namespace std;

void SystemNative::initNativeImpl() {

    // create out stream
    QuaValue outRef = heap->allocateNew(linkedTypes->at("OutFile"), 3);

    getFieldByIndex(outRef, 0).value = FILE_FLAG_UNCLOSEABLE;
    quaValuesFromPtr(&cout, getFieldByIndex(outRef, 1), getFieldByIndex(outRef, 2));
    getFieldByIndex(*BP, 0) = outRef;

    // TODO: create err and in stream
}
