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

#include "thread.h"

#define MYS_UINT64_MAX 0xFFFFFFFFFFFFFFFFULL

/**
 * @brief Generate random uint64_t value
 * 
 * @param minimum minimum value (inclusive)
 * @param maximum minimum value (inclusive)
 * @return uint64_t random value between [minimum, maximum]
 */
MYS_API uint64_t mys_randu64(uint64_t minimum, uint64_t maximum);
MYS_API int64_t mys_randi64(int64_t minimum, int64_t maximum);
MYS_API double mys_randf64(double minimum, double maximum);
MYS_API uint32_t mys_randu32(uint32_t minimum, uint32_t maximum);
MYS_API int32_t mys_randi32(int32_t minimum, int32_t maximum);
MYS_API float mys_randf32(float minimum, float maximum);
MYS_API const char *mys_randname();

////// Legacy

static inline uint64_t randu64(uint64_t minimum, uint64_t maximum) {
    return mys_randu64(minimum, maximum);
}
static inline int64_t randi64(int64_t minimum, int64_t maximum) {
    return mys_randi64(minimum, maximum);
}
static inline double randf64(double minimum, double maximum) {
    return mys_randf64(minimum, maximum);
}
static inline uint32_t randu32(uint32_t minimum, uint32_t maximum) {
    return mys_randu32(minimum, maximum);
}
static inline int32_t randi32(int32_t minimum, int32_t maximum) {
    return mys_randi32(minimum, maximum);
}
static inline float randf32(float minimum, float maximum) {
    return mys_randf32(minimum, maximum);
}
static inline const char *randname() {
    return mys_randname();
}

////// Advanced

MYS_API void mys_rand_init();
MYS_API void mys_rand_srand(uint64_t a0, uint64_t a1);
MYS_API uint64_t mys_rand_seed();
MYS_API uint64_t mys_rand_seed_fixed();
MYS_API uint64_t mys_rand_seed_time();
#ifdef ARCH_X64
MYS_API uint64_t mys_rand_seed_x64();
#endif
#ifdef ARCH_AARCH64
MYS_API uint64_t mys_rand_seed_aarch64();
#endif
/* legacy random */
MYS_API uint64_t mys_rand_legacy_u64();
/* https://prng.di.unimi.it
 * https://prng.di.unimi.it/splitmix64.c
 * This is a fixed-increment version of Java 8's SplittableRandom generator
 * See http://dx.doi.org/10.1145/2714064.2660195 and 
 * http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
 * It is a very fast generator passing BigCrush, and it can be useful if
 * for some reason you absolutely want 64 bits of state.
 */
MYS_API uint64_t mys_rand_splitmix_u64();
/* https://prng.di.unimi.it
 * https://prng.di.unimi.it/xoroshiro128starstar.c
 * This is xoroshiro128** 1.0, one of our all-purpose, rock-solid,
 * small-state generators. It is extremely (sub-ns) fast and it passes all
 * tests we are aware of, but its state space is large enough only for
 * mild parallelism.
 * For generating just floating-point numbers, xoroshiro128+ is even
 * faster (but it has a very mild bias, see notes in the comments).
 * The state must be seeded so that it is not everywhere zero. If you have
 * a 64-bit seed, we suggest to seed a splitmix64 generator and use its
 * output to fill __xoroshiro128_x.
 */
MYS_API uint64_t mys_rand_xoroshiro128ss_u64();

////// Internal

typedef struct _mys_rand_G_t {
    bool inited;
    uint64_t splitmix64_x;
    uint64_t xoroshiro128_x[2];
} _mys_rand_G_t;

extern mys_thread_local _mys_rand_G_t _mys_rand_G;


/* Tester:
- mpicc -O3 -g -lm -I../include xoshiro256.c && ./a.out 1000000000 > xoshiro256.log

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

// #define MYS_SPLITMIX_RANDOM
// #define MYS_LEGACY_RANDOM
#include <mys.h>

#define minimum 0
#define maximum 100
#define ssize ((maximum)-(minimum)+1)

int main(int argc , char **argv) {
    uint64_t n = 1000000000;
    if (argc > 1) {
        n = atoll(argv[1]);
    }
    uint64_t box[ssize] = {0};
    printf("%s\n", randname());
    double *buffer = (double *)malloc(n * sizeof(double));
    memset(buffer, 0, n * sizeof(double));
    double t1 = hrtime();
    for (uint64_t i = 0; i < n; i++) {
        buffer[i] = randf64(minimum, maximum);
    }
    double t2 = hrtime();
    double tdiff = t2 - t1;
    double ns = (tdiff * 1e9)/(double)(n * sizeof(double));
    double throughput = 1 / ns;
    printf("%lld double in %.2f sec (%.2f ns/Byte %.2f GB/s)\n", n, tdiff, ns, throughput);

    for (uint64_t i = 0; i < n; i++) {
        box[(int)buffer[i] - minimum] += 1;
    }

    for (int i = 0; i < ssize; i++) {
        printf("[%03d,%03d) %.3f %d\n", i-minimum, i+1-minimum, ((double)box[i]/n) * 100, (int)box[i]);
    }

    free(buffer);
}


*/
