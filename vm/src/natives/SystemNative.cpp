#include "SystemNative.h"

#include <iostream>

#include "FileNative.h"
#include "helpers.h"

using namespace std;

QuaValue SystemNative::initNativeImpl() {

    // TODO: refactor it

    // create in stream
    QuaValue inRef = newRawInstance(linkedTypes->at("InFile"));

    getFieldByIndex(inRef, 0) = createInteger(FILE_FLAG_UNCLOSEABLE);
    quaValuesFromPtr(&cout, getFieldByIndex(inRef, 1), getFieldByIndex(inRef, 2));
    getFieldByIndex(*VMBP, 0) = inRef;

    // create out stream
    QuaValue outRef = newRawInstance(linkedTypes->at("OutFile"));

    getFieldByIndex(outRef, 0) = createInteger(FILE_FLAG_UNCLOSEABLE);
    quaValuesFromPtr(&cout, getFieldByIndex(outRef, 1), getFieldByIndex(outRef, 2));
    getFieldByIndex(*VMBP, 1) = outRef;

    // create err stream
    QuaValue errRef = newRawInstance(linkedTypes->at("OutFile"));

    getFieldByIndex(errRef, 0) = createInteger(FILE_FLAG_UNCLOSEABLE);
    quaValuesFromPtr(&cout, getFieldByIndex(errRef, 1), getFieldByIndex(errRef, 2));
    getFieldByIndex(*VMBP, 2) = errRef;

    return QuaValue();
}
