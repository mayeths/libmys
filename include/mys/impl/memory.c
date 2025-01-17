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
#include "../_config.h"
#include "../errno.h"
#include "../assert.h"
#include "../mpistubs.h"
#include "../pmparser.h"
#include "../os.h"
#include "../string.h"
#include "../memory.h"
#include "uthash_hash.h"

#include <sys/stat.h>

mys_arena_t mys_predefined_arena_log = MYS_ARENA_INITIALIZER("mys_log");
mys_arena_t mys_predefined_arena_pool = MYS_ARENA_INITIALIZER("mys_pool");
mys_arena_t mys_predefined_arena_debug = MYS_ARENA_INITIALIZER("mys_debug");
mys_arena_t mys_predefined_arena_format = MYS_ARENA_INITIALIZER("mys_format");
mys_arena_t mys_predefined_arena_commgroup = MYS_ARENA_INITIALIZER("mys_commgroup");
mys_arena_t mys_predefined_arena_os = MYS_ARENA_INITIALIZER("mys_os");
mys_arena_t mys_predefined_arena_stat = MYS_ARENA_INITIALIZER("mys_statistic");
mys_arena_t mys_predefined_arena_str = MYS_ARENA_INITIALIZER("mys_string");
mys_arena_t mys_predefined_arena_trace = MYS_ARENA_INITIALIZER("mys_trace");
mys_arena_t mys_predefined_arena_user = MYS_ARENA_INITIALIZER("user");

#define MYS_MAX_REGISTERED_ARENA 64

#define MYS_MAX_ARENA_TRACE 16
typedef struct mys_arena_debugger_t {
    void *ptr;
    size_t size;
    void *backtrace[MYS_MAX_ARENA_TRACE];
    int ntrace;
    _mys_UT_hash_handle hh;
} mys_arena_debugger_t;

MYS_STATIC void _mys_arena_debug_get_stack(mys_arena_debugger_t *node, mys_string_t *str)
{
    const char *self_exe = mys_procname();
    struct stat self_st;
    stat(self_exe, &self_st);

    mys_string_fmt(str, "    %p (%zu bytes):\n", node->ptr, node->size);

    for (int i = 2; i < node->ntrace; i++) {
        const char *target = self_exe;
        void *relative = node->backtrace[i];
        mys_procmaps_t *self = mys_pmparser_self();
        mys_procmap_t *map = self->head;
        while (map) {
            if (node->backtrace[i] >= map->addr_start && node->backtrace[i] < map->addr_end) {
                struct stat st;
                if (stat(map->pathname, &st) == 0) {
                    bool is_self_exe = (st.st_ino == self_st.st_ino && st.st_dev == self_st.st_dev);
                    if (!is_self_exe) {
                        target = map->pathname;
                        relative = (void *)((uintptr_t)node->backtrace[i] - (uintptr_t)map->addr_start);
                    }
                }
                break;
            }
            map = map->next;
        }
        mys_prun_t run = mys_prun_create2("addr2line -e %s %p", target, relative);
        // DLOG(0, "cmd: %s", run.cmd);
        // mys_string_fmt(str, " %s;", run.out);
        mys_string_fmt(str, "        %s", run.out);
        if (i < node->ntrace - 1)
            mys_string_fmt(str, "\n");
        mys_prun_destroy(&run);
    }
}

MYS_STATIC mys_arena_debugger_t *_mys_arena_debug_find(mys_arena_debugger_t **head, void *ptr)
{
    mys_arena_debugger_t *node = NULL;
    _HASH_FIND_PTR(*head, &ptr, node);
    return node;
}

MYS_STATIC mys_arena_debugger_t *_mys_arena_debug_insert(mys_arena_debugger_t **head, void *ptr, size_t size)
{
    mys_arena_debugger_t *node = (mys_arena_debugger_t *)malloc(sizeof(mys_arena_debugger_t));
    node->ptr = ptr;
    node->size = size;
    node->ntrace = backtrace(node->backtrace, MYS_MAX_ARENA_TRACE);
    mys_arena_debugger_t *old_node = _mys_arena_debug_find(head, ptr);
    _HASH_ADD_PTR(*head, ptr, node);
    return node;
}

MYS_STATIC void _mys_arena_debug_delete(mys_arena_debugger_t **head, void *ptr, size_t size)
{
    mys_arena_debugger_t *node = _mys_arena_debug_find(head, ptr);
    if (node != NULL) {
        if (node->size != size) {
            mys_string_t *str = mys_string_create();
            mys_string_fmt(str, "pointer %p alloc %zu bytes but free %zu bytes. backtrace:\n", ptr, node->size, size);
            _mys_arena_debug_get_stack(node, str);
            FAILED("%s", str->text);
            mys_string_destroy(&str);
        }
        _HASH_DEL(*head, node);
        free(node);
    }
}

MYS_STATIC void _mys_arena_debug_delete_all(mys_arena_debugger_t **head)
{
    mys_arena_debugger_t *node = NULL;
    mys_arena_debugger_t *tmp = NULL;
    _HASH_ITER(hh, *head, node, tmp) {
        _HASH_DEL(*head, node);
        free(node);
    }
}

typedef struct _mys_memory_G_t {
    mys_mutex_t lock;
    // mys_arena_t **registered_arenas;
    mys_arena_t *registered_arenas[MYS_MAX_REGISTERED_ARENA];
    size_t arena_size;
    size_t arena_capacity;
} _mys_memory_G_t;

static _mys_memory_G_t _mys_memory_G = {
    .lock = MYS_MUTEX_INITIALIZER,
    // .registered_arenas = NULL,
    .registered_arenas = {NULL},
    .arena_size = 0,
    .arena_capacity = MYS_MAX_REGISTERED_ARENA,
};

MYS_STATIC void _mys_ensure_register_arena(mys_arena_t *arena)
{
    AS_NE_PTR(arena, NULL);
    if (arena->_registered)
        return;
    mys_mutex_lock(&_mys_memory_G.lock);
    {
        ASX_NE_SIZET(_mys_memory_G.arena_size, _mys_memory_G.arena_capacity,
            "mys_memory_G support maximum %zu arenas only", _mys_memory_G.arena_capacity);
        // if (_mys_memory_G.arena_size == _mys_memory_G.arena_capacity) {
        //     size_t new_cap = (_mys_memory_G.arena_capacity == 0) ? 16 : _mys_memory_G.arena_capacity * 2;
        //     void *p = realloc(_mys_memory_G.registered_arenas, new_cap * sizeof(mys_arena_t *));
        //     AS_NE_PTR(p, NULL);
        //     _mys_memory_G.registered_arenas = (mys_arena_t **)p;
        //     _mys_memory_G.arena_capacity = new_cap;
        // }
        _mys_memory_G.registered_arenas[_mys_memory_G.arena_size] = arena;
        _mys_memory_G.arena_size += 1;
        arena->_registered = true;
    }
    mys_mutex_unlock(&_mys_memory_G.lock);
}

MYS_STATIC void _mys_deregister_arena(mys_arena_t *arena)
{
    AS_NE_PTR(arena, NULL);
    if (!arena->_registered)
        return;
    mys_mutex_lock(&_mys_memory_G.lock);
    {
        for (size_t i = 0; i < _mys_memory_G.arena_size; i++) {
            if (_mys_memory_G.registered_arenas[i] == arena) {
                _mys_memory_G.registered_arenas[i] = NULL;
                for (size_t j = i + 1; j < _mys_memory_G.arena_size; j++) {
                    if (_mys_memory_G.registered_arenas[j] == NULL)
                        break;
                    _mys_memory_G.registered_arenas[j - 1] = _mys_memory_G.registered_arenas[j];
                }
                break;
            }
        }
        _mys_memory_G.arena_size -= 1;
        arena->_registered = false;
    }
    mys_mutex_unlock(&_mys_memory_G.lock);
}

MYS_PUBLIC mys_arena_t *mys_arena_create(const char *name)
{
    mys_arena_t *arena = (mys_arena_t *)malloc(sizeof(mys_arena_t));
    if (arena == NULL)
        return NULL;
    strncpy(arena->name, name, sizeof(arena->name));
    arena->name[sizeof(arena->name) - 1] = '\0';
    arena->peak = 0;
    arena->alive = 0;
    arena->freed = 0;
    arena->total = 0;
    arena->_registered = false;
    arena->_enable_debug = false;
    arena->_debug_trace = NULL;
    arena->_total_count = 0;
    arena->_alive_count = 0;
    arena->_freed_count = 0;
    _mys_ensure_register_arena(arena);
    return arena;
}

MYS_PUBLIC void mys_arena_destroy(mys_arena_t **arena)
{
    AS_NE_PTR(arena, NULL);
    if (*arena == NULL)
        return;
    if ((*arena)->_enable_debug) {
        mys_arena_debugger_t **head = (mys_arena_debugger_t **)&(*arena)->_debug_trace;
        _mys_arena_debug_delete_all(head);
    }
    _mys_deregister_arena(*arena);
    free(*arena);
    *arena = NULL;
}

MYS_PUBLIC void mys_arena_set_debug(mys_arena_t *arena, bool val)
{
    if (val) {
        mys_pmparser_init();
    }
    arena->_enable_debug = val;
}

MYS_PUBLIC void mys_arena_print_leaked(mys_arena_t *arena, size_t max_print)
{
    mys_string_t *str = mys_string_create();

    if (arena->_enable_debug) {
        mys_arena_debugger_t *head = (mys_arena_debugger_t *)arena->_debug_trace;
        mys_arena_debugger_t *node = NULL;
        mys_arena_debugger_t *tmp = NULL;
        size_t num_hash;
        num_hash = (size_t)_HASH_COUNT(head);
        AS_EQ_SIZET(num_hash, arena->_alive_count);
        mys_string_fmt(str, "arena %s alloc %zu, freed %zu, leaked %zu pointers:\n",
            arena->name, arena->_total_count, arena->_freed_count, arena->_alive_count);
        size_t count = 0;

        _HASH_ITER(hh, head, node, tmp) {
            _mys_arena_debug_get_stack(node, str);
            mys_string_fmt(str, "\n");
            count += 1;
            if (count > max_print)
                break;
        }

        if (count == 0) {
            if (arena->alive > 0) {
                mys_string_fmt(str,
                    "    no leak pointer is found, "
                    "but %zu bytes are still alive,"
                    "please check your free calls.",
                arena->alive);
            } else {
                mys_string_fmt(str, "    no leak pointer is found, %zu byte alive.", arena->alive);
            }
        }
    } else {
        mys_string_fmt(str, "arena %s is not set to debug mode", arena->name);
    }

    DLOG_SELF("%s", str->text);
    mys_string_destroy(&str);

}

MYS_PUBLIC mys_arena_t *mys_arena_next_leaked(mys_arena_t *pivot)
{
    mys_arena_t *leaked = NULL;
    mys_mutex_lock(&_mys_memory_G.lock);
    {
        bool passed_pivot = (pivot == NULL) ? true : false;
        for (size_t i = 0; i < _mys_memory_G.arena_size; i++) {
            mys_arena_t *arena = _mys_memory_G.registered_arenas[i];
            bool leaked = arena->alive != 0;
            if (passed_pivot && leaked) {
                leaked = arena;
                break;
            }
            if (arena == pivot) {
                passed_pivot = true;
            }
        }
    }
    mys_mutex_unlock(&_mys_memory_G.lock);
    return leaked;
}

#define MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size) do {       \
    _mys_ensure_register_arena(arena);                      \
    arena->alive += (size);                                 \
    arena->total += (size);                                 \
    AS_EQ_SIZET(arena->total, arena->alive + arena->freed); \
    if (arena->peak < arena->alive)                         \
        arena->peak = arena->alive;                         \
} while (0)

#define DEBUG_INSERT(arena, ptr, size) do {                                  \
    mys_arena_debugger_t **head = (mys_arena_debugger_t **)&arena->_debug_trace; \
    if (arena->_enable_debug)                                                \
        arena->_total_count += 1;                                            \
        arena->_alive_count += 1;                                            \
        _mys_arena_debug_insert(head, ptr, size);                            \
} while (0)

#define DEBUG_DELETE(arena, ptr, size) do {                                        \
    mys_arena_debugger_t **head = (mys_arena_debugger_t **)&arena->_debug_trace; \
    if (arena->_enable_debug)                                                \
        arena->_alive_count -= 1;                                            \
        arena->_freed_count += 1;                                            \
        _mys_arena_debug_delete(head, ptr, size);                            \
} while (0)

MYS_PUBLIC void* mys_malloc2(mys_arena_t *arena, size_t size)
{
    void *p = malloc(size);
    if (p != NULL) {
        MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size);
        DEBUG_INSERT(arena, p, size);
    }
    return p;
}

MYS_PUBLIC void* mys_calloc2(mys_arena_t *arena, size_t count, size_t size)
{
    void *p = calloc(count, size);
    if (p != NULL) {
        MAKE_GCC_HAPPY_ALLOC_RECORD(arena, count * size);
        DEBUG_INSERT(arena, p, count * size);
    }
    return p;
}

MYS_PUBLIC void* mys_aligned_alloc2(mys_arena_t *arena, size_t alignment, size_t size)
{
    void *p = NULL;
#if defined(_ISOC11_SOURCE) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)) // C11
    p = aligned_alloc(alignment, size);
#elif defined(_MSC_VER) // Microsoft
    p = _aligned_malloc(size, alignment);
#elif defined(__APPLE__) || defined(__MACH__) || defined(__GNUC__) || defined(__clang__) // POSIX
    if (posix_memalign(&p, alignment, size) != 0)
        p = NULL;
#else
    #error Unsupported
#endif
    if (p != NULL) {
        MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size);
        DEBUG_INSERT(arena, p, size);
    }
    return p;
}

MYS_PUBLIC void* mys_realloc2(mys_arena_t *arena, void* ptr, size_t size, size_t _old_size)
{
    void *p = realloc(ptr, size);
    if (p != NULL) {
        mys_free_record(arena, _old_size);
        MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size);
        DEBUG_DELETE(arena, ptr, _old_size);
        DEBUG_INSERT(arena, p, size);
    }
    return p;
}

MYS_PUBLIC void mys_free2(mys_arena_t *arena, void* ptr, size_t size)
{
    if (ptr != NULL) {
        mys_free_record(arena, size);
        DEBUG_DELETE(arena, ptr, size);
    }
    free(ptr);
}

MYS_PUBLIC void mys_alloc_record(mys_arena_t *arena, size_t size)
{
    MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size);
}

MYS_PUBLIC void mys_free_record(mys_arena_t *arena, size_t size)
{
    AS_NE_PTR(arena, NULL);
    _mys_ensure_register_arena(arena);
    AS_GE_SIZET(arena->alive, size);
    arena->alive -= size;
    arena->freed += size;
    AS_EQ_SIZET(arena->total, arena->alive + arena->freed);
}

// MYS_PUBLIC void* mys_malloc(size_t size)
// {
//     return mys_malloc2(mys_arena_std, size);
// }

// MYS_PUBLIC void* mys_calloc(size_t count, size_t size)
// {
//     return mys_calloc2(mys_arena_std, count, size);
// }

// MYS_PUBLIC void* mys_aligned_alloc(size_t alignment, size_t size)
// {
//     return mys_aligned_alloc2(mys_arena_std, alignment, size);
// }

// MYS_PUBLIC void* mys_realloc(void* ptr, size_t size)
// {
//     (void)ptr;
//     (void)size;
//     THROW_NOT_IMPL();
//     // FIXME: we require passing _old_size to mys_memory_reallocator, like c++ allocator do.
//     // This can help us don't have to trace ptr size internally.
//     // If you want to trace it, then trace it youself.
//     // Therefore, mys_arena_std should trace size by itself, instead of asking mys_realloc2 to trace
//     // return mys_realloc2(mys_arena_std, ptr, size, _old_size);
//     return NULL;
// }

// MYS_PUBLIC void mys_free(void* ptr)
// {
//     (void)ptr;
//     THROW_NOT_IMPL();
//     // FIXME: we require passing size to mys_free2, like c++ allocator do.
//     // This can help us don't have to trace ptr size internally.
//     // If you want to trace it, then trace it youself.
//     // Therefore, mys_arena_std should trace size by itself, instead of asking mys_free2 to trace
//     // return mys_free2(mys_arena_std, ptr, size);
// }

#if defined(OS_LINUX) && defined(MYS_ENABLE_SHM)
struct _mys_shm_G_t {
    bool inited;
    mys_mutex_t lock;
    int program_id;
    size_t counter;
};

static struct _mys_shm_G_t _mys_shm_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .program_id = 0,
    .counter = 0,
};

static void _mys_shm_G_init()
{
    if (_mys_shm_G.inited == true)
        return;
    mys_mutex_lock(&_mys_shm_G.lock);
    int myrank;
    mys_MPI_Comm_rank(mys_MPI_COMM_WORLD, &myrank);
    {
        if (myrank == 0)
            _mys_shm_G.program_id = getpid();
        _mys_MPI_Bcast(&_mys_shm_G.program_id, 1, mys_MPI_INT, 0, mys_MPI_COMM_WORLD);
    }
    _mys_shm_G.inited = true;
    mys_mutex_unlock(&_mys_shm_G.lock);
}

MYS_PUBLIC mys_shm_t mys_alloc_shared_memory(int owner_rank, size_t size)
{
    _mys_shm_G_init();
    mys_mutex_lock(&_mys_shm_G.lock);
    int myrank;
    mys_MPI_Comm_rank(mys_MPI_COMM_WORLD, &myrank);
    mys_shm_t shm;
    snprintf(shm._name, sizeof(shm._name), "/mys_%d_%zu", _mys_shm_G.program_id, _mys_shm_G.counter);
    _mys_shm_G.counter += 1;
    if (myrank == owner_rank) {
        shm._fd = shm_open(shm._name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        shm._size = size;
        ftruncate(shm._fd, shm._size);
        shm.mem = mmap(NULL, shm._size, PROT_READ | PROT_WRITE, MAP_SHARED, shm._fd, 0);
        memset(shm.mem, 0, shm._size);
        mys_memory_smp_mb();
        mys_MPI_Barrier(mys_MPI_COMM_WORLD);
    } else {
        mys_MPI_Barrier(mys_MPI_COMM_WORLD);
        shm._fd = shm_open(shm._name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        shm.mem = mmap(NULL, shm._size, PROT_READ | PROT_WRITE, MAP_SHARED, shm._fd, 0);
    }
    mys_mutex_unlock(&_mys_shm_G.lock);
    return shm;
}

MYS_PUBLIC void mys_free_shared_memory(mys_shm_t *shm)
{
    _mys_shm_G_init();
    mys_mutex_lock(&_mys_shm_G.lock);
    {
        munmap(shm->mem, shm->_size);
        close(shm->_fd);
        shm_unlink(shm->_name);
        shm->mem = NULL;
    }
    mys_mutex_unlock(&_mys_shm_G.lock);
}

#endif

MYS_PUBLIC mys_bits_t mys_bits(const void *data, size_t size)
{
    mys_bits_t res;
    memset(&res, 0, sizeof(mys_bits_t));
    const uint8_t *bytes = (const uint8_t *)data;
    int count = 0;
    for (int i = size - 1; i >= 0; --i) { // begin from high bytes
        for (int j = 7; j >= 0; --j) { // begin from high bits
            unsigned int bit = (bytes[i] >> j) & 1;
            res.bits[count++] = bit ? '1' : '0';
        }
    }
    return res;
}

MYS_PUBLIC void mys_cache_flush(size_t nbytes)
{
    char * volatile arr = (char *)malloc(nbytes * sizeof(char));
    memset(arr, 0, nbytes);
    for (size_t i = 1; i < nbytes; i++) {
        arr[i] = i | (arr[i - 1]);
    }
    volatile char result;
    result = arr[nbytes - 1];
    result = (char)(uint64_t)(&arr[(uint64_t)result]);
    mys_atomic_fence(MYS_ATOMIC_RELEASE);
    free(arr);
}
