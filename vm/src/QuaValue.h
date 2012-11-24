#ifndef _QUA_VALUE_H_
#define _QUA_VALUE_H_

extern "C" {
    #include <stdint.h>
}

#pragma pack(1)

struct QuaValue {
	uint32_t value;
	uint16_t type;
	uint16_t flags;
	
    QuaValue() : value(0), type(0), flags(0) {}
	QuaValue(uint32_t value, uint16_t type, uint16_t flags) : value(value), type(type), flags(flags) {}	
};

#pragma pack()

#endif
