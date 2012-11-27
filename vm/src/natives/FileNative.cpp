#include "FileNative.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include "helpers.h"

using namespace std;

QuaValue FileNative::initNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue FileNative::readLineNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue FileNative::writeLineNativeImpl() {

    if(getFieldByIndex(*BP, 0).value & FILE_FLAG_CLOSED) {
        throw createException(typeCache.typeIOException, "File is already closed!");
    }

    ostream* thisStream = (ostream*)ptrFromQuaValues(getFieldByIndex(*BP, 1), getFieldByIndex(*BP, 2));

    // TODO: test if arg is string... if not, call stringValue
    uint32_t stringLength = getFieldByIndex(*(BP + 1), 1).value;
    string buffer;
    buffer.reserve(stringLength);

    QuaValue stringRef = getFieldByIndex(*(BP + 1), 0);
    for(uint32_t i = 0; i < stringLength; i++) {
        buffer.append(1, (unsigned char)(getFieldByIndex(stringRef, i).value));
    }

    // TODO: test stream fail and bad bit
    (*thisStream) << buffer << endl;
    return QuaValue();
}


QuaValue FileNative::eofNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue FileNative::finalizeNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue OutFileNative::initNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue InFileNative::initNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}

