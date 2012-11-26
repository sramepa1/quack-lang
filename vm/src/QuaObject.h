#ifndef QUAOBJECT_H
#define QUAOBJECT_H

#include "QuaValue.h"

/* WARNING: This variable-length struct is intended for reinterpret pointer cast from existing data blobs   */
/*          Do not allocate new ones on the C++ heap!                                                       */

struct QuaObject
{
    __extension__ QuaValue fields[];
};

#endif // QUAOBJECT_H
