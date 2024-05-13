#include "../algorithm.h"

MYS_API int mys_sortfn_i32(const void* _a, const void* _b)
{
    int32_t a = *(const int32_t*)_a;
    int32_t b = *(const int32_t*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_API int mys_sortfn_i64(const void* _a, const void* _b)
{
    int64_t a = *(const int64_t*)_a;
    int64_t b = *(const int64_t*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_API int mys_sortfn_f32(const void* _a, const void* _b)
{
    float a = *(const float*)_a;
    float b = *(const float*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_API int mys_sortfn_f64(const void* _a, const void* _b)
{
    double a = *(const double*)_a;
    double b = *(const double*)_b;
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

MYS_API int mys_sortfn_i32_r(const void* _a, const void* _b) { return -mys_sortfn_i32(_a, _b); }
MYS_API int mys_sortfn_i64_r(const void* _a, const void* _b) { return -mys_sortfn_i64(_a, _b); }
MYS_API int mys_sortfn_f32_r(const void* _a, const void* _b) { return -mys_sortfn_f32(_a, _b); }
MYS_API int mys_sortfn_f64_r(const void* _a, const void* _b) { return -mys_sortfn_f64(_a, _b); }
