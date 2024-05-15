#include "../algorithm.h"

MYS_API void mys_sort_i32(int32_t *values, size_t n) { qsort(values, n, sizeof(int32_t), mys_sortfn_i32); }
MYS_API void mys_sort_i64(int64_t *values, size_t n) { qsort(values, n, sizeof(int64_t), mys_sortfn_i64); }
MYS_API void mys_sort_f32(float *values, size_t n)   { qsort(values, n, sizeof(float), mys_sortfn_f32);   }
MYS_API void mys_sort_f64(double *values, size_t n)  { qsort(values, n, sizeof(double), mys_sortfn_f64);  }

MYS_API void mys_sort_i32_r(int32_t *values, size_t n) { qsort(values, n, sizeof(int32_t), mys_sortfn_i32_r); }
MYS_API void mys_sort_i64_r(int64_t *values, size_t n) { qsort(values, n, sizeof(int64_t), mys_sortfn_i64_r); }
MYS_API void mys_sort_f32_r(float *values, size_t n)   { qsort(values, n, sizeof(float), mys_sortfn_f32_r);   }
MYS_API void mys_sort_f64_r(double *values, size_t n)  { qsort(values, n, sizeof(double), mys_sortfn_f64_r);  }

MYS_STATIC int mys_sortfn_i32(const void* _a, const void* _b)
{
    int32_t a = *(const int32_t*)_a;
    int32_t b = *(const int32_t*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_STATIC int mys_sortfn_i64(const void* _a, const void* _b)
{
    int64_t a = *(const int64_t*)_a;
    int64_t b = *(const int64_t*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_STATIC int mys_sortfn_f32(const void* _a, const void* _b)
{
    float a = *(const float*)_a;
    float b = *(const float*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_STATIC int mys_sortfn_f64(const void* _a, const void* _b)
{
    double a = *(const double*)_a;
    double b = *(const double*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_STATIC int mys_sortfn_i32_r(const void* _a, const void* _b) { return -mys_sortfn_i32(_a, _b); }
MYS_STATIC int mys_sortfn_i64_r(const void* _a, const void* _b) { return -mys_sortfn_i64(_a, _b); }
MYS_STATIC int mys_sortfn_f32_r(const void* _a, const void* _b) { return -mys_sortfn_f32(_a, _b); }
MYS_STATIC int mys_sortfn_f64_r(const void* _a, const void* _b) { return -mys_sortfn_f64(_a, _b); }
