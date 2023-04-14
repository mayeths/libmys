#pragma once

#include "config.h"
#include "macro.h"

/**
 * @brief Return balanced partition of i in n
 * 
 * @param gs global start index
 * @param ge global end index
 * @param n number of partition
 * @param i index of local partition
 * @param ls start index of local partition
 * @param le end index of local partition
 */
MYS_API void mys_partition_naive(const int gs, const int ge, const int n, const int i, int *ls, int *le);

#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_PARTITION)
MYS_API int partition1DSimple(
    const int start, const int end,
    const int nworkers, const int workerid,
    int *wstart, int *wend /* return values */
);
#endif
