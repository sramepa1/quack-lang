#include "QuaClass.h"
#include <cstdlib>
#include <stdexcept>
#include <sstream>

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

QuaValue QuaClass::getInstance() {
    if(!isStatic()) {
        throw runtime_error("Attempted to take static instance from a non-static class.");
    }
    return instance;
}

void QuaClass::setInstance(QuaValue newInstance) {
    if(!isStatic()) {
        throw runtime_error("Attempted to set static instance for a non-static class.");
    }
    if(instance.value != 0) {
        throw runtime_error("Attempted to overwrite a non-null static instance.");
    }
    instance = newInstance;
}

// TODO add class name to error message (how come QuaClass doesn't know its name?)

QuaMethod* QuaClass::lookupMethod(QuaSignature* sig) {

    map<QuaSignature*, QuaMethod*, QuaSignatureComp>::iterator it = methods.find(sig);
    if(it != methods.end()) {
        return it->second;
    }
    if(parent == NULL) {
        ostringstream os;
        os << "Method \"" << sig->name << "\" with " << (int)sig->argCnt << " arg(s) not found in internal VM lookup.";
        throw runtime_error(os.str());
    }
    return parent->lookupMethod(sig);
}

uint16_t QuaClass::lookupFieldIndex(const char *fieldName) {

    map<const char*, uint16_t, FieldNameComparator>::iterator it = fieldIndices.find(fieldName);
    if(it != fieldIndices.end()) {
        return it->second;
    }
    if(parent == NULL) {
        throw runtime_error(string("Field ") + fieldName + " not found in internal VM lookup.");
    }
    return parent->lookupFieldIndex(fieldName);
}

QuaValue QuaClass::defaultDeserializer(const char* data) {
    throw runtime_error("Attempted to deserialize a class that doesn't support loading from data blobs.");
}

