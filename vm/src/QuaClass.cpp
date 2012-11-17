#include "QuaClass.h"
#include <cstdlib>

using namespace std;

QuaClass::~QuaClass() {
    for(map<QuaSignature*, QuaMethod*, QuaSignatureComp>::iterator it = methods.begin(); it != methods.end(); ++it) {
        delete (*it).second;
    }
}

uint16_t QuaClass::getFieldCount() {

    uint16_t fieldCount = myFieldCount;
    QuaClass* ancestor = parent;

    while(ancestor != NULL) {
        fieldCount += ancestor->myFieldCount;
        ancestor = ancestor->parent;
    }

    return fieldCount;
}

QuaMethod* QuaClass::lookupMethod(QuaSignature* sig) {
    //TODO
    return NULL;
}

uint16_t QuaClass::lookupFieldIndex(string fieldName) {
    //TODO
    return 13;
}

