#ifndef QUASIGNATURE_H
#define QUASIGNATURE_H


/* WARNING: This variable-length struct is intended for reinterpret pointer cast from existing data blobs   */
/*          Do not allocate new ones on the C++ heap!                                                       */

struct QuaSignature {
    unsigned char argCnt;
    char name[];
};

#endif // QUASIGNATURE_H
