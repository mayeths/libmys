// make test-pool && valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./a.out
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
    int repeat = 100000;
    int nallocated = 0;
    int **allocateds = (int **)calloc(sizeof(int*), repeat);
    mys_pool_t *pool = mys_pool_create2(sizeof(int), 1, 0);
    int acquire_count = 0;
    int release_count = 0;

    for (int i = 0; i < repeat; i++) {
        if (mys_rand_i32(0, INT_MAX) % 2 == 0) {
            int *obj = (int*)mys_pool_acquire(pool);
            AS_NE_PTR(obj, NULL);
            *obj = i;
            allocateds[nallocated] = obj;
            nallocated += 1;
            acquire_count += 1;
            // printf("obj-%d: %d\n", i, *obj);
        } else {
            if (nallocated != 0) {
                int *obj = allocateds[nallocated - 1];
                AS_NE_PTR(obj, NULL);
                mys_pool_release(pool, obj);
                nallocated -= 1;
                release_count += 1;
            }
        }
    }

    printf("acquire_count=%d release_count=%d\n", acquire_count, release_count);
    mys_pool_destroy(&pool);
    AS_EQ_SIZET(mys_arena_pool->alive, 0);
    AS_EQ_SIZET(mys_arena_pool->freed, mys_arena_pool->total);
    free(allocateds);

    return 0;
}
