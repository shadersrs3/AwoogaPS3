#ifndef _PS3_COMMON_BYTE_SWAP_H
#define _PS3_COMMON_BYTE_SWAP_H

#include <cstdint>

#include <Common/ByteOrdering.h>

#if !defined(_IS_BIG_ENDIAN_MACHINE)
# error "Expected big-endian machine macro to work properly."
#endif

template<typename T> inline constexpr T swapBytes(const T& data) {
    if constexpr (sizeof(T) == 2) {
        return (uint16_t)(uint8_t)(data >> 8) | ((data & 0xFF) << 8);
    } else if constexpr (sizeof(T) == 4) {
        return ((data & 0xFF0000) >> 8) | ((data & 0xFF) << 24) | ((data & 0xFF00) << 8) | ((data & 0xFF000000) >> 24);
    } else if constexpr (sizeof(T) == 8) {
        return (uint64_t)swapBytes<uint32_t>((uint32_t) data) << 32 | (uint64_t)swapBytes<uint32_t>((uint32_t) (data >> 32));
    }
    return (T) data;
}

template<typename T, ByteOrdering byteOrdering> T getValue(const T& data) {
    if constexpr (byteOrdering == LittleEndian) {
#if _IS_BIG_ENDIAN_MACHINE == 1
        return swapBytes<T>(data);
#else
        return data;
#endif
    } else if constexpr (byteOrdering == BigEndian) {
#if _IS_BIG_ENDIAN_MACHINE == 0
        return swapBytes<T>(data);
#else
        return data;
#endif
    }
    return 0;
}

#define V16_LE(x) getValue<uint16_t, ByteOrdering::LittleEndian>(x)
#define V32_LE(x) getValue<uint32_t, ByteOrdering::LittleEndian>(x)
#define V64_LE(x) getValue<uint64_t, ByteOrdering::LittleEndian>(x)
#define V16_BE(x) getValue<uint16_t, ByteOrdering::BigEndian>(x)
#define V32_BE(x) getValue<uint32_t, ByteOrdering::BigEndian>(x)
#define V64_BE(x) getValue<uint64_t, ByteOrdering::BigEndian>(x)

#endif