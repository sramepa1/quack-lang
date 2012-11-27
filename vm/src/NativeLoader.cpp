#include "NativeLoader.h"

#ifdef DEBUG
#include <iostream>
#endif

#include <stdexcept>
#include <sstream>

#include "globals.h"
#include "helpers.h"

#include "FileNative.h"
#include "SystemNative.h"
#include "StringNative.h"

using namespace std;

NativeLoader::NativeLoader() {
    nativeMethods = new map<string, map<QuaSignature*, void*, QuaSignatureComp> >();
    deserializers = new map<std::string, QuaValue(*)(const char*)>();

    SystemNative(this);

    FileNative(this);
    OutFileNative(this);
    InFileNative(this);

    StringNative(this);
}

NativeLoader::~NativeLoader() {
    delete nativeMethods;
    delete deserializers;
}

void* NativeLoader::getNativeMethod(string className, QuaSignature* methodSig) {
    map<string, map<QuaSignature*, void*, QuaSignatureComp> >::iterator it = nativeMethods->find(className);
    if(it == nativeMethods->end()) {
        throw runtime_error("Class " + className + " has no native methods!");
    }
    map<QuaSignature*, void*, QuaSignatureComp>& innerMap = it->second;

    map<QuaSignature*, void*, QuaSignatureComp>::iterator innerIt = innerMap.find(methodSig);
    if(innerIt == innerMap.end()) {
        stringstream errStr;
        errStr << "No native implementation of method " << methodSig->name << " with " << (int)methodSig->argCnt
                  << " arg(s) has been found!";
        throw runtime_error(errStr.str());
    }

    return innerIt->second;
}

void NativeLoader::registerNativeMethod(string className, QuaSignature* methodSig, void* nativeImpl) {
    map<string, map<QuaSignature*, void*, QuaSignatureComp> >::iterator it = nativeMethods->find(className);
    if(it == nativeMethods->end()) {
        it = nativeMethods->insert(make_pair(className, map<QuaSignature*, void*, QuaSignatureComp>())).first;
    }
    map<QuaSignature*, void*, QuaSignatureComp>& innerMap = it->second;

    map<QuaSignature*, void*, QuaSignatureComp>::iterator innerIt = innerMap.find(methodSig);
    if(innerIt != innerMap.end()) {
        stringstream errStr;
        errStr << "Native method " << methodSig->name << " with " << methodSig->argCnt << " arg(s) of class "
               << className << " is already registered!";
        throw runtime_error(errStr.str());
    }

    innerMap.insert(make_pair(methodSig, nativeImpl));
}

QuaValue (*NativeLoader::getClassDeserializer(string className))(const char*) {
    map<std::string, QuaValue(*)(const char*)>::iterator it = deserializers->find(className);
    if(it == deserializers->end()) {
        return &QuaClass::defaultDeserializer;
    }
    return it->second;
}

void NativeLoader::registerClassDeserializer(string className, QuaValue (*deserializer)(const char *)) {
#ifdef DEBUG
    cout << "Registering deserializer of class " << className << endl;
#endif

    map<std::string, QuaValue(*)(const char*)>::iterator it = deserializers->find(className);
    if(it != deserializers->end()) {
        throw runtime_error("Class " + className + " has already registered a deserializer!");
    }
    deserializers->insert(make_pair(className, deserializer));
}

QuaValue createException(uint16_t type, const char* msg) {
    QuaValue msgRef = stringDeserializer(msg);
    *(--SP) = msgRef;                               // Push argument for constructor
    QuaValue exRef = newRawInstance(type);
    nativeCall(exRef, (QuaSignature*)"\1initN");    // Call native constructor
    return exRef;
}
