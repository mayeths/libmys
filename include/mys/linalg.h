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
MYS_PUBLIC void mys_partition_naive(const int gbegin, const int gend, const int npart, const int ipart, int *lbegin, int *lend);

/**
 * @brief Recursive Coordinate Bisection (RCB) partitioning with weights
 * 
 * The "imbalance" metric is the ratio of maximum load over average load.
 * 
 * @param gbegin global begin index (inclusive)
 * @param gend global end index (exclusive)
 * @param npart total partition num
 * @param ipart this partition index
 * @param weights array of weights for each element in the range [gbegin, gend)
 * @param tolerance allowable imbalance between partitions (default 1.1)
 * @param num_tests number of test cuts for irregular distributions (default 1)
 * @param lbegin [RETURN] begin index of this partition (inclusive)
 * @param lend [RETURN] end index of this partition (exclusive)
 * @return final imbalance
 */
MYS_PUBLIC double mys_partition_rcb(const int gbegin, const int gend, const int npart, const int ipart, const double *weights, double tolerance, int num_tests, int *lbegin, int *lend);

/* gcc -Wall -Wextra  -I${MYS_DIR}/include test.c && ./a.out
#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.h"

int main() {
    int gbegin = 0, gend = 5;
    int npart = 2, ipart = 0;
    double weights[] = {1, 2, 3, 4, 10}; // Sample weights
    int lbegin, lend;
    double imbalance = mys_partition_rcb(gbegin, gend, npart, ipart, weights, 1.1, 1, &lbegin, &lend);
    printf("Partition [%d, %d) imbalance %f\n", lbegin, lend, imbalance);
    return 0;
}
*/
