#ifndef NATIVELOADER_H
#define NATIVELOADER_H

#ifdef DEBUG
#include <iostream>
#endif

#include "globals.h"

#include "QuaClass.h"

class NativeLoader {

protected:

    NativeLoader(std::string name, uint16_t fieldCount) {
#ifdef DEBUG
        std::cout << "Creating native class: " << name << std::endl;
#endif
        QuaClass* nativeClass = new QuaClass();

        parent = &(nativeClass->parent);
        flags = &(nativeClass->flags);
        deserializer = &(nativeClass->deserializer);

        fields = &(nativeClass->fieldIndices);
        methods = &(nativeClass->methods);

        nativeClass->className = name;
        nativeClass->relevantCP = NULL; // This is OK as long as native classes don't have bytecode accessing these
        nativeClass->relevantCT = NULL;
        nativeClass->myFieldCount = fieldCount;

        typeArray[linkedTypes->size()] = nativeClass;
        linkedTypes->insert(make_pair(name, linkedTypes->size()));

#ifdef DEBUG
        std::cout << "Native class: " << name << " registered!" << std::endl;
#endif
    }

    void createMethod(QuaSignature* signature, void* code) {
        QuaMethod* method = new QuaMethod();
        method->action = QuaMethod::C_CALL;
        method->code = code;
        methods->insert(std::make_pair(signature, method));
    }

    QuaClass** parent;
    uint16_t* flags;
    QuaValue (**deserializer)(const char* data);

    std::map<const char*, uint16_t, FieldNameComparator>* fields;
    std::map<QuaSignature*, QuaMethod*, QuaSignatureComp>* methods;

};

// WARNING: This native QuaClass is only used as helper data storage for arrays or strings. It cannot be created
//          by calling of "new" instruction!
class DataBlobNative : protected NativeLoader {
public:
    DataBlobNative() : NativeLoader("", 0) {}
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
