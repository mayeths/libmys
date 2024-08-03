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
#include "../pool.h"
#include "../memory.h"

struct mys_pool_object_meta_t;
struct mys_pool_object_t;
struct mys_pool_olist_t; // object list
struct mys_pool_block_t;
struct mys_pool_t;

typedef struct mys_pool_object_meta_t {
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
    Meta data comes with object is because in mys_pool_release, it returns object address.
    If you separate meta data and object, then you have to search for meta data
*/
typedef struct mys_pool_object_t {
    // uint8_t object[];
    // uint8_t padding[];
    // mys_pool_object_meta_t meta;
} mys_pool_object_t;

typedef struct mys_pool_olist_t {
    void* self;
    struct mys_pool_olist_t* next;
} mys_pool_olist_t;

typedef struct mys_pool_block_t {
    size_t capacity;
    size_t free;
    size_t acquired_count;
    uint8_t* memory;
    struct mys_pool_olist_t* objects;
    struct mys_pool_olist_t* free_object_head;
    struct mys_pool_block_t* prev;
    struct mys_pool_block_t* next;
} mys_pool_block_t;

typedef struct mys_pool_t {
    size_t robj_size;
    size_t pobj_size;
    size_t mobj_size;
    size_t block_capacity;
    int strategy;
    struct mys_pool_block_t* free_block_head;
    struct mys_pool_block_t* free_block_tail;
    struct mys_pool_block_t* full_block_head;
    struct mys_pool_block_t* full_block_tail;
} mys_pool_t;

static mys_pool_object_t* _mys_pool_get_object(mys_pool_t* pool, mys_pool_block_t* block, size_t i)
{
    if (pool == NULL || block == NULL)
        return NULL;
    return (mys_pool_object_t*)(block->memory + pool->mobj_size * i);
}

static size_t _mys_pool_index_object(mys_pool_t* pool, mys_pool_block_t* block, mys_pool_object_t* object)
{
    if (pool == NULL || block == NULL || object == NULL)
        return 0;
    return ((uint8_t*)object - block->memory) / pool->mobj_size;
}

static mys_pool_object_meta_t* _mys_pool_get_object_meta(mys_pool_t* pool, mys_pool_object_t* object)
{
    if (pool == NULL || object == NULL)
        return NULL;
    uint8_t* memory = (uint8_t*)object;
    mys_pool_object_meta_t* meta = (mys_pool_object_meta_t*)(memory + pool->pobj_size);
    return meta;
}

MYS_PUBLIC mys_pool_t* mys_pool_create(size_t object_size)
{
    return mys_pool_create2(object_size, 64, MYS_POOL_DEFAULT);
}

MYS_PUBLIC mys_pool_t* mys_pool_create2(size_t object_size, size_t initial_capacity, int pool_strategy)
{
    if (initial_capacity == 0)
        return NULL;

    mys_pool_t* pool = (mys_pool_t*)mys_malloc2(mys_arena_pool, sizeof(mys_pool_t));
    if (!pool)
        return NULL;

    pool->robj_size = object_size;
    pool->pobj_size = MYS_ALIGN_UP(pool->robj_size, 8);
    pool->mobj_size = pool->pobj_size + MYS_ALIGN_UP(sizeof(mys_pool_object_meta_t), 8);
    pool->block_capacity = initial_capacity;
    pool->strategy = pool_strategy;
    pool->free_block_head = NULL;
    pool->free_block_tail = NULL;
    pool->full_block_head = NULL;
    pool->full_block_tail = NULL;

    return pool;
}

static void allocate_block(mys_pool_t* pool)
{
    mys_pool_block_t* block = (mys_pool_block_t*)mys_malloc2(mys_arena_pool, sizeof(mys_pool_block_t));
    if (block == NULL)
        return;
    // DLOG(0, "    Allocate block %p", block);

    block->capacity = pool->block_capacity;
    block->free = pool->block_capacity;
    block->acquired_count = 0;

    block->memory = (uint8_t*)mys_aligned_alloc2(mys_arena_pool, sysconf(_SC_PAGE_SIZE), block->capacity * pool->mobj_size);
    if (block->memory == NULL) {
        mys_free2(mys_arena_pool, block, sizeof(mys_pool_block_t));
        return;
    }

    block->objects = (mys_pool_olist_t*)mys_malloc2(mys_arena_pool, block->capacity * sizeof(mys_pool_olist_t));
    if (block->objects == NULL) {
        mys_free2(mys_arena_pool, block->memory, block->capacity * pool->mobj_size);
        mys_free2(mys_arena_pool, block, sizeof(mys_pool_block_t));
        return;
    }

    block->free_object_head = NULL;
    for (size_t i = block->capacity; i > 0; i--) { // i=block->capacity-1 will crash if cap=0
        mys_pool_object_t* object = _mys_pool_get_object(pool, block, i - 1);
        mys_pool_object_meta_t* meta = _mys_pool_get_object_meta(pool, object);
        meta->block = block;
        block->objects[i - 1].self = (void* )object;
        block->objects[i - 1].next = block->free_object_head;
        block->free_object_head = &block->objects[i - 1];
        // DLOG(0, "    Prepare object %zu %p", i - 1, object);
    }

    mys_pool_block_t *next_block = pool->free_block_head;
    pool->free_block_head = block;
    block->prev = NULL;
    block->next = next_block;
    // DLOG(0, "    Block %p->next is set to %p", block, next_block);
    if (next_block != NULL) next_block->prev = block;
    if (pool->free_block_tail == NULL) pool->free_block_tail = block;

    pool->block_capacity *= 2;
}

MYS_STATIC void deallocate_block(mys_pool_t* pool, mys_pool_block_t* block)
{
    AS_NE_PTR(pool, NULL);
    AS_NE_PTR(block, NULL);

    if (block == pool->free_block_head) pool->free_block_head = block->next;
    if (block == pool->free_block_tail) pool->free_block_tail = block->prev;
    if (block == pool->full_block_head) pool->full_block_head = block->next;
    if (block == pool->full_block_tail) pool->full_block_tail = block->prev;
    if (block->prev != NULL) block->prev->next = block->next;
    if (block->next != NULL) block->next->prev = block->prev;

    // DLOG(0, "    Deallocate block %p", block);
    mys_free2(mys_arena_pool, block->objects, block->capacity * sizeof(mys_pool_olist_t));
    mys_free2(mys_arena_pool, block->memory, block->capacity * pool->mobj_size);
    mys_free2(mys_arena_pool, block, sizeof(mys_pool_block_t));
}

MYS_PUBLIC void mys_pool_destroy(mys_pool_t* *pool)
{
    AS_NE_PTR(pool, NULL);

    if (*pool == NULL)
        return;
    mys_pool_block_t* block = NULL;
    block = (*pool)->free_block_head;
    while (block != NULL) {
        mys_pool_block_t* next_block = block->next;
        deallocate_block((*pool), block);
        block = next_block;
    }
    block = (*pool)->full_block_head;
    while (block != NULL) {
        mys_pool_block_t* next_block = block->next;
        deallocate_block((*pool), block);
        block = next_block;
    }
    mys_free2(mys_arena_pool, *pool, sizeof(mys_pool_t));
    *pool = NULL;
}

MYS_PUBLIC void* mys_pool_acquire(mys_pool_t* pool)
{
    // DLOG(0, "Acquiring");
    AS_NE_PTR(pool, NULL);

    if (pool->free_block_head == NULL) {
        allocate_block(pool);
        if (pool->free_block_head == NULL) {
            // DLOG(0, "    Failed to allocate block for pool %p", pool);
            return NULL;
        }
    }

    mys_pool_block_t* block = pool->free_block_head;
    if (block->free_object_head == NULL) {
        // DLOG(0, "    What? %p block=%p", pool, block);
        return NULL;
    }

    mys_pool_olist_t* llist_node = block->free_object_head;
    block->free_object_head = llist_node->next;
    llist_node->next = NULL;

    block->free--;
    block->acquired_count++;

    if (block->free == 0) {
        // DLOG(0, "    Block %p is full, move to full list", block);
        // Remove block from free list
        mys_pool_block_t *next_block = block->next;
        if (next_block != NULL) next_block->prev = NULL;

        pool->free_block_head = next_block;
        // DLOG(0, "    free_block_head is set to %p", next_block);
        if (pool->free_block_head == NULL) {
            pool->free_block_tail = NULL;
        }

        // Move block to full list
        block->next = NULL;
        if (pool->full_block_tail == NULL) {
            pool->full_block_head = block;
            block->prev = NULL;
        } else {
            pool->full_block_tail->next = block;
            block->prev = pool->full_block_tail;
        }
        pool->full_block_tail = block;
        // DLOG(0, "    full_block_tail is set to %p", block);
    }

    // DLOG(0, "    Acquired %p", llist_node->self);
    return (void* )llist_node->self;
}

MYS_PUBLIC void mys_pool_release(mys_pool_t* pool, void* object_)
{
    // DLOG(0, "Releasing %p", object_);
    AS_NE_PTR(pool, NULL);

    mys_pool_object_t* object = (mys_pool_object_t*)object_;
    mys_pool_object_meta_t* meta = _mys_pool_get_object_meta(pool, object);
    mys_pool_block_t* block = meta->block;
    size_t index = _mys_pool_index_object(pool, block, object);
    mys_pool_olist_t* llist_node = &block->objects[index];

    // Add to free list
    llist_node->next = block->free_object_head;
    block->free_object_head = llist_node;

    block->free++;

    if (block->acquired_count > block->capacity && block->free == block->capacity) {
        deallocate_block(pool, block);
    } else if (block->free == 1) { // Block is free to use
        // DLOG(0, "    Block %p is free, move to free list", block);
        // Remove block from full list
        // Handle left to right relationship
        if (block->prev == NULL) {
            pool->full_block_head = block->next;
        } else {
            block->prev->next = block->next;
        }
        // Handle right to left relationship
        if (block->next == NULL) {
            pool->full_block_tail = block->prev;
        } else {
            block->next->prev = block->prev;
        }

        // Move block to free list
        // Handle left to right relationship
        block->next = pool->free_block_head;
        // DLOG(0, "    Block %p->next is set to %p", block, pool->free_block_head);
        // Handle right to left relationship
        block->prev = NULL;
        if (pool->free_block_head == NULL) {
            pool->free_block_tail = block;
        } else {
            pool->free_block_head->prev = block;
        }
        pool->free_block_head = block;
        // DLOG(0, "    free_block_head is set to %p", block);
    }
    // DLOG(0, "    Released %p", llist_node->self);
}
