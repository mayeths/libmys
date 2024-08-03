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
#include "_private.h"
#include "../linalg.h"

MYS_PUBLIC void mys_partition_naive(const int gbegin, const int gend, const int npart, const int ipart, int *lbegin, int *lend) {
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

// MYS_PUBLIC void mys_partition_weighted(const int gbegin, const int gend, const int npart, const int ipart, const int *weights, int *lbegin, int *lend)
// {
//     const int total = gend - gbegin;
//     int size = total / npart;
//     int rest = total % npart;
//     (*lbegin) = gbegin;
//     if (ipart < rest) {
//         size += 1;
//         (*lbegin) += (ipart * size);
//     } else {
//         (*lbegin) += rest + (ipart * size);
//     }
//     (*lend) = (*lbegin) + size;
// }
