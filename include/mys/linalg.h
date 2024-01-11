#pragma once

#include "_config.h"
#include "macro.h"

/**
 * @brief Naive partition of [gs, ge) to <npart> partition
 * 
 * @param gbegin global begin index (inclusive)
 * @param gend global end index (exclusive)
 * @param npart total partition num
 * @param ipart this partition index
 * @param lbegin [RETURN] begin index of this partition (inclusive)
 * @param lend [RETURN] end index of this partition (exclusive)
 */
MYS_API void mys_partition_naive(const int gbegin, const int gend, const int npart, const int ipart, int *lbegin, int *lend);

/**
 * @brief Naive partition of [gs, ge) to <npart> partition
 * 
 * @param gbegin global begin index (inclusive)
 * @param gend global end index (exclusive)
 * @param npart total partition num
 * @param ipart this partition index
 * @param weights weights of each partition
 * @param lbegin [RETURN] begin index of this partition (inclusive)
 * @param lend [RETURN] end index of this partition (exclusive)
 */
// MYS_API void mys_partition_weighted(const int gbegin, const int gend, const int npart, const int ipart, const int *weights, int *lbegin, int *lend);
