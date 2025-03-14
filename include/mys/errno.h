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
#pragma once

#include "thread.h"

#include <errno.h>

/**
 * @brief The number of last libmys error
 * @note 1. The value in `mys_errno` is significant only when the return value of the call indicated an error (i.e., `-1` or `NULL` from most libmys functions).
 * @note 2. The value of errno is never set to `0` by any libmys function.
 * @note 3. At program startup, the value stored is `0`.
 * @note 4. Any libmys function can alter the value in `mys_errno` before return, whether or not they detect errors.
 * @note 5. A few functions require the caller to preset errno to `0` and test it afterwards to see if an error was detected.
 */
MYS_PUBVAR mys_thread_local int mys_errno;

// `MYS_GOTOIF(arg1 == NULL, MYS_EINVAL, failed);`
#define MYS_GOTOIF(errcond, errcode, label) do { \
    if (errcond) {                               \
        mys_errno = (errcode);                   \
        goto label;                              \
    }                                            \
} while (0)

// `MYS_RETIF(arg == 0, MYS_EINVAL);`
// `MYS_RETIF(newptr == NULL, MYS_ENOMEM, NULL);`
#define MYS_RETIF(errcond, errcode, ...) do { \
    if (errcond) {                            \
        mys_errno = (errcode);                \
        return  __VA_ARGS__;                  \
    }                                         \
} while (0)

// Standard error [1:256]
#define	MYS_EPERM       EPERM	/* Operation not permitted */
#define	MYS_ENOENT       ENOENT	/* No such file or directory */
#define	MYS_ESRCH       ESRCH	/* No such process */
#define	MYS_EINTR       EINTR	/* Interrupted system call */
#define	MYS_EIO       EIO	/* I/O error */
#define	MYS_ENXIO       ENXIO	/* No such device or address */
#define	MYS_E2BIG       E2BIG	/* Argument list too long */
#define	MYS_ENOEXEC       ENOEXEC	/* Exec format error */
#define	MYS_EBADF       EBADF	/* Bad file number */
#define	MYS_ECHILD       ECHILD	/* No child processes */
#define	MYS_EAGAIN       EAGAIN	/* Try again */
#define	MYS_ENOMEM       ENOMEM	/* Out of memory */
#define	MYS_EACCES       EACCES	/* Permission denied */
#define	MYS_EFAULT       EFAULT	/* Bad address */
#define	MYS_ENOTBLK       ENOTBLK	/* Block device required */
#define	MYS_EBUSY       EBUSY	/* Device or resource busy */
#define	MYS_EEXIST       EEXIST	/* File exists */
#define	MYS_EXDEV       EXDEV	/* Cross-device link */
#define	MYS_ENODEV       ENODEV	/* No such device */
#define	MYS_ENOTDIR       ENOTDIR	/* Not a directory */
#define	MYS_EISDIR       EISDIR	/* Is a directory */
#define	MYS_EINVAL       EINVAL	/* Invalid argument */
#define	MYS_ENFILE       ENFILE	/* File table overflow */
#define	MYS_EMFILE       EMFILE	/* Too many open files */
#define	MYS_ENOTTY       ENOTTY	/* Not a typewriter */
#define	MYS_ETXTBSY       ETXTBSY	/* Text file busy */
#define	MYS_EFBIG       EFBIG	/* File too large */
#define	MYS_ENOSPC       ENOSPC	/* No space left on device */
#define	MYS_ESPIPE       ESPIPE	/* Illegal seek */
#define	MYS_EROFS       EROFS	/* Read-only file system */
#define	MYS_EMLINK       EMLINK	/* Too many links */
#define	MYS_EPIPE       EPIPE	/* Broken pipe */
#define	MYS_EDOM       EDOM	/* Math argument out of domain of func */
#define	MYS_ERANGE       ERANGE	/* Math result not representable */
// Custom error [256:)

/*
mys_pool_block_t *allocate_block(size_t capacity)
{
    mys_pool_block_t* block = (mys_pool_block_t*)mys_malloc2(MYS_ARENA_POOL, sizeof(mys_pool_block_t));
    MYS_RETIF(block == NULL, MYS_ENOMEM, NULL);

    block->objects = (mys_pool_olist_t*)mys_malloc2(MYS_ARENA_POOL, capacity * sizeof(mys_pool_olist_t));
    MYS_GOTOIF(block->objects == NULL, MYS_ENOMEM, failed);

    {
        mys_pool_block_t *next_block = NULL;
        // ...
        block->prev = NULL;
        block->next = next_block;
        if (next_block != NULL) next_block->prev = block;
    }

    return block;
failed:
    if (block) {
        if (block->objects) mys_free2(MYS_ARENA_POOL, block->objects, capacity * sizeof(mys_pool_olist_t));
        mys_free2(MYS_ARENA_POOL, block, sizeof(mys_pool_block_t));
    }
}

void deallocate_block(mys_pool_block_t* block)
{
    MYS_RETIF(block == NULL, MYS_EINVAL);

    if (block->prev != NULL) block->prev->next = block->next;
    if (block->next != NULL) block->next->prev = block->prev;

    mys_free2(MYS_ARENA_POOL, block->objects, capacity * sizeof(mys_pool_olist_t));
    mys_free2(MYS_ARENA_POOL, block, sizeof(mys_pool_block_t));
}
*/