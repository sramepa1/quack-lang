#ifndef QUACLASS_H
#define QUACLASS_H

extern "C" {
    #include <stdint.h>
}

#include <map>
#include <string>

#include "QuaSignature.h"
#include "QuaMethod.h"

#define CLS_STATIC 1;

class Loader;

class QuaClass
{
public:

    ~QuaClass();

    bool isStatic() { return flags & (uint16_t)CLS_STATIC; }
    uint16_t getFieldCount();
    QuaMethod* lookupMethod(QuaSignature* sig);
    uint16_t lookupFieldIndex(std::string fieldName);

private:
    // !!! defined in Loader.cpp !!!
    QuaClass(void* constantPool, void* classDef, const std::string& className);
    friend class Loader;

    QuaClass* parent;
    void* relevantCP;
    std::map<std::string, uint16_t> fieldIndices;
    std::map<QuaSignature*, QuaMethod*, QuaSignatureComp> methods;
    uint16_t myFieldCount;
    uint16_t flags;
};

#endif // QUACLASS_H