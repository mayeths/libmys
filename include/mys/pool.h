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
#define MYS_POOL_COMPACT (1u << 0) // align each element to minimum padding (8 bytes)
#define MYS_POOL_ALIGN_TO_CACHELINE (1u << 1) // align each element to cache line (at least 128 bytes)
#define MYS_POOL_ALIGN_TO_PAGE (1u << 2) // align each element to page (at leat 4096 bytes)

MYS_PUBLIC mys_pool_t *mys_pool_create(size_t object_size);
MYS_PUBLIC mys_pool_t *mys_pool_create2(size_t object_size, size_t initial_capacity, int pool_strategy);
MYS_PUBLIC void mys_pool_destroy(mys_pool_t **pool);
MYS_PUBLIC void *mys_pool_acquire(mys_pool_t *pool);
MYS_PUBLIC void mys_pool_release(mys_pool_t *pool, void *object);
