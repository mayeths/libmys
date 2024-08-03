/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "_private.h"
#include "../rand.h"

struct _mys_rand_G_t {
    bool inited;
    uint64_t seed[2];
};

mys_thread_local struct _mys_rand_G_t _mys_rand_G = {
    .inited = false,
    .seed = {0, 0},
};

static void _mys_rand_init()
{
    if (_mys_rand_G.inited == true)
        return;
    _mys_rand_G.seed[0] = 0;
    _mys_rand_G.seed[1] = 1;
    _mys_rand_G.inited = true;
}

MYS_PUBLIC void mys_rand_seed(uint64_t a1)
{
    _mys_rand_init();
    _mys_rand_G.seed[0] = 0;
    _mys_rand_G.seed[1] = a1;
}

MYS_PUBLIC void mys_rand_seed2(uint64_t a0, uint64_t a1)
{
    _mys_rand_init();
    _mys_rand_G.seed[0] = a0;
    _mys_rand_G.seed[1] = a1;
}

MYS_PUBLIC void mys_rand_seed_time()
{
    uint64_t t;
    t = (uint64_t)time(NULL);
    /* A(1010)5(0101) won't INVALID(1111_1111) again */
    uint64_t a0 = (t << 32) | (t & 0xAAAA5555);
    uint64_t a1 = UINT64_MAX - a0;
    mys_rand_seed2(a0, a1);
}

MYS_PUBLIC void mys_rand_seed_hardware()
{
    uint64_t t;
#if defined(ARCH_X64)
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
    t = ((uint64_t)hi << 32) | (uint64_t)lo;
#elif defined(ARCH_AARCH64)
    __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(t));
#else
    mys_rand_seed_time();
    return;
#endif
    /* A(1010)5(0101) won't INVALID(1111_1111) again */
    uint64_t a0 = (t << 32) | (t & 0xAAAA5555);
    uint64_t a1 = UINT64_MAX - a0;
    mys_rand_seed2(a0, a1);
}

static uint64_t _mys_rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}
MYS_PUBLIC uint64_t mys_rand_xoroshiro128ss()
{
    _mys_rand_init();
    const uint64_t s0 = _mys_rand_G.seed[0];
    uint64_t s1 = _mys_rand_G.seed[1];
    const uint64_t result = _mys_rotl(s0 * 5, 7) * 9;
    s1 ^= s0;
    _mys_rand_G.seed[0] = _mys_rotl(s0, 24) ^ s1 ^ (s1 << 16);
    _mys_rand_G.seed[1] = _mys_rotl(s1, 37);
    return result;
}

MYS_PUBLIC uint64_t mys_rand_u64(uint64_t mi, uint64_t ma)
{
    uint64_t v = mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    if (mi == 0 && ma == UINT64_MAX)
        return v;
    return mi + v % (ma - mi);
}

MYS_PUBLIC uint32_t mys_rand_u32(uint32_t mi, uint32_t ma) { return (uint32_t)mys_rand_u64(mi, ma); }
MYS_PUBLIC uint16_t mys_rand_u16(uint16_t mi, uint16_t ma) { return (uint16_t)mys_rand_u64(mi, ma); }
MYS_PUBLIC uint8_t  mys_rand_u8 (uint8_t  mi, uint8_t  ma) { return (uint8_t )mys_rand_u64(mi, ma); }

MYS_PUBLIC int64_t mys_rand_i64(int64_t mi, int64_t ma)
{
    union _mys_dicast_t { uint64_t u64; int64_t i64; } v;
    v.u64 = mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    if (mi == INT64_MIN && ma == INT64_MAX)
        return v.i64;
    return mi + v.u64 % (ma - mi);
}

MYS_PUBLIC int32_t mys_rand_i32(int32_t mi, int32_t ma) { return (int32_t)mys_rand_i64(mi, ma); }
MYS_PUBLIC int16_t mys_rand_i16(int16_t mi, int16_t ma) { return (int16_t)mys_rand_i64(mi, ma); }
MYS_PUBLIC int8_t  mys_rand_i8 (int8_t  mi, int8_t  ma) { return (int8_t )mys_rand_i64(mi, ma); }

MYS_PUBLIC size_t mys_rand_sizet(size_t mi, size_t ma)
{
#if SIZE_MAX == UINT64_MAX
    return (size_t)mys_rand_u64(mi, ma);
#elif SIZE_MAX == UINT32_MAX
    return (size_t)mys_rand_u32(mi, ma);
#else
#error Invalid SIZE_MAX
#endif
}

MYS_PUBLIC ssize_t mys_rand_ssizet(ssize_t mi, ssize_t ma)
{
    // Do not use SSIZE_MAX macro here because C99 doesn't have it
    if (sizeof(ssize_t) == sizeof(int64_t))
        return (ssize_t)mys_rand_i64(mi, ma);
    if (sizeof(ssize_t) == sizeof(int32_t))
        return (ssize_t)mys_rand_i32(mi, ma);
    else
        return (ssize_t)mys_rand_i64(mi, ma);
}

MYS_PUBLIC double mys_rand_f64(double mi, double ma)
{
    uint64_t v = mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    // See PostgreSQL pg_prng_double()
    double p252 = 2.2204460492503131e-16; // pow(2, -52)
    double v01 = (double)(v >> (64 - 52)) * p252; // [0, 1)
    return mi + v01 * (ma - mi); // [mi, ma] due to IEEE 754 rounding
}

MYS_PUBLIC float mys_rand_f32(float mi, float ma) {
    uint32_t v = (uint32_t)mys_rand_xoroshiro128ss();
    if (ma <= mi)
        return mi;
    float p223 = 1.1920928955078125e-07; // pow(2, -23)
    float v01 = (float)(v >> (32 - 23)) * p223; // [0, 1)
    return mi + v01 * (ma - mi); // [mi, ma] due to IEEE 754 rounding
}
