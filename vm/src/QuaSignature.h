#ifndef QUASIGNATURE_H
#define QUASIGNATURE_H

#include <cstring>

/* WARNING: This variable-length struct is intended for reinterpret pointer cast from existing data blobs   */
/*          Do not allocate new ones on the C++ heap!                                                       */

struct QuaSignature {
    unsigned char argCnt;
    char name[];
};


/* Helper struct which is needed for storing QuaSignature classes as key in a map */
struct QuaSignatureComp {
    bool operator()(QuaSignature* first, QuaSignature* second) const {
        if(first->argCnt != second->argCnt) {
            return first->argCnt < second->argCnt;
        }
        return std::strcmp(first->name, second->name) < 0;
    }
};

#endif // QUASIGNATURE_H
