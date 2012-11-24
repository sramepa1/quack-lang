#include "FileNative.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include "helpers.h"

using namespace std;

void FileNative::initNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void FileNative::readLineNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


void FileNative::writeLineNativeImpl() {

    if(getFieldByIndex(*BP, 0).value & FILE_FLAG_CLOSED) {
        // TODO: throw IOException - file is already closed
        return;
    }

    ostream* thisStream = (ostream*)ptrFromQuaValues(getFieldByIndex(*BP, 1), getFieldByIndex(*BP, 2));

    // TODO: test if arg is string... if not, call stringValue
    uint32_t stringLength = getFieldByIndex(*(BP + 1), 0).value;
    string buffer;
    buffer.reserve(stringLength);

    QuaValue stringRef = getFieldByIndex(*(BP + 1), 1);
    for(uint32_t i = 0; i < stringLength; i++) {
        buffer.append(1, (unsigned char)(getFieldByIndex(stringRef, i).value));
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
