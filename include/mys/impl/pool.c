#include "_private.h"
#include "../pool.h"
#include "../memory.h"

struct mys_pool_object_meta_t;
struct mys_pool_object_t;
struct mys_pool_block_t;
struct mys_pool_t;

typedef struct mys_pool_object_meta_t {
    struct mys_pool_object_t* next;
    struct mys_pool_block_t* block;
} mys_pool_object_meta_t;

/*
    +--------------+-----+--------+
    |    Object    | Pad |  Meta  |
    +--------------+-----+--------+
     ^^^^^^^^^^^^^^
        robj_size
     ^^^^^^^^^^^^^^^^^^^^
        pobj_size
     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        mobj_size
*/
typedef struct mys_pool_object_t {
    // uint8_t object[];
    // uint8_t padding[];
    // mys_pool_object_meta_t meta;
} mys_pool_object_t;

typedef struct mys_pool_block_t {
    struct mys_pool_block_t* prev_block;
    struct mys_pool_block_t* next_block;
    size_t capacity;
    size_t free;
    size_t used_count;
    uint8_t* objects;
} mys_pool_block_t;

typedef struct mys_pool_t {
    size_t robj_size;
    size_t pobj_size;
    size_t mobj_size;
    size_t block_capacity;
    int straregy;
    mys_pool_block_t* blocks;
    mys_pool_object_t* free_objects; // objects that free to use
} mys_pool_t;

static mys_pool_object_t *_mys_pool_get_object(mys_pool_t *pool, mys_pool_block_t *block, size_t i)
{
    return (mys_pool_object_t *)(block->objects + pool->mobj_size * i);
}

static mys_pool_object_meta_t* _mys_pool_get_object_meta(mys_pool_t *pool, mys_pool_object_t *object)
{
    if (pool == NULL || object == NULL)
        return NULL;
    uint8_t *memory = (uint8_t *)object;
    mys_pool_object_meta_t *meta = (mys_pool_object_meta_t *)(memory + pool->pobj_size);
    return meta;
}

MYS_PUBLIC mys_pool_t* mys_pool_create(size_t object_size)
{
    return mys_pool_create2(object_size, 64, MYS_POOL_DEFAULT);
}

MYS_PUBLIC mys_pool_t *mys_pool_create2(size_t object_size, size_t initial_capacity, int pool_straregy)
{
    if (initial_capacity == 0)
        return NULL;

    mys_pool_t *pool = (mys_pool_t *)mys_malloc2(mys_arena_pool, sizeof(mys_pool_t));
    if (!pool)
        return NULL;

    pool->robj_size = object_size;
    // pool->pobj_size =
    //     (pool->robj_size > 8) ? MYS_ALIGN_UP(pool->robj_size, 8) :
    //     (pool->robj_size > 4) ? pool->pobj_size = 8 :
    //     4;
    pool->pobj_size = MYS_ALIGN_UP(pool->robj_size, 8);
    pool->mobj_size = pool->pobj_size + MYS_ALIGN_UP(sizeof(mys_pool_object_meta_t), 8);
    pool->block_capacity = initial_capacity;
    pool->straregy = pool_straregy;
    pool->blocks = NULL;
    pool->free_objects = NULL;

    return pool;
}


static void allocate_block(mys_pool_t* pool)
{
    mys_pool_block_t* block = (mys_pool_block_t *)mys_malloc2(mys_arena_pool, sizeof(mys_pool_block_t));
    if (block == NULL)
        return;

    block->capacity = pool->block_capacity;
    block->free = pool->block_capacity;
    block->used_count = 0;
    block->objects = (uint8_t *)mys_aligned_alloc2(mys_arena_pool, sysconf(_SC_PAGE_SIZE), block->capacity * pool->mobj_size);
    if (block->objects == NULL) {
        mys_free2(mys_arena_pool, block, sizeof(mys_pool_block_t));
        return;
    }
    for (size_t i = 0; i < block->capacity; i++) {
        mys_pool_object_t *object = get_object(pool, block, i);
        mys_pool_object_meta_t *meta = _mys_pool_get_object_meta(pool, object);
        meta->block = block;
        meta->next = pool->free_objects;
        pool->free_objects = object;
    }

    mys_pool_block_t* next_block = pool->blocks;
    block->prev_block = NULL;
    block->next_block = next_block;
    if (next_block != NULL) next_block->prev_block = block;
    pool->blocks = block;
    pool->block_capacity = block->capacity * 2;
}

MYS_STATIC void deallocate_block(mys_pool_t* pool, mys_pool_block_t *block)
{
    AS_NE_PTR(pool, NULL);
    AS_NE_PTR(block, NULL);

    mys_pool_block_t *block1 = block->prev_block;
    mys_pool_block_t *block2 = block;
    mys_pool_block_t *block3 = block->next_block;
    if (block1 != NULL) block1->next_block = block3;
    if (block2 != NULL) block2->prev_block = NULL;
    if (block2 != NULL) block2->next_block = NULL;
    if (block3 != NULL) block3->prev_block = block1;

    if (pool->blocks == block2) pool->blocks = block3;

    // remove object belong to block2 in freelist
    mys_pool_object_t *prev_object = NULL;
    mys_pool_object_t *object = pool->free_objects;
    mys_pool_object_t *next_object = NULL;
    while (object != NULL) {
        mys_pool_object_meta_t *meta = _mys_pool_get_object_meta(pool, object);
        next_object = meta->next;
        {
            uint8_t *memory_start = block2->objects;
            uint8_t *memory_end = block2->objects + pool->mobj_size * block2->capacity;
            bool is_blong_to_block2 = ((uint8_t *)object >= memory_start) && ((uint8_t *)object < memory_end);
            if (is_blong_to_block2) {
                if (prev_object != NULL) {
                    mys_pool_object_meta_t *prev_meta = _mys_pool_get_object_meta(pool, prev_object);
                    prev_meta->next = next_object;
                }
                if (pool->free_objects == object) {
                    pool->free_objects = next_object;
                }
            }
            if (!is_blong_to_block2) {
                // this object is not belong to block2, so it's still valid in freelist
                prev_object = object;
            }
            object = next_object;
        }
    }

    AS_NE_PTR(block2->objects, NULL);
    mys_free2(mys_arena_pool, block2->objects, block2->capacity * pool->mobj_size);
    mys_free2(mys_arena_pool, block2, sizeof(mys_pool_block_t));
}

MYS_PUBLIC void mys_pool_destroy(mys_pool_t **pool)
{
    AS_NE_PTR(pool, NULL);
    if (*pool == NULL)
        return;
    mys_pool_block_t* block = (*pool)->blocks;
    while (block != NULL) {
        mys_pool_block_t* next_block = block->next_block;
        deallocate_block((*pool), block);
        block = next_block;
    }
    mys_free2(mys_arena_pool, *pool, sizeof(mys_pool_t));
    *pool = NULL;
}

MYS_PUBLIC void *mys_pool_acquire(mys_pool_t* pool)
{
    if (pool->free_objects == NULL) {
        allocate_block(pool);
        if (pool->free_objects == NULL) {
            return NULL;
        }
    }

    mys_pool_object_t *object = pool->free_objects;
    mys_pool_object_meta_t *meta = _mys_pool_get_object_meta(pool, object);
    pool->free_objects = meta->next;
    meta->next = NULL;
    meta->block->free -= 1;
    meta->block->used_count += 1;
    return (void *)object;
}

MYS_PUBLIC void mys_pool_release(mys_pool_t *pool, void *object_)
{
    mys_pool_object_t *object = (mys_pool_object_t *)object_;
    mys_pool_object_meta_t *meta = _mys_pool_get_object_meta(pool, object);
    meta->next = pool->free_objects;
    meta->block->free += 1;
    pool->free_objects = object;

    if (meta->block->used_count > meta->block->capacity && meta->block->free == meta->block->capacity)
        deallocate_block(pool, meta->block);
}
