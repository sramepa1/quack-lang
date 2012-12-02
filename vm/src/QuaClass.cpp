#include "QuaClass.h"
#include "Exceptions.h"

#include <cstdlib>
#include <sstream>

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

QuaValue QuaClass::getInstance() {
	if(!isStatic()) {
		throw runtime_error(string("Attempted to take static instance from non-static class ") + className);
	}
	return instance;
}

void QuaClass::setInstance(QuaValue newInstance) {
	if(!isStatic()) {
		throw runtime_error(string("Attempted to set static instance for non-static class ") + className);
	}
	if(instance.value != 0) {
		throw runtime_error(string("Attempted to overwrite a non-null static instance of class ") + className);
	}
	instance = newInstance;
}

const char* QuaClass::getName() {
	return className.c_str();
}

QuaMethod* QuaClass::lookupMethod(QuaSignature* sig) {

	map<QuaSignature*, QuaMethod*, QuaSignatureComp>::iterator it = methods.find(sig);
	if(it != methods.end()) {
		return it->second;
	}
	if(parent == NULL) {
		throw NoSuchMethodException("No!");
		/*ostringstream os;
		os << "Method \"" << sig->name << "\" with " << (int)sig->argCnt << " arg(s) of class " << className
				<< " not found in internal VM lookup.";
		throw NoSuchMethodException(os.str());*/
	}
	return parent->lookupMethod(sig);
}

uint16_t QuaClass::lookupFieldIndex(const char *fieldName) {

	map<const char*, uint16_t, FieldNameComparator>::iterator it = fieldIndices.find(fieldName);
	if(it != fieldIndices.end()) {
		return it->second;
	}
	if(parent == NULL) {
		throw NoSuchFieldException(string("Field ") + fieldName + " of class "
								   + className + " not found in internal VM lookup.");
	}
	return parent->lookupFieldIndex(fieldName);
}

QuaValue QuaClass::defaultDeserializer(const char* data) {
	throw runtime_error("Attempted to deserialize a class that doesn't support loading from data blobs.");
}       // TODO how to pass class name without breaking a lot of things?
