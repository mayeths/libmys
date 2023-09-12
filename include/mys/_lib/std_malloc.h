// TODO: Add MYS_MALLOC and MYS_CALLOC macro that enable leak check
// 
// Level 1 (MYS_ALLOC_CHECK_LEVEL 1): only record unfree nbytes
// Level 2 (MYS_ALLOC_CHECK_LEVEL 1): Record FILE and LINE of every invoke. use mys_report_leaked() to print information.

// TODO: Support GUARD
// int shm_fd;
// GUARD_INIT("shmem", shm_fd, -1, shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)); // Do AS_NE_I32(-1, shm_open(...)) in this macro
// ...
// GUARD_FINI("shmem", shm_fd, -1, close(shm_fd)); // Do AS_NE_I32(-1, close(...)) in this macro
// ----- Other convinent macros: -----
// GUARD_FILE_OPEN();
// GUARD_CLOSE_OPEN();
