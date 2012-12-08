#include "FileNative.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include "helpers.h"
#include "ExceptionNative.h"
#include "StringNative.h"

using namespace std;

QuaValue FileNative::initNativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue FileNative::readLineNativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue FileNative::writeLineNativeImpl() {

	if(getFieldByIndex(*VMBP, 0).value & FILE_FLAG_CLOSED) {
		throw createException(typeCache.typeIOException, "File is already closed!");
	}

	ostream* thisStream = (ostream*)ptrFromQuaValues(getFieldByIndex(*VMBP, 1), getFieldByIndex(*VMBP, 2));

    try {
        (*thisStream) << stringSerializer(*(VMBP + 1)) << endl;
    } catch (runtime_error& ex) {
        throw createException(typeCache.typeIOException,
                              "Attempt to call with argument which is not an instance of String!");
    }

    if(thisStream->bad() || thisStream->fail()) {
        throw createException(typeCache.typeIOException, "Writing operation failed!");
    }

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

