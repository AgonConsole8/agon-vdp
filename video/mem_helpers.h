#ifndef MEM_HELPERS_H
#define MEM_HELPERS_H

#include <cstdint>
#if defined(__XTENSA__)
#include <xtensa/config/core-isa.h>
#endif

struct __attribute__((__may_alias__)) uint16_aligned {
    uint16_t value;
};

struct __attribute__((packed, __may_alias__)) uint16_unaligned {
    uint16_t value;
};

struct __attribute__((__may_alias__)) uint32_aligned {
    uint32_t value;
};

struct __attribute__((packed, __may_alias__)) uint32_unaligned {
    uint32_t value;
};

static inline uint_fast16_t read16_aligned(const void * p) {
    return reinterpret_cast<const uint16_aligned *>(p)->value;
}

static inline uint_fast16_t read16_unaligned(const void * p) {
#if defined(__XTENSA__) && XCHAL_UNALIGNED_LOAD_HW && !XCHAL_UNALIGNED_LOAD_EXCEPTION
    // Just do a direct read, -mno-strict-align isn't available until GCC 14
    return read16_aligned(p);
#else
    return reinterpret_cast<const uint16_unaligned *>(p)->value;
#endif
}

static inline void write16_aligned(void * p, uint_fast16_t value) {
    reinterpret_cast<uint16_aligned *>(p)->value = value;
}

static inline void write16_unaligned(void * p, uint_fast16_t value) {
#if defined(__XTENSA__) && XCHAL_UNALIGNED_STORE_HW && !XCHAL_UNALIGNED_STORE_EXCEPTION
    // Just do a direct write, -mno-strict-align isn't available until GCC 14
    write16_aligned(p, value);
#else
    reinterpret_cast<uint16_unaligned *>(p)->value = value;
#endif
}

static inline uint32_t read32_aligned(const void * p) {
    return reinterpret_cast<const uint32_aligned *>(p)->value;
}

static inline uint32_t read32_unaligned(const void * p) {
#if defined(__XTENSA__) && XCHAL_UNALIGNED_LOAD_HW && !XCHAL_UNALIGNED_LOAD_EXCEPTION
    // Just do a direct read, -mno-strict-align isn't available until GCC 14
    return read32_aligned(p);
#else
    return reinterpret_cast<const uint32_unaligned *>(p)->value;
#endif
}

static inline void write32_aligned(void * p, uint32_t value) {
    reinterpret_cast<uint32_aligned *>(p)->value = value;
}

static inline void write32_unaligned(void * p, uint32_t value) {
#if defined(__XTENSA__) && XCHAL_UNALIGNED_STORE_HW && !XCHAL_UNALIGNED_STORE_EXCEPTION
    // Just do a direct write, -mno-strict-align isn't available until GCC 14
    write32_aligned(p, value);
#else
    reinterpret_cast<uint32_unaligned *>(p)->value = value;
#endif
}

static inline uint16_t from_le16(uint16_t value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return value;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return __builtin_bswap16(value);
#else
    auto bytes = reinterpret_cast<const uint8_t *>(value);
    return (uint16_t)bytes[0] << 0 |
           (uint16_t)bytes[1] << 8;
#endif
}

static inline uint16_t to_le16(uint16_t value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return value;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return __builtin_bswap16(value);
#else
    uint16_t result;
    auto bytes = reinterpret_cast<uint8_t *>(result);
    bytes[0] = value >> 0;
    bytes[1] = value >> 8;
    return result;
#endif
}

static inline uint32_t from_le32(uint32_t value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return value;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return __builtin_bswap32(value);
#else
    auto bytes = reinterpret_cast<const uint8_t *>(value);
    return (uint32_t)bytes[0] << 0  |
           (uint32_t)bytes[1] << 8  |
           (uint32_t)bytes[2] << 16 |
           (uint32_t)bytes[3] << 24;
#endif
}

static inline uint32_t to_le32(uint32_t value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return value;
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return __builtin_bswap32(value);
#else
    uint32_t result;
    auto bytes = reinterpret_cast<uint8_t *>(result);
    bytes[0] = value >> 0;
    bytes[1] = value >> 8;
    bytes[2] = value >> 16;
    bytes[3] = value >> 24;
    return result;
#endif
}

#endif // MEM_HELPERS_H
