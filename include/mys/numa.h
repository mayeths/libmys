#pragma once

#include "_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef OS_LINUX
#include <unistd.h>
#include <numa.h>

/**
 * @brief Get the numa id where each page currently resides
 * 
 * @param ptr page pointer that hope to query (not necessary to be page aligned)
 * @return The numa id where the page resides, otherwise -1 on failed.
 */
_MYS_UNUSED static int mys_numa_query(void *ptr)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t mask = ~((size_t)page_size-1);
    void *aligned_ptr = (void *)((size_t)ptr & mask);
    int status[1] = { -1 };
    int ret = numa_move_pages(0, 1, &aligned_ptr, NULL, status, 0);
    if (ret != 0) {
        return -1;
    }
    return status[0];
}

/**
 * @brief Move a page to a specific numa node
 * 
 * @param ptr page pointer that hope to move (not necessary to be page aligned)
 * @param numa_id the numa node id to move to
 * @return 0 on success, otherwise -1. If positive value is returned, it is the number of nonmigrated pages.
 */
_MYS_UNUSED static int mys_numa_move(void *ptr, int numa_id)
{
    long page_size = sysconf(_SC_PAGESIZE);
    size_t mask = ~((size_t)page_size-1);
    void *aligned_ptr = (void *)((size_t)ptr & mask);
    int nodes[1] = { numa_id };
    int status[1] = { -1 };
    return numa_move_pages(0, 1, &aligned_ptr, nodes, status, 0);
}

#endif
