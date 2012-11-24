#include "SystemNative.h"

#include <iostream>

#include "FileNative.h"

using namespace std;

void quaValuesFromPtr(void* ptr, QuaValue& first, QuaValue& second) {
    first.value = (uint32_t)((uint64_t)ptr >> 32);
    second.value = (uint32_t)((uint64_t)ptr & 0xFFFFFFFF);
}

void SystemNative::initNativeImpl() {

    QuaObject* thisPtr = heap->dereference(*BP).instance;

    // create out stream
    QuaValue outRef = heap->allocateNew(linkedTypes->at("OutFile"), 3);
    QuaObject* outInstance = heap->dereference(outRef).instance;

    ((QuaValue*)outInstance)[0].value = FILE_FLAG_UNCLOSEABLE;
    quaValuesFromPtr(&cout, ((QuaValue*)outInstance)[1], ((QuaValue*)outInstance)[2]);
    ((QuaValue*)thisPtr)[0] = outRef;

    // TODO: create err and in stream
}
