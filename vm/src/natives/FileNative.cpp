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
        throw createException(linkedTypes->at("BadArgumentException"),
                              "Attempt to call with argument which is not an instance of String!");
    }

    return QuaValue();
}

void testClosed() {
    if(getFieldByIndex(*VMBP, 0).value & FILE_FLAG_CLOSED) {
        throw createException(typeCache.typeIOException, "File is already closed!");
    }
}

void testFailAndBad(ios* stream, const char* errStr) {
    if(stream->bad() || stream->fail()) {
        throw createException(typeCache.typeIOException, errStr);
    }
}

istream* getIStream() {
    testClosed();
    return (istream*)ptrFromQuaValues(getFieldByIndex(*VMBP, 1), getFieldByIndex(*VMBP, 2));
}

QuaValue FileNative::initNativeImpl() {
    return fileConstructor<fstream>();
}


QuaValue FileNative::readLineNativeImpl() {

    istream* thisStream = getIStream();
    string loadedString;
    std::getline(*thisStream, loadedString);
    testFailAndBad(thisStream, "Reading operation failed!");

    return stringDeserializer(loadedString.c_str());
}


QuaValue FileNative::readIntNativeImpl() {
    istream* thisStream = getIStream();
    int32_t loadedInt;
    (*thisStream) >> loadedInt;
    cout << "Loaded int: " << loadedInt << endl;
    testFailAndBad(thisStream, "Reading operation failed!");
    return createInteger(loadedInt);
}


QuaValue FileNative::readFloatNativeImpl() {
    throw runtime_error("Native method not yet implemented.");
}


QuaValue FileNative::writeLineNativeImpl() {
    testClosed();
	ostream* thisStream = (ostream*)ptrFromQuaValues(getFieldByIndex(*VMBP, 1), getFieldByIndex(*VMBP, 2));

    try {
        (*thisStream) << stringSerializer(*(VMBP + 1)) << endl;
    } catch (runtime_error& ex) {
        throw createException(linkedTypes->at("BadArgumentException"),
                              "Attempt to call with argument which is not an instance of String!");
    }

    testFailAndBad(thisStream, "Writing operation failed!");
	return QuaValue();
}


QuaValue FileNative::eofNativeImpl() {
    testClosed();
    ios* thisStream = (ios*)ptrFromQuaValues(getFieldByIndex(*VMBP, 1), getFieldByIndex(*VMBP, 2));
    if(thisStream->eof()) {
        cout << "EOF found" << endl;
        return createBool(true);
    } else {
        cout << "EOF not found" << endl;
        return createBool(false);
    }
}


QuaValue FileNative::finalizeNativeImpl() {
    if(!((getFieldByIndex(*VMBP, 0).value & FILE_FLAG_CLOSED)
            || (getFieldByIndex(*VMBP, 0).value & FILE_FLAG_UNCLOSEABLE))) {
        delete (ios*)ptrFromQuaValues(getFieldByIndex(*VMBP, 1), getFieldByIndex(*VMBP, 2));
        getFieldByIndex(*VMBP, 0).value |= FILE_FLAG_CLOSED;
    }
    return QuaValue();
}


QuaValue OutFileNative::initNativeImpl() {
    return fileConstructor<ofstream>();
}


QuaValue InFileNative::initNativeImpl() {
    return fileConstructor<ifstream>();
}



