#include "FileNative.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;

void* ptrFromQuaValues(QuaValue& first, QuaValue& second) {
    return (void*)((uint64_t)first.value << 32 | (uint64_t)second.value);
}

void FileNative::initNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void FileNative::readLineNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void FileNative::writeLineNativeImpl() {
    QuaObject* thisPtr = heap->dereference(*BP).instance;
    if(((QuaValue*)thisPtr)[1].value & FILE_FLAG_CLOSED) {
        // TODO: throw IOException - file is already closed
        return;
    }

    ostream* thisStream = (ostream*)ptrFromQuaValues(((QuaValue*)thisPtr)[1], ((QuaValue*)thisPtr)[2]);

    QuaObject* argPtr = heap->dereference(*(BP + 1)).instance;
    // TODO: test if arg is string... if not, call stringValue
    uint32_t stringLength = ((QuaValue*)argPtr)[0].value;
    string buffer;
    buffer.reserve(stringLength);

    QuaObject* stringData = heap->dereference(((QuaValue*)argPtr)[1]).instance;
    for(uint32_t i = 0; i < stringLength; i++) {
        buffer.append(1, (unsigned char)(((QuaValue*)stringData)[i].value));
    }

    // TODO: test stream fail and bad bit
    (*thisStream) << buffer << endl;
}


void FileNative::eofNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void FileNative::closeNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void OutFileNative::initNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void OutFileNative::readLineNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void InFileNative::initNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void InFileNative::writeLineNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}
