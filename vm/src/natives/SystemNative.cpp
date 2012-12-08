#include "SystemNative.h"

#include <iostream>

#include "FileNative.h"
#include "helpers.h"

using namespace std;

QuaValue SystemNative::initNativeImpl() {

    // create out stream
    QuaValue outRef = newRawInstance(linkedTypes->at("OutFile"));

    getFieldByIndex(outRef, 0) = createInteger(FILE_FLAG_UNCLOSEABLE /*| FILE_FLAG_CLOSED*/);
    quaValuesFromPtr(&cout, getFieldByIndex(outRef, 1), getFieldByIndex(outRef, 2));
    getFieldByIndex(*VMBP, 1) = outRef;

    // TODO: create err and in stream
    return QuaValue();
}
