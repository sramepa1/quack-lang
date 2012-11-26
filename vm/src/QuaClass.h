#ifndef QUACLASS_H
#define QUACLASS_H

extern "C" {
    #include <stdint.h>
}

#include <map>
#include <string>

#include "QuaSignature.h"
#include "QuaMethod.h"
#include "QuaValue.h"

#define CLS_STATIC 0x1;
#define CLS_DESTRUCTIBLE 0x2;

#define CLS_VARIABLE_LENGTH 0x4000
#define CLS_INCONSTRUCTIBLE 0x8000

class Loader;

class FieldNameComparator {
public:
    bool operator()(const char* first, const char* second) const { return std::strcmp(first, second) < 0; }
};

class QuaClass
{
public:
    ~QuaClass();

    bool isStatic() { return flags & (uint16_t)CLS_STATIC; }
    bool isInstanceOf(QuaClass* ancestorClass) {
        return ancestorClass == this ? true : ( parent == NULL ? false : parent->isInstanceOf(ancestorClass) );
    }
    uint16_t getFieldCount();
    void* getCP() { return relevantCP; }
    void* getCT() { return relevantCT; }
    QuaMethod* lookupMethod(QuaSignature* sig);
    uint16_t lookupFieldIndex(const char* fieldName);
    QuaValue getInstance();
    void setInstance(QuaValue newInstance);
    const char* getName();

    QuaValue deserialize(const char* data) { return deserializer(data); }

private:
    // !!! defined in Loader.cpp !!!
    QuaClass(void* constantPool, void* classDef, const std::string& className, void* clsTabPtr);
    friend class Loader;

    QuaValue (*deserializer)(const char* data);
    static QuaValue defaultDeserializer(const char* data);

    std::string className;
    QuaClass* parent;
    void* relevantCP;
    void* relevantCT;
    std::map<const char*, uint16_t, FieldNameComparator> fieldIndices;
    std::map<QuaSignature*, QuaMethod*, QuaSignatureComp> methods;
    uint16_t myFieldCount;
    uint16_t flags;
    QuaValue instance; // default "null"
};

#endif // QUACLASS_H
