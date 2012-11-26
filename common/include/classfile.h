#ifndef CLASSFILE_H
#define CLASSFILE_H

// little endian
#define MAGIC_NUM 0x4B0C550D


// Class flags
#define CLS_STATIC 0x1;
#define CLS_DESTRUCTIBLE 0x2;

#define CLS_VARIABLE_LENGTH 0x4000
#define CLS_INCONSTRUCTIBLE 0x8000


// Field flags
#define FIELD_FLAG_HIDDEN 0x8000


// Method flags
#define METHOD_FLAG_NATIVE 0x1

#endif
