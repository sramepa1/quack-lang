#ifndef NATIVELOADER_H
#define NATIVELOADER_H

#include <map>

#include "QuaClass.h"

class NativeLoader {

public:
    NativeLoader();
    ~NativeLoader();

    void* getNativeMethod(std::string className, QuaSignature* methodSig);
    void registerNativeMethod(std::string className, QuaSignature* methodSig, void* nativeImpl);

    QuaValue (*getClassDeserializer(std::string className))(const char*);
    void registerClassDeserializer(std::string className, QuaValue (*deserializer)(const char*));

private:

    std::map<std::string, std::map<QuaSignature*, void*, QuaSignatureComp> >* nativeMethods;
    std::map<std::string, QuaValue(*)(const char*)>* deserializers;

};

QuaValue createException(uint16_t type, const char*msg);


#endif // NATIVELOADER_H
