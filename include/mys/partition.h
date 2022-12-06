#pragma once

static inline int partition1DSimple(
    const int start, const int end,
    const int nworkers, const int workerid,
    int *wstart, int *wend /* return values */
) {
    const int total = end - start;
    int size = total / nworkers;
    int rest = total % nworkers;
    (*wstart) = start;
    if (workerid < rest) {
        size += 1;
        (*wstart) += (workerid * size);
    } else {
        (*wstart) += rest + (workerid * size);
    }
    (*wend) = (*wstart) + size;
    return 0;
}

static inline int partition1DLoadBalanced(
    const int *p, const int *j,
    const int start, const int end,
    const int nworkers, const int workerid,
    int *wstart, int *wend
) {
    return 1;
}
