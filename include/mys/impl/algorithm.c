#include "../algorithm.h"

#define _MYS_SORTFN_IMPL(typ, l, r) typ a = *(typ*)_a, b = *(typ*)_b; return (int)((l) > (r)) - (int)((l) < (r));

MYS_STATIC int _mys_sortfn_i32(const void* _a, const void* _b)    { _MYS_SORTFN_IMPL(int32_t, a, b); }
MYS_STATIC int _mys_sortfn_i64(const void* _a, const void* _b)    { _MYS_SORTFN_IMPL(int64_t, a, b); }
MYS_STATIC int _mys_sortfn_f32(const void* _a, const void* _b)    { _MYS_SORTFN_IMPL(float, a, b); }
MYS_STATIC int _mys_sortfn_f64(const void* _a, const void* _b)    { _MYS_SORTFN_IMPL(double, a, b); }
MYS_STATIC int _mys_sortfn_f32i(const void *_a, const void *_b)   { _MYS_SORTFN_IMPL(mys_f32i_t, a.v, b.v); }
MYS_STATIC int _mys_sortfn_f64i(const void *_a, const void *_b)   { _MYS_SORTFN_IMPL(mys_f64i_t, a.v, b.v); }
MYS_STATIC int _mys_sortfn_sizeti(const void *_a, const void *_b) { _MYS_SORTFN_IMPL(mys_sizeti_t, a.v, b.v); }

MYS_STATIC int _mys_sortfn_i32_r(const void* _a, const void* _b)    { return -_mys_sortfn_i32(_a, _b); }
MYS_STATIC int _mys_sortfn_i64_r(const void* _a, const void* _b)    { return -_mys_sortfn_i64(_a, _b); }
MYS_STATIC int _mys_sortfn_f32_r(const void* _a, const void* _b)    { return -_mys_sortfn_f32(_a, _b); }
MYS_STATIC int _mys_sortfn_f64_r(const void* _a, const void* _b)    { return -_mys_sortfn_f64(_a, _b); }
MYS_STATIC int _mys_sortfn_f32i_r(const void *_a, const void *_b)   { return -_mys_sortfn_f32i(_a, _b); }
MYS_STATIC int _mys_sortfn_f64i_r(const void *_a, const void *_b)   { return -_mys_sortfn_f64i(_a, _b); }
MYS_STATIC int _mys_sortfn_sizeti_r(const void *_a, const void *_b) { return -_mys_sortfn_sizeti(_a, _b); }

#undef _MYS_SORTFN_IMPL


MYS_API void mys_sort_i32(int32_t *values, size_t n) { qsort(values, n, sizeof(int32_t), _mys_sortfn_i32); }
MYS_API void mys_sort_i64(int64_t *values, size_t n) { qsort(values, n, sizeof(int64_t), _mys_sortfn_i64); }
MYS_API void mys_sort_f32(float *values, size_t n)   { qsort(values, n, sizeof(float),   _mys_sortfn_f32); }
MYS_API void mys_sort_f64(double *values, size_t n)  { qsort(values, n, sizeof(double),  _mys_sortfn_f64); }

MYS_API void mys_sort_i32_r(int32_t *values, size_t n) { qsort(values, n, sizeof(int32_t), _mys_sortfn_i32_r); }
MYS_API void mys_sort_i64_r(int64_t *values, size_t n) { qsort(values, n, sizeof(int64_t), _mys_sortfn_i64_r); }
MYS_API void mys_sort_f32_r(float *values, size_t n)   { qsort(values, n, sizeof(float),   _mys_sortfn_f32_r); }
MYS_API void mys_sort_f64_r(double *values, size_t n)  { qsort(values, n, sizeof(double),  _mys_sortfn_f64_r); }


MYS_API mys_f64i_t *mys_sort_f64_to_f64i(double *values, size_t n)
{
    mys_f64i_t *pairs = (mys_f64i_t *)malloc(n * sizeof(mys_f64i_t));
    for (size_t i = 0; i < n; i++)
    {
        pairs[i].v = values[i];
        pairs[i].i = (int)i;
    }
    qsort(pairs, n, sizeof(mys_f64i_t), _mys_sortfn_f64i);
    return pairs;
}

MYS_API mys_sizeti_t *mys_sort_sizet_to_sizeti(size_t *values, size_t n, mys_sortctl_t sort_type)
{
    mys_sizeti_t *pairs = (mys_sizeti_t *)malloc(n * sizeof(mys_sizeti_t));
    for (size_t i = 0; i < n; i++)
    {
        pairs[i].v = values[i];
        pairs[i].i = (int)i;
    }
    if (sort_type == MYS_SORT_ASCEND) {
        qsort(pairs, n, sizeof(mys_sizeti_t), _mys_sortfn_sizeti);
    } else if (sort_type == MYS_SORT_DESCEND) {
        qsort(pairs, n, sizeof(mys_sizeti_t), _mys_sortfn_sizeti_r);
    } else {
        assert(0);
    }
    return pairs;
}

// MYS_API void mys_sortidx_f64(double *values, int n, int *sorted_indexes)
// {
//     mys_f64i_t *pairs = (mys_f64i_t *)malloc(n * sizeof(mys_f64i_t));
//     for (int i = 0; i < n; i++) {
//         pairs[i].v = values[i];
//         pairs[i].i = i;
//     }
//     qsort(pairs, n, sizeof(mys_f64i_t), _mys_sortfn_di);
//     for (int i = 0; i < n; i++) {
//         sorted_indexes[i] = pairs[i].i;
//     }
//     free(pairs);
// }

// MYS_API void mys_sortidx_sizet(size_t *values, int n, int *sorted_indexes)
// {
//     mys_sizeti_t *pairs = (mys_sizeti_t *)malloc(n * sizeof(mys_sizeti_t));
//     for (int i = 0; i < n; i++) {
//         pairs[i].v = values[i];
//         pairs[i].i = i;
//     }
//     qsort(pairs, n, sizeof(mys_sizeti_t), _mys_sortfn_di);
//     for (int i = 0; i < n; i++) {
//         sorted_indexes[i] = pairs[i].i;
//     }
//     free(pairs);
// }
