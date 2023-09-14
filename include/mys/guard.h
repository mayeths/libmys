// TODO: Add MYS_MALLOC and MYS_CALLOC macro that enable leak check
// 
// Level 1 (MYS_ALLOC_CHECK_LEVEL 1): only record unfree nbytes
// Level 2 (MYS_ALLOC_CHECK_LEVEL 1): Record FILE and LINE of every invoke. use mys_report_leaked() to print information.

// TODO: Support GUARD
// int shm_fd;
// GUARD_REGION_BEGIN();
// GUARD_ACQUIRE(shm_fd, -1, shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)); // Do AS_NE_I32(-1, shm_open(...)) in this macro
// ...
// GUARD_RELEASE(shm_fd, -1, close(shm_fd)); // Do AS_NE_I32(-1, close(...)) in this macro
// GUARD_REGION_END();
// ------ Output:
// ------ Unclosed resource:
//     src/main.c:68

#include <stdint.h>
#include <stdlib.h>
#include "_config.h"
#include "macro.h"

MYS_API void mys_guard_begin(const char *type_name, size_t type_size, void *variable_ptr, const char *file, int line);
MYS_API void mys_guard_end(const char *type_name, size_t type_size, void *variable_ptr, const char *file, int line);

#define G_BEGIN(typ, variable) mys_guard_begin(#typ, sizeof(typ), &variable, MYS_FNAME, __LINE__)
#define G_END(typ, variable) mys_guard_end(#typ, sizeof(typ), &variable, MYS_FNAME, __LINE__)

/////// General handle
#define G_BEGIN_INT(variable) G_BEGIN(int, variable)    // Begin a life-cycle of [int] handle
#define G_BEGIN_PTR(variable) G_BEGIN(void *, variable) // Begin a life-cycle of [void *] handle
#define G_END_INT(variable)   G_END(int, variable)    // End a life-cycle of [int] handle
#define G_END_PTR(variable)   G_END(void *, variable) // End a life-cycle of [void *] handle
/////// MPI handle
#define G_BEGIN_MPI_COMM(variable)     G_BEGIN(MPI_Comm, variable)     // Begin a life-cycle of [MPI_Comm] handle
#define G_BEGIN_MPI_DATATYPE(variable) G_BEGIN(MPI_Datatype, variable) // Begin a life-cycle of [MPI_Datatype] handle
#define G_BEGIN_MPI_OP(variable)       G_BEGIN(MPI_Op, variable)       // Begin a life-cycle of [MPI_Op] handle
#define G_END_MPI_COMM(variable)       G_END(MPI_Comm, variable)       // End a life-cycle of [MPI_Comm] handle
#define G_END_MPI_DATATYPE(variable)   G_END(MPI_Datatype, variable)   // End a life-cycle of [MPI_Datatype] handle
#define G_END_MPI_OP(variable)         G_END(MPI_Op, variable)         // End a life-cycle of [MPI_Op] handle

/* gcc -I${MYS_DIR}/include a.c -lrt && valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes ./a.out
int main()
{
    void *ptr1 = malloc(65536);
    G_BEGIN_PTR(ptr1);

    FILE *file1 = fopen("./a.c", "r");
    G_BEGIN_PTR(file1);

    void *ptr2 = malloc(65536);
    G_BEGIN_PTR(ptr2);

    FILE *file2 = fopen("./a.c", "r");
    G_BEGIN_PTR(file2);

    FILE *file3 = fopen("./a.c", "r");
    G_BEGIN_PTR(file3);

    int fd = shm_open("/jasklfdjlds_shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    G_BEGIN_INT(fd);

    G_END_PTR(ptr1);
    free(ptr1);

    G_END_INT(fd);
    close(fd);
    shm_unlink("/jasklfdjlds_shm");

    G_END_PTR(ptr2);
    free(ptr2);

    void *ptr3 = malloc(65536);
    G_BEGIN_PTR(ptr3);

    void *ptr4 = malloc(65536);
    G_BEGIN_PTR(ptr4);

    G_END_PTR(ptr3);
    free(ptr3);

    G_END_PTR(file3);
    fclose(file3);

    G_END_PTR(file1);
    fclose(file1);

    G_END_PTR(file2);
    fclose(file2);

    G_END_PTR(ptr4);
    free(ptr4);

    void *ptr5 = malloc(65536);
    G_BEGIN_PTR(ptr5);

    G_END_PTR(ptr5);
    free(ptr5);
}
*/
