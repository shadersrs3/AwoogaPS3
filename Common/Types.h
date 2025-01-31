#ifndef _COMMON_TYPES_H
#define _COMMON_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
#include <Common/ByteSwap.h>

namespace AwoogaCommon {
#pragma pack(push, 1)
template<typename T, ByteOrdering ordering>
struct uint {
    T data;
    uint() {}
    uint(const T& _data) : data(_data) {}
    inline T getValue() { return ::getValue<T, ordering>(data); }
    void setValue(const T& data) { this->data = ::getValue<T, ordering>(data); }
};

#pragma pack(pop)
}

using u16_le = AwoogaCommon::uint<uint16_t, ByteOrdering::LittleEndian>;
using u16_be = AwoogaCommon::uint<uint16_t, ByteOrdering::BigEndian>;

using u32_le = AwoogaCommon::uint<uint32_t, ByteOrdering::LittleEndian>;
using u32_be = AwoogaCommon::uint<uint32_t, ByteOrdering::BigEndian>;

using u64_le = AwoogaCommon::uint<uint64_t, ByteOrdering::LittleEndian>;
using u64_be = AwoogaCommon::uint<uint64_t, ByteOrdering::BigEndian>;

using s16_le = AwoogaCommon::uint<int16_t, ByteOrdering::LittleEndian>;
using s16_be = AwoogaCommon::uint<int16_t, ByteOrdering::BigEndian>;

using s32_le = AwoogaCommon::uint<int32_t, ByteOrdering::LittleEndian>;
using s32_be = AwoogaCommon::uint<int32_t, ByteOrdering::BigEndian>;

using s64_le = AwoogaCommon::uint<int64_t, ByteOrdering::LittleEndian>;
using s64_be = AwoogaCommon::uint<int64_t, ByteOrdering::BigEndian>;

using f32_le = AwoogaCommon::uint<float, ByteOrdering::LittleEndian>;
using f32_be = AwoogaCommon::uint<float, ByteOrdering::BigEndian>;

using f64_le = AwoogaCommon::uint<double, ByteOrdering::LittleEndian>;
using f64_be = AwoogaCommon::uint<double, ByteOrdering::BigEndian>;
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#endif