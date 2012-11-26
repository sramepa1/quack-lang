#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <cerrno>
extern "C" {
    #include <sys/mman.h>
    #include <stdint.h>
}

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
        type = (*linkedTypes)[std::string(getCurrentCPEntry(cpIndex))];
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
    return getClassFromValue(*BP);
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
    return getClassFromValue(inst)->isInstanceOf(typeArray[ofWhatType]);
}

// Call a native method from another one. Push args on value stack, call this, and the rest should happen automagically.
inline QuaValue nativeCall(QuaValue& instance, QuaSignature* natSig) {

    // address stack is ignored when calling this way - the native method becomes "inlined" in the callee
    *(--SP) = instance;
    QuaValue* oldBP = BP;
    BP = SP;
    QuaMethod* natMeth = getThisClass()->lookupMethod(natSig);
    if(natMeth->action != QuaMethod::C_CALL) {
        std::ostringstream os;
        os << "Expected internally used method " << natSig->name << '(' + (int)natSig->argCnt << ')' << std::endl
                << "of class " << getThisClass()->getName() << " to be native but it is not.";
        throw std::runtime_error(os.str());
    }
    QuaValue retVal = ( __extension__ (QuaValue (*)())natMeth->code )();
    BP = oldBP;
    SP = SP + 1 /*this*/ + (unsigned int)natSig->argCnt;

    return retVal;
}

#endif // HELPERS_H
