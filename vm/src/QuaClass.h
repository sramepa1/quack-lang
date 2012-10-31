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

class QuaClass
{
public:
    QuaClass(void* constantPool, void* classDef);

    bool isStatic() { return flags & (uint16_t)CLS_STATIC; }
    uint32_t getFieldCount();
    QuaMethod* lookupMethod(QuaSignature* sig);
    uint32_t lookupFieldIndex(std::string fieldName);


private:
    QuaClass* parent;
    void* relevantCP;
    std::map<std::string, uint32_t> fieldIndices;
    std::map<QuaSignature*, QuaMethod*> methods;
    uint32_t myFieldCount;
    uint16_t flags;
};

#endif // QUACLASS_H
