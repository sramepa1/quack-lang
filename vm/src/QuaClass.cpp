#include "QuaClass.h"
#include <cstdlib>

using namespace std;

QuaClass::QuaClass(void *constantPool, void* classDef) : relevantCP(constantPool)
{
    //TODO
}

uint32_t QuaClass::getFieldCount() {
    return 42;  //TODO
}

QuaMethod* QuaClass::lookupMethod(QuaSignature *sig) {
    //TODO
    return NULL;
}

uint32_t QuaClass::lookupFieldIndex(string fieldName) {
    //TODO
    return 13;
}
