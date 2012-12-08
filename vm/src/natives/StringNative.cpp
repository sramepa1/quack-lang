#include "StringNative.h"

#include <stdexcept>
#include <cstring>

#include "helpers.h"
#include "ExceptionNative.h"

#include <iostream>

using namespace std;

QuaValue stringDeserializer(const char* data) {
	QuaValue strRef = newRawInstance(typeCache.typeString);
	uint32_t length = strlen(data);
	QuaValue blobRef = heap->allocateNew(typeCache.typeDataBlob, length);

	for(uint32_t i = 0; i < length; i++) {
        getFieldByIndex(blobRef, i) = createInteger((unsigned char)data[i]);
	}

	getFieldByIndex(strRef, 0) = blobRef;
    getFieldByIndex(strRef, 1) = createInteger(length);

	return strRef;
}

string stringSerializer(QuaValue val) {
	if(val.type != typeCache.typeString) {
		throw runtime_error("Attempted to serialize a non-String as a String.");
	}

    uint32_t stringLength = getFieldByIndex(val, 1).value;
    string buffer;
    buffer.reserve(stringLength);

    QuaValue stringRef = getFieldByIndex(val, 0);
    for(uint32_t i = 0; i < stringLength; i++) {
        buffer.append(1, (unsigned char)(getFieldByIndex(stringRef, i).value));
    }

    return buffer;
}


QuaValue StringNative::init0NativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::init1NativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_opPlusNativeImpl() {
    try {
        string first = stringSerializer(*VMBP);
        string second = stringSerializer(*(VMBP + 1));

        return stringDeserializer((first + second).c_str());
    } catch (runtime_error& ex) {
        throw createException(linkedTypes->at("BadArgumentException"),
                              "Attempt to call with argument which is not an instance of String!");
    }
    return QuaValue();
}


QuaValue StringNative::_opIndexNativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_stringValueNativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_intValueNativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::_floatValueNativeImpl() {
	throw runtime_error("Native method not yet implemented.");
}


QuaValue StringNative::explodeNativeImpl() {

    // TODO: does not work yet
    const char* delim = NULL;

    try {
        delim = stringSerializer(*(VMBP + 1)).c_str();
    } catch (runtime_error& ex) {
        throw createException(typeCache.typeIOException,
                              "Attempt to call with argument which is not an instance of String!");
    }

    string me = stringSerializer(*VMBP);
    char* myCopy = new char[me.size() + 1];
    strncpy(myCopy, me.c_str(), me.size() + 1);
    vector<QuaValue> stringRefs;

    char* token;
    token = strtok(myCopy, delim);

    while(token != NULL) {
        stringRefs.push_back(stringDeserializer(token));
        token = strtok(NULL, delim);
    }

    QuaValue tokenCount = createInteger(stringRefs.size());
    *(--VMSP) = tokenCount;
    QuaValue arrRef = newRawInstance(typeCache.typeArray);
    nativeCall(arrRef, (QuaSignature*)"\1initN");

    uint32_t index = 0;
    for(vector<QuaValue>::iterator it = stringRefs.begin(); it != stringRefs.end(); ++it) {

        *(--VMSP) = (*it);

        QuaValue indexRef = createInteger(index);
        *(--VMSP) = indexRef;

        nativeCall(arrRef, (QuaSignature*)"\2_opIndexWN");
        index++;
    }

    delete[] myCopy;
    return arrRef;
}
