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

    map<QuaSignature*, QuaMethod*, QuaSignatureComp>::iterator it = methods.find(sig);
    if(it != methods.end()) {
        return it->second;
    }
    if(parent == NULL) {
        throw runtime_error("This should throw a NoSuchMethodException, but it's not yet implemented."); // TODO
    }
    return parent->lookupMethod(sig);
}

uint16_t QuaClass::lookupFieldIndex(string fieldName) {

    map<std::string, uint16_t>::iterator it = fieldIndices.find(fieldName);
    if(it != fieldIndices.end()) {
        return it->second;
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

