#pragma once

/*
  No pseudo-random generator function is perfect, but some are useful.
  DO NOT use these code for cryptographic purposes.
  Seed is modified to be fast and easy to feed.
*/

#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "_config.h"

/**
 * @brief Set the seed for a new sequence to be returned by mys_rand_xoroshiro128ss().
 * 
 * @param a0 upper 64 bits of xoroshiro128ss algorithm's internal state
 * @param a1 lower 64 bits of xoroshiro128ss algorithm's internal state
 * 
 * @note By default, `mys_rand_xoroshiro128ss()` is seeded with `a0=0, a1=1`.
 */
MYS_API void mys_rand_srand(uint64_t a0, uint64_t a1);
/**
 * @brief Set the seed by querying some fast hardware performance counters.
 * @note On failure, it fallbacks to `a0=0, a1=time(NULL)`.
 */
MYS_API void mys_rand_srand_hardware();
/**
 * @brief All-purpose, rock-solid, small-state pseudo-random number generator
 * 
 * @note
 * This is xoroshiro128** 1.0, one of our all-purpose, rock-solid,
 * small-state generators. It is extremely (sub-ns) fast and it passes all
 * tests we are aware of, but its state space is large enough only for
 * mild parallelism.
 * @note
 * For generating just floating-point numbers, xoroshiro128+ is even
 * faster (but it has a very mild bias, see notes in the comments).
 * The state must be seeded so that it is not everywhere zero. If you have
 * a 64-bit seed, we suggest to seed a splitmix64 generator and use its
 * output to fill __xoroshiro128_x.
 * @note
 * https://prng.di.unimi.it
 * @note
 * https://prng.di.unimi.it/xoroshiro128starstar.c
 * @note
 * https://doxygen.postgresql.org/pg__prng_8c_source.html
 */
MYS_API uint64_t mys_rand_xoroshiro128ss();


//////// exclusive maximum value for convenient array element access


/**
 * @brief Generate random uint64_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 * 
 * @note
 * To have a random value between [mi, maximum], use `mys_rand_xoroshiro128ss()` instead.
 */
MYS_API uint64_t mys_rand_u64(uint64_t mi, uint64_t ma);
/**
 * @brief Generate random uint32_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API uint32_t mys_rand_u32(uint32_t mi, uint32_t ma);
/**
 * @brief Generate random uint16_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API uint16_t mys_rand_u16(uint16_t mi, uint16_t ma);
/**
 * @brief Generate random uint8_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API uint8_t mys_rand_u8(uint8_t mi, uint8_t ma);
/**
 * @brief Generate random int64_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API int64_t mys_rand_i64(int64_t mi, int64_t ma);
/**
 * @brief Generate random int32_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API int32_t mys_rand_i32(int32_t mi, int32_t ma);
/**
 * @brief Generate random int16_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API int16_t mys_rand_i16(int16_t mi, int16_t ma);
/**
 * @brief Generate random int8_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API int8_t mys_rand_i8(int8_t mi, int8_t ma);
/**
 * @brief Generate random size_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API size_t mys_rand_sizet(size_t mi, size_t ma);
/**
 * @brief Generate random ssize_t value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (exclusive)
 * @return random value between [mi, ma)
 */
MYS_API ssize_t mys_rand_ssizet(ssize_t mi, ssize_t ma);
/**
 * @brief Generate random double value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (inclusive)
 * @return random value between [mi, ma]
 * 
 * @note This subroutine originates from scaling a floating-point
 * number within the range [0, 1). Since IEEE 754 specifies five
 * distinct rounding rules that yield varying results, the value
 * `ma` can be obtained when using either the "round to nearest"
 * (default) or "rounding up (ceiling)" rounding rules.
 */
MYS_API double mys_rand_f64(double mi, double ma);
/**
 * @brief Generate random float value
 * 
 * @param mi minimum value (inclusive)
 * @param ma maximum value (inclusive)
 * @return random value between [mi, ma]
 * 
 * @note This subroutine originates from scaling a floating-point
 * number within the range [0, 1). Since IEEE 754 specifies five
 * distinct rounding rules that yield varying results, the value
 * `ma` can be obtained when using either the "round to nearest"
 * (default) or "rounding up (ceiling)" rounding rules.
 */
MYS_API float mys_rand_f32(float mi, float ma);


///////////////////////

#define _RLOOP(fn) do { for (size_t i = 0; i < nelem; i++) array[i] = fn(mi, ma); } while (0)
MYS_STATIC void mys_rand_u64_array   (uint64_t *array, size_t nelem, uint64_t mi, uint64_t ma) { _RLOOP(mys_rand_u64   ); }
MYS_STATIC void mys_rand_u32_array   (uint32_t *array, size_t nelem, uint32_t mi, uint32_t ma) { _RLOOP(mys_rand_u32   ); }
MYS_STATIC void mys_rand_u16_array   (uint16_t *array, size_t nelem, uint16_t mi, uint16_t ma) { _RLOOP(mys_rand_u16   ); }
MYS_STATIC void mys_rand_u8_array    (uint8_t  *array, size_t nelem, uint8_t  mi, uint8_t  ma) { _RLOOP(mys_rand_u8    ); }
MYS_STATIC void mys_rand_i64_array   (int64_t  *array, size_t nelem, int64_t  mi, int64_t  ma) { _RLOOP(mys_rand_i64   ); }
MYS_STATIC void mys_rand_i32_array   (int32_t  *array, size_t nelem, int32_t  mi, int32_t  ma) { _RLOOP(mys_rand_i32   ); }
MYS_STATIC void mys_rand_i16_array   (int16_t  *array, size_t nelem, int16_t  mi, int16_t  ma) { _RLOOP(mys_rand_i16   ); }
MYS_STATIC void mys_rand_i8_array    (int8_t   *array, size_t nelem, int8_t   mi, int8_t   ma) { _RLOOP(mys_rand_i8    ); }
MYS_STATIC void mys_rand_sizet_array (size_t   *array, size_t nelem, size_t   mi, size_t   ma) { _RLOOP(mys_rand_sizet ); }
MYS_STATIC void mys_rand_ssizet_array(ssize_t  *array, size_t nelem, ssize_t  mi, ssize_t  ma) { _RLOOP(mys_rand_ssizet); }
MYS_STATIC void mys_rand_f64_array   (double   *array, size_t nelem, double   mi, double   ma) { _RLOOP(mys_rand_f64   ); }
MYS_STATIC void mys_rand_f32_array   (float    *array, size_t nelem, float    mi, float    ma) { _RLOOP(mys_rand_f32   ); }
#undef _RLOOP


///////////////////////


/* Tester:
The following performance metrics are performed with old test suite on old rand.h.
They differ from current implementation.
- Intel Xeon Gold 6132 (icc 19.0.5.281)
    - xoroshiro128ss: 1000000000 double in 4.33 sec (0.54 ns/Byte 1.85 GB/s)
    - splitmix:       1000000000 double in 5.78 sec (0.72 ns/Byte 1.38 GB/s)
    - legacy:         1000000000 double in 15.24 sec (1.91 ns/Byte 0.52 GB/s)
- Huawei Kunpeng920 (gcc 9.3.1):
    - xoroshiro128ss: 1000000000 double in 7.42 sec (0.93 ns/Byte 1.08 GB/s)
    - splitmix:       1000000000 double in 7.42 sec (0.93 ns/Byte 1.08 GB/s)
    - legacy:         1000000000 double in 24.04 sec (3.01 ns/Byte 0.33 GB/s)
- Apple M1 (Apple clang version 14.0.0):
    - xoroshiro128ss: 1000000000 double in 6.11 sec (0.76 ns/Byte 1.31 GB/s)
    - splitmix:       1000000000 double in 3.11 sec (0.39 ns/Byte 2.57 GB/s)
    - legacy:         1000000000 double in 8.74 sec (1.09 ns/Byte 0.92 GB/s)

Tester 2.0: gcc -I${MYS_DIR}/include -Wall -Wextra a.c && ./a.out

#define MYS_IMPL
#define MYS_NO_MPI
#include <mys.h>

#define mi (uint64_t)(10)
#define ma (uint64_t)(20)

// #define data_t  uint64_t
// #define rand_fn mys_rand_u64_array
// #define cmp_fn  ASX_BETWEEN_IE_U64
#define data_t  double
#define rand_fn mys_rand_f64_array
#define cmp_fn  ASX_BETWEEN_II_F64 // rounding to ma
// #define data_t  float
// #define rand_fn mys_rand_f32_array
// #define cmp_fn  ASX_BETWEEN_II_F32 // rounding to ma

int main(int argc , char **argv) {
    size_t n = 100000000;
    if (argc > 1) {
        n = atoll(argv[1]);
    }
    const ssize_t range = ma - mi + 2;
    ssize_t *box = (ssize_t *)calloc(sizeof(ssize_t), range);

    data_t *buffer = (data_t *)calloc(sizeof(data_t), n);
    memset(buffer, 7, n * sizeof(data_t));
    memset(buffer, 3, n * sizeof(data_t));
    memset(buffer, 0, n * sizeof(data_t));

    // mys_rand_srand_hardware();
    double t1 = mys_hrtime();
    rand_fn(buffer, n, mi, ma);
    double t2 = mys_hrtime();

    double tdiff = t2 - t1;
    double ns = (tdiff * 1e9)/(double)(n * sizeof(double));
    double throughput = 1 / ns;
    printf("%zu %s in %.2f sec (%.2f ns/Byte %.2f GB/s)\n",
        n, MYS_MACRO2STR(data_t), tdiff, ns, throughput);

    for (size_t i = 0; i < n; i++) {
        cmp_fn((data_t)mi, buffer[i], (data_t)ma, "i=%zu", i);
        // printf("%.17f\n", buffer[i]);
        box[1 + (ssize_t)buffer[i] - mi] += 1;
    }

    printf("    range      count      percent\n");
    for (ssize_t i = 0; i < range; i++) {
        printf("[%03lld,%03lld) %10zd %10.3f %%\n",
            -1+(i+mi), -1+(i+1+mi), box[i], ((double)box[i]/n) * 100);
    }

    free(buffer);
    free(box);
}

*/
