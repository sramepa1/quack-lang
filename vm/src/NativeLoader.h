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

        fields = &(nativeClass->fieldIndices);
        methods = &(nativeClass->methods);

        nativeClass->relevantCP = NULL; // ???
        nativeClass->myFieldCount = fieldCount;

        typeArray[linkedTypes->size()] = nativeClass;
        linkedTypes->insert(make_pair(name, linkedTypes->size()));

#ifdef DEBUG
        std::cout << "Native class: " << name << " registered!" << std::endl;
#endif
    }

    void createMethod(QuaSignature* signature, void* code) {
        QuaMethod* method = new QuaMethod();
        method->action = QuaMethod::C_VOID_CALL;
        method->code = code;
        methods->insert(std::make_pair(signature, method));
    }

    QuaClass** parent;
    uint16_t* flags;

    std::map<std::string, uint16_t>* fields;
    std::map<QuaSignature*, QuaMethod*, QuaSignatureComp>* methods;

};

// WARNING: This native QuaClass is only used as helper data storage for arrays or strings. It cannot be created
//          by calling of "new" instruction!
class DataBlobNative : protected NativeLoader {
public:
    DataBlobNative() : NativeLoader("", 0) {}
};

#endif // NATIVELOADER_H
