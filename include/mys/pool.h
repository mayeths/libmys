#pragma once

#include "_config.h"
#include "macro.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


typedef struct mys_pool_t mys_pool_t;

#define MYS_POOL_DEFAULT 0

MYS_API mys_pool_t *mys_pool_create(size_t object_size);
MYS_API mys_pool_t *mys_pool_create2(size_t object_size, size_t initial_size, int pool_straregy);
MYS_API void mys_pool_destroy(mys_pool_t **pool);
MYS_API void *mys_pool_acquire(mys_pool_t *pool);
MYS_API void mys_pool_release(mys_pool_t *pool, void *object);