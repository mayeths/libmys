#pragma once
#ifdef __cplusplus
extern "C" {
#endif

static inline int partition1DSimple(
    const int start, const int end,
    const int nworkers, const int widx,
    int *wstart, int *wend
) {
    const int total = end - start;
    int size = total / nworkers;
    int rest = total % nworkers;
    (*wstart) = start;
    if (widx < rest) {
        size += 1;
        (*wstart) += (widx * size);
    } else {
        (*wstart) += rest + (widx * size);
    }
    (*wend) = (*wstart) + size;
    return 0;
}

static inline int partition1DLoadBalanced(
    const int *p, const int *j,
    const int start, const int end,
    const int nworkers, const int widx,
    int *wstart, int *wend
) {
    return 1;
}


#ifdef __cplusplus
} /*extern "C"*/
#endif
