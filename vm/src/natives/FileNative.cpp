#include "FileNative.h"

#include <iostream>
#include <fstream>
#include <stdexcept>

#include "helpers.h"
#include "ExceptionNative.h"
#include "StringNative.h"

using namespace std;

template <typename streamType>
QuaValue fileConstructor() {
    try {
        string fileName = stringSerializer(*(VMBP + 1));

        streamType* newFile = new streamType(fileName.c_str());

        if(newFile->bad() || newFile->fail()) {
            throw createException(typeCache.typeIOException, string("File " + fileName + " cannot be opened!").c_str());
        }

        getFieldByIndex(*VMBP, 0) = createInteger(0); // no flags
        quaValuesFromPtr(newFile, getFieldByIndex(*VMBP, 1), getFieldByIndex(*VMBP, 2));

    } catch (runtime_error& ex) {
        throw createException(typeCache.typeIOException,
                              "Attempt to call with argument which is not an instance of String!");
    }

    return QuaValue();
}

QuaValue FileNative::initNativeImpl() {
    return fileConstructor<fstream>();
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
    return fileConstructor<ofstream>();
}


QuaValue InFileNative::initNativeImpl() {
    return fileConstructor<ifstream>();
}

