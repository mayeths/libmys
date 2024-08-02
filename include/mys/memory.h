#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#ifdef OS_LINUX
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "_config.h"
#include "macro.h"

typedef struct mys_arena_t {
    char name[32];
    size_t peak; // memory bytes that peak alive
    size_t alive; // memory bytes that being used
    size_t freed; // memory bytes that freed
    size_t total; // memory bytes that total allocated
    bool _reged;
} mys_arena_t;

MYS_API extern mys_arena_t _mys_predefined_arena_ext;
MYS_API extern mys_arena_t _mys_predefined_arena_log;
MYS_API extern mys_arena_t _mys_predefined_arena_pool;
#define mys_arena_ext ((mys_arena_t *)(&_mys_predefined_arena_ext))
#define mys_arena_log ((mys_arena_t *)(&_mys_predefined_arena_log))
#define mys_arena_pool ((mys_arena_t *)(&_mys_predefined_arena_pool))

// For external use (using mys_arena_ext)
MYS_API void* mys_malloc(size_t size);
MYS_API void* mys_calloc(size_t count, size_t size);
MYS_API void* mys_aligned_alloc(size_t alignment, size_t size);
MYS_API void* mys_realloc(void* ptr, size_t size);
MYS_API void mys_free(void* ptr);

MYS_API mys_arena_t *mys_arena_create(const char *name);
MYS_API void mys_arena_destroy(mys_arena_t **arena);
MYS_API void* mys_malloc2(mys_arena_t *arena, size_t size);
MYS_API void* mys_calloc2(mys_arena_t *arena, size_t count, size_t size);
MYS_API void* mys_aligned_alloc2(mys_arena_t *arena, size_t alignment, size_t size);
MYS_API void* mys_realloc2(mys_arena_t *arena, void* ptr, size_t size, size_t _old_size);
MYS_API void mys_free2(mys_arena_t *arena, void* ptr, size_t _size);

/* Cache clean */
MYS_API void mys_cache_flush(size_t nbytes);

#if defined(OS_LINUX) && defined(MYS_ENABLE_SHM)
typedef struct mys_shm_t {
    void *mem;
    size_t _size;
    int _fd;
    char _name[NAME_MAX];
} mys_shm_t;

MYS_API mys_shm_t mys_alloc_shared_memory(int owner_rank, size_t size);
MYS_API void mys_free_shared_memory(mys_shm_t *shm);
#endif


typedef struct mys_bits_t {
    char bits[64 + 1];
} mys_bits_t;

MYS_API mys_bits_t mys_bits(const void *data, size_t size);
MYS_STATIC mys_bits_t mys_bits_u64(uint64_t n) { return mys_bits(&n, sizeof(uint64_t)); } /* printf("%s\n", mys_bits_u64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_u32(uint32_t n) { return mys_bits(&n, sizeof(uint32_t)); } /* printf("%s\n", mys_bits_u32(1).bits); */
MYS_STATIC mys_bits_t mys_bits_i64(int64_t  n) { return mys_bits(&n, sizeof(int64_t )); } /* printf("%s\n", mys_bits_i64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_i32(int32_t  n) { return mys_bits(&n, sizeof(int32_t )); } /* printf("%s\n", mys_bits_i32(1).bits); */
MYS_STATIC mys_bits_t mys_bits_f64(double   n) { return mys_bits(&n, sizeof(double  )); } /* printf("%s\n", mys_bits_f64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_f32(float    n) { return mys_bits(&n, sizeof(float   )); } /* printf("%s\n", mys_bits_f32(1).bits); */
