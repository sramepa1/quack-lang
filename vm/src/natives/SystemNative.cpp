#include "SystemNative.h"

void SystemNative::initNativeImpl() {

    // create out stream
    QuaValue outRef = heap->allocateNew(linkedTypes->at("OutFile"), 3);
    QuaObject* outInstance = heap->getObjRecord(outRef.value).instance;

    ((QuaValue*)outInstance)[0].value = 2;      // FILE_FLAG_UNCLOSEABLE

}

