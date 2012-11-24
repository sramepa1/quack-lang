#include "QuaClass.h"
#include <cstdlib>
#include <stdexcept>

using namespace std;

QuaClass::QuaClass() : deserializer(&defaultDeserializer) { }

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

// TODO Change lookups to const char* to avoid unnecessary std::string construction every time!

QuaMethod* QuaClass::lookupMethod(QuaSignature* sig) {

    // Double map lookup should be faster than iterator construction and end() comparison for very small maps
    if(methods.count(sig)) {
        return methods[sig];
    }
    if(parent == NULL) {
        throw runtime_error("This should throw a NoSuchMethodException, but it's not yet implemented."); // TODO
    }
    return parent->lookupMethod(sig);
}

uint16_t QuaClass::lookupFieldIndex(string fieldName) {

    if(fieldIndices.count(fieldName)) {
        return fieldIndices[fieldName];
    }
    if(parent == NULL) {
        throw runtime_error("This should throw a NoSuchFieldException, but it's not yet implemented."); // TODO
    }
    return parent->lookupFieldIndex(fieldName);
}

QuaValue QuaClass::defaultDeserializer(const char* data) {
    throw runtime_error("Attempted to deserialize a class that doesn't support loading from data blobs.");
    // TODO add class name to error message (how come QuaClass doesn't know its name?)
}

