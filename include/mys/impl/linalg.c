/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../linalg.h"
#include "../math.h"

MYS_PUBLIC void mys_partition_naive(const int gbegin, const int gend, const int npart, const int ipart, int *lbegin, int *lend)
{
    const int total = gend - gbegin;
    int size = total / npart;
    int rest = total % npart;
    (*lbegin) = gbegin;
    if (ipart < rest) {
        size += 1;
        (*lbegin) += (ipart * size);
    } else {
        (*lbegin) += rest + (ipart * size);
    }
    (*lend) = (*lbegin) + size;
}

double mys_partition_rcb(const int gbegin, const int gend, const int npart, const int ipart,
    const double *weights, double tolerance, int num_tests, int *lbegin, int *lend)
{
    const int total = gend - gbegin;

    if (npart == 1) {
        *lbegin = gbegin;
        *lend = gend;
        return 1.0;
    }

    double total_weight = 0.0;
    for (int i = gbegin; i < gend; i++) {
        total_weight += weights[i];
    }

    double half_weight = total_weight / 2.0;
    double cumulative = 0.0;
    int midpoint = gbegin;

    // Find midpoint by weight
    for (; midpoint < gend; ++midpoint) {
        cumulative += weights[midpoint];
        if (cumulative >= half_weight * tolerance) {
            break;
        }
    }

    // For test cuts, find the best midpoint that minimizes imbalance
    if (num_tests > 1) {
        int best_midpoint = midpoint;
        double best_balance = mys_math_fabs(total_weight / 2.0 - cumulative);

        for (int test_cut = 1; test_cut < num_tests; ++test_cut) {
            int test_midpoint = gbegin + (total / (test_cut + 1));
            double test_cumulative = 0.0;
            for (int i = gbegin; i < test_midpoint; i++) {
                test_cumulative += weights[i];
            }

            double current_balance = mys_math_fabs(total_weight / 2.0 - test_cumulative);
            if (current_balance < best_balance) {
                best_balance = current_balance;
                best_midpoint = test_midpoint;
            }
        }

        midpoint = best_midpoint;
    }

    int half_npart = npart / 2;

    if (ipart < half_npart) {
        mys_partition_rcb(gbegin, midpoint, half_npart, ipart, weights, tolerance, num_tests, lbegin, lend);
    } else {
        mys_partition_rcb(midpoint, gend, npart - half_npart, ipart - half_npart, weights, tolerance, num_tests, lbegin, lend);
    }

    // Calculate the imbalance as the ratio of the maximum load to the average load
    double weight_lower = 0.0;
    double weight_upper = 0.0;
    for (int i = gbegin; i < midpoint; i++) {
        weight_lower += weights[i];
    }
    for (int i = midpoint; i < gend; i++) {
        weight_upper += weights[i];
    }

    double avg_load = total_weight / npart;
    double max_load = (weight_lower > weight_upper) ? weight_lower : weight_upper;
    double imbalance = max_load / avg_load;
    return imbalance;
}
