/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
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
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#ifdef KERNEL_LINUX
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "_config.h"
#include "macro.h"

// if (arena->alive != 0) {
//     WLOG_SELF("Memory leaked happened in %s size %zu.", arena->name, arena->alive);
//     mys_arena_print_leaked(arena, 10);
// }

typedef struct mys_arena_debugger_t mys_arena_debugger_t;

typedef struct mys_arena_t {
    char name[32];
    size_t peak;  // memory bytes that peak alive
    size_t alive; // memory bytes that being used
    size_t freed; // memory bytes that freed
    size_t total; // memory bytes that total allocated
    bool _registered; // internal use
    bool _enable_debug; // internal use
    mys_arena_debugger_t *_debug_trace; // internal use
    size_t _total_count; // internal use
    size_t _alive_count; // internal use
    size_t _freed_count; // internal use
} mys_arena_t;

#define MYS_ARENA_INITIALIZER(name) { /*name=*/name, /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false, /*_enable_debug*/false, /*_debug_trace*/NULL, /*_total_count*/0, /*_alive_count*/0, /*_freed_count*/0 }

MYS_PUBVAR mys_arena_t mys_predefined_arena_log;
MYS_PUBVAR mys_arena_t mys_predefined_arena_pool;
MYS_PUBVAR mys_arena_t mys_predefined_arena_debug;
MYS_PUBVAR mys_arena_t mys_predefined_arena_format;
MYS_PUBVAR mys_arena_t mys_predefined_arena_commgroup;
MYS_PUBVAR mys_arena_t mys_predefined_arena_os;
MYS_PUBVAR mys_arena_t mys_predefined_arena_stat;
MYS_PUBVAR mys_arena_t mys_predefined_arena_str;
MYS_PUBVAR mys_arena_t mys_predefined_arena_trace;
MYS_PUBVAR mys_arena_t mys_predefined_arena_user;

#define MYS_ARENA_LOG ((mys_arena_t *)&mys_predefined_arena_log) // The arena used by mys_log
#define MYS_ARENA_POOL ((mys_arena_t *)&mys_predefined_arena_pool) // The arena used by mys_pool
#define MYS_ARENA_DEBUG ((mys_arena_t *)&mys_predefined_arena_debug) // The arena used by mys_debug
#define MYS_ARENA_FORMAT ((mys_arena_t *)&mys_predefined_arena_format) // The arena used by mys_format
#define MYS_ARENA_COMMGROUP ((mys_arena_t *)&mys_predefined_arena_commgroup) // The arena used by mys_commgroup
#define MYS_ARENA_OS ((mys_arena_t *)&mys_predefined_arena_os) // The arena used by mys_os
#define MYS_ARENA_STAT ((mys_arena_t *)&mys_predefined_arena_stat) // The arena used by mys_statistic
#define MYS_ARENA_STR ((mys_arena_t *)&mys_predefined_arena_str) // The arena used by mys_string
#define MYS_ARENA_TRACE ((mys_arena_t *)&mys_predefined_arena_trace) // The arena used by mys_trace
#define MYS_ARENA_USER ((mys_arena_t *)&mys_predefined_arena_user) // The predefined arena available for user use

/**
 * @brief Create a new memory arena with the specified name.
 *
 * This function allocates and initializes a new memory arena with the given name.
 *
 * @param name The name of the new memory arena.
 * @return A pointer to the newly created memory arena, or NULL if creation fails.
 * 
 * @note Normally, you may want to check `ASX_EQ_SIZET(arena->alive, 0, "Internal error: memory leaked happened in %s.", arena->name);`
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
MYS_PUBLIC void mys_arena_set_debug(mys_arena_t *arena, bool val);
MYS_PUBLIC void mys_arena_print_leaked(mys_arena_t *arena, size_t max_print);
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

MYS_PUBLIC void mys_alloc_record(mys_arena_t *arena, size_t size);
MYS_PUBLIC void mys_free_record(mys_arena_t *arena, size_t size);

// For external use (using mys_arena_std)
// MYS_PUBLIC void* mys_malloc(size_t size);
// MYS_PUBLIC void* mys_calloc(size_t count, size_t size);
// MYS_PUBLIC void* mys_aligned_alloc(size_t alignment, size_t size);
// MYS_PUBLIC void* mys_realloc(void* ptr, size_t size);
// MYS_PUBLIC void mys_free(void* ptr);

/* Cache clean */
MYS_PUBLIC void mys_cache_flush(size_t nbytes);

#if defined(KERNEL_LINUX) && defined(MYS_ENABLE_SHM)
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
