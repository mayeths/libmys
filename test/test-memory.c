// make test-memory && valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./a.out
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.h"

int main()
{
    // mys_debug_init();
    mys_rand_seed_hardware();
    // int repeat = 10;
    int repeat = 10000;
    int nallocated = 0;
    uint8_t **allocateds = (uint8_t **)calloc(sizeof(uint8_t*), repeat);
    size_t *sizes = (size_t *)calloc(sizeof(size_t), repeat);
    mys_arena_t *arena = mys_arena_create("test");
    size_t alloc_count = 0;
    size_t free_count = 0;

    for (int i = 0; i < repeat; i++) {
        if (mys_rand_i32(0, INT_MAX) % 2 == 0) {
            size_t size = mys_rand_i32(0, 1 * 1024);
            uint8_t *obj = (uint8_t *)mys_malloc2(arena, size);
            AS_NE_PTR(obj, NULL);
            memset(obj, 1, size);
            allocateds[nallocated] = obj;
            sizes[nallocated] = size;
            nallocated += 1;
            alloc_count += 1;
            // printf("alloc obj-%d: %p  size=%zu alive=%zu freed=%zu total=%zu\n", i, obj, size, arena->alive, arena->freed, arena->total);
        } else {
            if (nallocated != 0) {
                uint8_t *obj = allocateds[nallocated - 1];
                size_t size = sizes[nallocated - 1];
                AS_NE_PTR(obj, NULL);
                mys_free2(arena, obj, size);
                // printf("free obj-%d: %p  size=%zu alive=%zu freed=%zu total=%zu\n", i, obj, size, arena->alive, arena->freed, arena->total);
                nallocated -= 1;
                free_count += 1;
            }
        }
    }

    size_t cleanup_count = 0;
    while (nallocated != 0) {
        uint8_t *obj = allocateds[nallocated - 1];
        size_t size = sizes[nallocated - 1];
        AS_NE_PTR(obj, NULL);
        mys_free2(arena, obj, size);
        nallocated -= 1;
        cleanup_count += 1;
    }

    printf("alloc_count=%zu free_count=%zu cleanup_count=%zu\n", alloc_count, free_count, cleanup_count);
    AS_EQ_SIZET(arena->alive, 0);
    AS_EQ_SIZET(arena->freed, arena->total);
    mys_arena_destroy(&arena);
    free(allocateds);
    free(sizes);

    return 0;
}
