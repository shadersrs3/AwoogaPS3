#ifndef _PS3_COMMON_BYTE_ORDERING_H
#define _PS3_COMMON_BYTE_ORDERING_H

// TODO: change this if big endian
#define _IS_BIG_ENDIAN_MACHINE (0)

enum ByteOrdering : int {
    BigEndian, LittleEndian
};

#endif