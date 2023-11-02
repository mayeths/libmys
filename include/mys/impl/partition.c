#include "../partition.h"

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

#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_PARTITION)
MYS_API int partition1DSimple(
    const int start, const int end,
    const int nworkers, const int workerid,
    int *wstart, int *wend /* return values */
) {
    mys_partition_naive(start, end, nworkers, workerid, wstart, wend);
    return 0;
}
#endif
