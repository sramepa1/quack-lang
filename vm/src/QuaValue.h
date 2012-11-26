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
    uint16_t flags;

    // constructs a null QuaValue (type 0 must be null in rt.qc and dereferencing value 0 will also fail)
    QuaValue() : value(0), type(0), flags(0) {}
    QuaValue(uint32_t value, uint16_t type, uint16_t flags) : value(value), type(type), flags(flags) {}
};

#pragma pack()

#endif
