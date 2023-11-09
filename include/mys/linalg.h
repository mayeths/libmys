#pragma once

#include "_config.h"
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
