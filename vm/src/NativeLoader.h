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


inline void* ptrFromQuaValues(QuaValue first, QuaValue second) {
    return (void*)((uint64_t)first.value << 32 | (uint64_t)second.value);
}
inline void quaValuesFromPtr(void* ptr, QuaValue& first, QuaValue& second) {
    first.flags = TAG_INT;
    first.value = (uint32_t)((uint64_t)ptr >> 32);
    second.flags = TAG_INT;
    second.value = (uint32_t)((uint64_t)ptr & 0xFFFFFFFF);
}


#endif // NATIVELOADER_H
