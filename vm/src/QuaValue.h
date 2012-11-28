#ifndef _QUA_VALUE_H_
#define _QUA_VALUE_H_

extern "C" {
    #include <stdint.h>
}

#include "bytecode.h"

// these tags are used in flags
#define TAG_REF SOP_TAG_NONE
#define TAG_INT SOP_TAG_INT
#define TAG_FLOAT SOP_TAG_FLOAT
#define TAG_BOOL SOP_TAG_BOOL

// this is for the type field
#define TYPE_UNRESOLVED 0x8000

#pragma pack(1)

struct QuaValue {
    uint32_t value;
    uint16_t type;
    uint8_t tag;
    uint8_t flags;

    // constructs a null QuaValue (type 0 must be null in rt.qc and dereferencing value 0 will also fail)
    QuaValue() : value(0), type(0), tag(0), flags(0) {}

    // constructs a user-defined QuaValue with default flags
    QuaValue(uint32_t value, uint16_t type, uint8_t tag) : value(value), type(type), tag(tag), flags(0) {}
};

#pragma pack()

#endif
