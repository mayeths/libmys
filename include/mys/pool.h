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

#include "_config.h"
#include "macro.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


typedef struct mys_pool_t mys_pool_t;

#define MYS_POOL_DEFAULT 0u
#define MYS_POOL_COMPACT (1u << 0) // Align each object to minimum padding (8 bytes)
#define MYS_POOL_ALIGN_TO_CACHELINE (1u << 1) // Align each object to cache line (at least 128 bytes)
#define MYS_POOL_ALIGN_TO_PAGE (1u << 2) // Align each object to page (at leat 4096 bytes)
#define MYS_POOL_FIXED_SIZE (1u << 3) // Fixed-size pool

/**
 * @brief Creates an object pool with a specified object size.
 * 
 * @param object_size Size of the objects to be stored in the pool. It will be aligned to 8 bytes.
 * @return A pointer to the created object pool, or NULL on failure.
 */
MYS_PUBLIC mys_pool_t *mys_pool_create(size_t object_size);

/**
 * @brief Creates an object pool with specified parameters.
 *
 * This function initializes an object pool for objects of a given size, with an initial capacity and a pool strategy.
 * 
 * @param object_size Size of the objects to be stored in the pool.
 * @param initial_capacity Initial number of objects that the pool can hold.
 * @param pool_strategy Strategy to use for aligning and managing the pool's memory.
 * @return A pointer to the created object pool, or NULL on failure.
 */
MYS_PUBLIC mys_pool_t *mys_pool_create2(size_t object_size, size_t initial_capacity, int pool_strategy);

/**
 * @brief Destroys an object pool.
 *
 * This function releases all memory associated with an object pool and sets the pool pointer to NULL.
 * 
 * @param pool A pointer to the object pool to be destroyed. The pool pointer will be set to NULL after destruction.
 */
MYS_PUBLIC void mys_pool_destroy(mys_pool_t **pool);

/**
 * @brief Acquires an object from the object pool.
 *
 * This function retrieves an object from the pool.
 * If the pool is empty, it allocates memory regions for new objects.
 * The pool size is dynamic by default unless the MYS_POOL_FIXED_SIZE strategy is used.
 * 
 * @param pool A pointer to the object pool.
 * @return A pointer to the acquired object, or NULL if the operation fails.
 */
MYS_PUBLIC void *mys_pool_acquire(mys_pool_t *pool);

/**
 * @brief Releases an object back to the object pool.
 *
 * This function returns an object to the pool, making it available for future acquisitions.
 * It may deallocate memory regions containing unused objects to reduce the internal size of the pool.
 * The pool size is dynamic by default unless the MYS_POOL_FIXED_SIZE strategy is used.
 * 
 * @param pool A pointer to the object pool.
 * @param object A pointer to the object to be released.
 */
MYS_PUBLIC void mys_pool_release(mys_pool_t *pool, void *object);
