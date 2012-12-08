#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cerrno>
extern "C" {
	#include <sys/mman.h>
	#include <stdint.h>
}

#include "Exceptions.h"
#include "globals.h"
#include "QuaClass.h"


inline void checkMmap(void* ptr, const char* errMsg) {
	if(ptr == MAP_FAILED) {
		int err = errno;
		std::ostringstream os;
		os << errMsg << std::endl;
		os << "Memory mapping failed with error code " << err << ", that is \"" << strerror(err) << "\"." << std::endl;
		throw std::runtime_error(os.str());
	}
}

inline const char* getConstantPoolEntry(void* poolPtr, uint16_t index) {
	return (char*)poolPtr + ((uint32_t*)((char*)poolPtr + 8))[index];
}

inline const uint64_t* getClassTableEntry(void* tablePtr, uint16_t index) {
	return (uint64_t*)tablePtr + 1 + index; // 1 jumps over table header
}

QuaClass* getThisClass();

inline const char* getCurrentCPEntry(uint16_t index) {
	return getConstantPoolEntry(getThisClass()->getCP(), index);
}

inline uint16_t resolveType(uint16_t & type)  {
	if(type & TYPE_UNRESOLVED) {
		uint16_t cpIndex = *((uint16_t*)getClassTableEntry(getThisClass()->getCT(), type ^ TYPE_UNRESOLVED));
		std::map<std::string, uint16_t>::iterator it = linkedTypes->find(std::string(getCurrentCPEntry(cpIndex)));
		if(it == linkedTypes->end()) {
			throw NoSuchClassException(std::string("Class '") + getCurrentCPEntry(cpIndex)
									   + "' not found in internal VM lookup.");
		} else {
			type = it->second;
		}
	}
	return type;
}

inline QuaClass* getClassFromType(uint16_t & type) {
	return typeArray[resolveType(type)];
}

inline QuaClass* getClassFromValue(QuaValue val) {
	return getClassFromType(val.type);
}

inline QuaClass* getThisClass() {
	return getClassFromValue(*VMBP);
}

inline QuaValue loadConstant(uint16_t & type, uint16_t cpIndex) {
	return getClassFromType(type)->deserialize(getCurrentCPEntry(cpIndex));
}

inline QuaValue newRawInstance(uint16_t & type) {
	uint16_t fieldCount = getClassFromType(type)->getFieldCount(); // also resolves type
	return heap->allocateNew(type, fieldCount);
}

inline QuaValue& getFieldByIndex(QuaValue that, uint16_t index) {
	return heap->dereference(that).instance->fields[index];
}

inline uint16_t getFieldIndex(QuaValue& that, const char* fieldName) {
	return getClassFromValue(that)->lookupFieldIndex(fieldName);
}

inline QuaValue& getFieldByName(QuaValue& that, const char* fieldName) {
	return getFieldByIndex(that, getFieldIndex(that, fieldName));
}

inline bool instanceOf(QuaValue& inst, uint16_t ofWhatType) {
	return getClassFromValue(inst)->isInstanceOf(getClassFromType(ofWhatType));
}

// Call a native method from another one. Push args on value stack, call this, and the rest should happen automagically.
inline QuaValue nativeCall(QuaValue& instance, QuaSignature* natSig) {

	// address stack is ignored when calling this way - the native method becomes "inlined" in the callee
	*(--VMSP) = instance;
	QuaValue* oldBP = VMBP;
	VMBP = VMSP;
	QuaMethod* natMeth = getThisClass()->lookupMethod(natSig);
	if(natMeth->action != QuaMethod::C_CALL) {
		std::ostringstream os;
		os << "Expected internally used method " << natSig->name << '(' + (int)natSig->argCnt << ')' << std::endl
				<< "of class " << getThisClass()->getName() << " to be native but it is not.";
		throw std::runtime_error(os.str());
	}
	QuaValue retVal = ( __extension__ (QuaValue (*)())natMeth->code )();
	VMBP = oldBP;
	VMSP += (unsigned int)natSig->argCnt + 1 /*this*/;

	return retVal;
}

__attribute__ ((noreturn)) inline void errorUnknownTag(uint8_t tag) {
	std::ostringstream os;
	os << "Unknown value tag encountered: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)tag << ".";
	throw std::runtime_error(os.str());
}

inline uint16_t getTaggedType(uint8_t tag) {
	switch(tag) {
		case SOP_TAG_NONE: throw std::runtime_error("Tagged type extraction attempted on an untagged reference value.");
		case SOP_TAG_BOOL: return typeCache.typeBool;
		case SOP_TAG_INT:  return typeCache.typeInteger;
		case SOP_TAG_FLOAT:return typeCache.typeFloat;
		default: errorUnknownTag(tag);
	}
}

// These are not actual QuaValue constructors to avoid including globals.h in QuaValue.h (dependency on typeCache)

inline QuaValue createBool(bool value) {
	return QuaValue((uint32_t)value, typeCache.typeBool, TAG_BOOL);
}

inline QuaValue createInteger(int32_t value) {
	return QuaValue(*((uint32_t*)&value), typeCache.typeInteger, TAG_INT);
}

inline QuaValue createFloat(float value) {
	return QuaValue(*((uint32_t*)&value), typeCache.typeFloat, TAG_FLOAT);
}

// moved from NativeLoader.h to allow usage of QuaValue constructors which set type correctly
// NativeLoader.h didn't (and shouldn't) have access to typeCache.
// Tag and type must be kept coherent or horrible things start happening :)
inline void quaValuesFromPtr(void* ptr, QuaValue& first, QuaValue& second) {
	first = createInteger((uint32_t)((uint64_t)ptr >> 32));
	second = createInteger((uint32_t)((uint64_t)ptr & 0xFFFFFFFF));
}

inline void* ptrFromQuaValues(QuaValue first, QuaValue second) {
	return (void*)((uint64_t)first.value << 32 | (uint64_t)second.value);
}

#endif // HELPERS_H
