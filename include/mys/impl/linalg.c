#include "../linalg.h"

MYS_API void mys_partition_naive(const int gs, const int ge, const int n, const int i, int *ls, int *le) {
    const int total = ge - gs;
    int size = total / n;
    int rest = total % n;
    (*ls) = gs;
    if (i < rest) {
        size += 1;
        (*ls) += (i * size);
    } else {
        (*ls) += rest + (i * size);
    }
    (*le) = (*ls) + size;
}
