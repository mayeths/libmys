// make test-pool && valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./a.out
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.h"

int main()
{
    mys_debug_init();
    mys_rand_seed_hardware();
    // mys_rand_seed(777);
    mys_pool_t *pool = mys_pool_create2(sizeof(int), 1, 0);
    int acquire_count = 0;
    int release_count = 0;

    int noperation = 100;
    int *operations = (int *)calloc(sizeof(int), noperation);
    mys_rand_i32_array(operations, noperation, -10000, +10000);
    int **allocateds = (int **)calloc(sizeof(int*), 100 * 10000);
    int nallocated = 0;

    for (int iop = 0; iop < noperation; iop++) {
        int op = operations[iop];
        if (op > 0) {
            DLOG_WHEN(iop < 10, "repeat acquire %d times", op);
            for (int i = 0; i < op; i++) {
                int *obj = (int*)mys_pool_acquire(pool);
                AS_NE_PTR(obj, NULL);
                *obj = iop;
                allocateds[nallocated] = obj;
                nallocated += 1;
                acquire_count += 1;
                // printf("obj-%d: %d\n", i, *obj);
            }
        } else if (op < 0) {
            DLOG_WHEN(iop < 10, "repeat release %d times", -op);
            for (int i = 0; i < -op; i++) {
                if (nallocated == 0)
                    break;
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
    free(operations);

    mys_debug_fini();
    return 0;
}
