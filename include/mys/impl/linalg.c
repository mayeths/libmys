#include "../linalg.h"

MYS_API void mys_partition_naive(const int gbegin, const int gend, const int npart, const int ipart, int *lbegin, int *lend) {
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

// MYS_API void mys_partition_weighted(const int gbegin, const int gend, const int npart, const int ipart, const int *weights, int *lbegin, int *lend)
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
