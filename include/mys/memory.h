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
    size_t peak;  // memory bytes that peak alive
    size_t alive; // memory bytes that being used
    size_t freed; // memory bytes that freed
    size_t total; // memory bytes that total allocated
    bool _reged;
} mys_arena_t;

// MYS_PUBLIC mys_arena_t* mys_arena_std; // The arena used by mys_malloc, mys_calloc, ..., mys_free
MYS_PUBLIC mys_arena_t* mys_arena_log; // The arena used by mys_log
MYS_PUBLIC mys_arena_t* mys_arena_pool; // The arena used by mys_pool
MYS_PUBLIC mys_arena_t* mys_arena_debug; // The arena used by mys_debug
MYS_PUBLIC mys_arena_t* mys_arena_format; // The arena used by mys_format
MYS_PUBLIC mys_arena_t* mys_arena_commgroup; // The arena used by mys_commgroup
MYS_PUBLIC mys_arena_t* mys_arena_os; // The arena used by mys_os
MYS_PUBLIC mys_arena_t* mys_arena_stat; // The arena used by mys_statistic
MYS_PUBLIC mys_arena_t* mys_arena_str; // The arena used by mys_string
MYS_PUBLIC mys_arena_t* mys_arena_trace; // The arena used by mys_trace
MYS_PUBLIC mys_arena_t* mys_arena_user; // The predefined arena available for user use

/**
 * @brief Create a new memory arena with the specified name.
 *
 * This function allocates and initializes a new memory arena with the given name.
 *
 * @param name The name of the new memory arena.
 * @return A pointer to the newly created memory arena, or NULL if creation fails.
 */
MYS_PUBLIC mys_arena_t *mys_arena_create(const char *name);
/**
 * @brief Destroy a memory arena and free its resources.
 *
 * This function destroys the specified memory arena and frees all associated resources.
 * The pointer to the arena is set to NULL after destruction.
 *
 * @param arena A double pointer to the memory arena to be destroyed.
 */
MYS_PUBLIC void mys_arena_destroy(mys_arena_t **arena);
/**
 * @brief Find the next leaked memory arena after a given pivot.
 *
 * This function searches for the next leaked memory arena after the specified
 * pivot arena. If the pivot is NULL, it starts searching from the beginning.
 *
 * @param pivot The memory arena to start the search after. If NULL, starts from the beginning.
 * @return A pointer to the next leaked memory arena, or NULL if no more leaked arenas are found.
 */
MYS_PUBLIC mys_arena_t *mys_arena_next_leaked(mys_arena_t *pivot);

MYS_PUBLIC void* mys_malloc2(mys_arena_t *arena, size_t size) MYS_ATTR_MALLOC;
MYS_PUBLIC void* mys_calloc2(mys_arena_t *arena, size_t count, size_t size) MYS_ATTR_MALLOC;
MYS_PUBLIC void* mys_aligned_alloc2(mys_arena_t *arena, size_t alignment, size_t size);
MYS_PUBLIC void* mys_realloc2(mys_arena_t *arena, void* ptr, size_t size, size_t _old_size);
MYS_PUBLIC void mys_free2(mys_arena_t *arena, void* ptr, size_t _size);

// For external use (using mys_arena_std)
// MYS_PUBLIC void* mys_malloc(size_t size);
// MYS_PUBLIC void* mys_calloc(size_t count, size_t size);
// MYS_PUBLIC void* mys_aligned_alloc(size_t alignment, size_t size);
// MYS_PUBLIC void* mys_realloc(void* ptr, size_t size);
// MYS_PUBLIC void mys_free(void* ptr);

/* Cache clean */
MYS_PUBLIC void mys_cache_flush(size_t nbytes);

#if defined(OS_LINUX) && defined(MYS_ENABLE_SHM)
typedef struct mys_shm_t {
    void *mem;
    size_t _size;
    int _fd;
    char _name[NAME_MAX];
} mys_shm_t;

MYS_PUBLIC mys_shm_t mys_alloc_shared_memory(int owner_rank, size_t size);
MYS_PUBLIC void mys_free_shared_memory(mys_shm_t *shm);
#endif


typedef struct mys_bits_t {
    char bits[64 + 1];
} mys_bits_t;

MYS_PUBLIC mys_bits_t mys_bits(const void *data, size_t size);
MYS_STATIC mys_bits_t mys_bits_u64(uint64_t n) { return mys_bits(&n, sizeof(uint64_t)); } /* printf("%s\n", mys_bits_u64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_u32(uint32_t n) { return mys_bits(&n, sizeof(uint32_t)); } /* printf("%s\n", mys_bits_u32(1).bits); */
MYS_STATIC mys_bits_t mys_bits_i64(int64_t  n) { return mys_bits(&n, sizeof(int64_t )); } /* printf("%s\n", mys_bits_i64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_i32(int32_t  n) { return mys_bits(&n, sizeof(int32_t )); } /* printf("%s\n", mys_bits_i32(1).bits); */
MYS_STATIC mys_bits_t mys_bits_f64(double   n) { return mys_bits(&n, sizeof(double  )); } /* printf("%s\n", mys_bits_f64(1).bits); */
MYS_STATIC mys_bits_t mys_bits_f32(float    n) { return mys_bits(&n, sizeof(float   )); } /* printf("%s\n", mys_bits_f32(1).bits); */
