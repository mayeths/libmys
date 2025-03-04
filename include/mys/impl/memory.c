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
#include "../mpistubs.h"
#include "../memory.h"

mys_arena_t mys_predefined_arena_log = { /*name=*/"mys_log", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_pool = { /*name=*/"mys_pool", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_debug = { /*name=*/"mys_debug", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_format = { /*name=*/"mys_format", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_commgroup = { /*name=*/"mys_commgroup", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_os = { /*name=*/"mys_os", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_stat = { /*name=*/"mys_statistic", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_str = { /*name=*/"mys_string", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_trace = { /*name=*/"mys_trace", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };
mys_arena_t mys_predefined_arena_user = { /*name=*/"user", /*peak=*/0, /*alive=*/0, /*freed=*/0, /*total=*/0, /*_registered=*/false };

#define MYS_MAX_REGISTERED_ARENA 64

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
    _mys_ensure_register_arena(arena);
    return arena;
}

MYS_PUBLIC void mys_arena_destroy(mys_arena_t **arena)
{
    AS_NE_PTR(arena, NULL);
    if (*arena == NULL)
        return;
    _mys_deregister_arena(*arena);
    free(*arena);
    *arena = NULL;
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
    arena->alive += size;                                   \
    arena->total += size;                                   \
    AS_EQ_SIZET(arena->total, arena->alive + arena->freed); \
    if (arena->peak < arena->alive)                         \
        arena->peak = arena->alive;                         \
} while (0)

MYS_PUBLIC void* mys_malloc2(mys_arena_t *arena, size_t size)
{
    void *p = malloc(size);
    if (p != NULL) {
        MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size);
    }
    return p;
}

MYS_PUBLIC void* mys_calloc2(mys_arena_t *arena, size_t count, size_t size)
{
    void *p = calloc(count, size);
    if (p != NULL) {
        MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size);
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
    }
    return p;
}

MYS_PUBLIC void* mys_realloc2(mys_arena_t *arena, void* ptr, size_t size, size_t _old_size)
{
    void *p = realloc(ptr, size);
    if (p != NULL) {
        mys_free_record(arena, _old_size);
        MAKE_GCC_HAPPY_ALLOC_RECORD(arena, size);
    }
    return p;
}

MYS_PUBLIC void mys_free2(mys_arena_t *arena, void* ptr, size_t size)
{
    if (ptr != NULL) {
        mys_free_record(arena, size);
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
