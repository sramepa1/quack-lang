#ifndef HELPERS_H
#define HELPERS_H

#include <sstream>
#include <stdexcept>
#include <cerrno>
extern "C" {
    #include <sys/mman.h>
    #include <stdint.h>
}

#include "globals.h"
#include "QuaClass.h"

#define TYPE_UNRESOLVED 0x8000



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

inline QuaClass* resolveType(uint16_t & type) {
    if(type & TYPE_UNRESOLVED) {
        type = type ^ TYPE_UNRESOLVED; // TODO this is an incorrect placeholder! Replace with type mapping!
    }
    return typeArray[type];
}

inline QuaClass* getClass(QuaValue val) {
    return typeArray[val.type];
}

inline QuaClass* getThisClass() {
    return getClass(*BP);
}

inline const char* getCurrentCPEntry(uint16_t index) {
    return getConstantPoolEntry(getThisClass()->getCP(), index);
}

inline QuaValue loadConstant(uint16_t & type, uint16_t cpIndex) {
    return resolveType(type)->deserialize(getCurrentCPEntry(cpIndex));
}

inline QuaValue& getFieldByIndex(QuaValue that, uint16_t index) {
    return heap->dereference(that).instance->fields[index];
}

inline uint16_t getFieldIndex(QuaValue that, const char* fieldName) {
    return typeArray[that.type]->lookupFieldIndex(fieldName);
}

inline QuaValue& getFieldByName(QuaValue that, const char* fieldName) {
    return getFieldByIndex(that, getFieldIndex(that, fieldName));
}

#endif // HELPERS_H
