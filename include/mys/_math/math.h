// We mainly derive from fdlibm, but also use some snippets from musl's libm.
#pragma once

#include "fdlibm_copysign.h"
#include "fdlibm_fabs.h"
#include "fdlibm_log.h"
#include "fdlibm_log10.h"
#include "fdlibm_sqrt.h"
#include "fdlibm_scalbn.h"
#include "fdlibm_pow.h"

#include "musl_trunc.h"

/*
int main()
{
    double lo = 1e-6;
    double hi = 100;
    double step = 0.1;
    for (double i = lo; i < hi; i += step) {
        double my = _mys_math_pow(i, i);
        double his = pow(i, i);
        double diff = my - his;
        double tolerance = 2.3e-16;
        printf("%.17f %.17f %.17f %.17f\n", i, my, his, diff);
        if (diff / his > tolerance) {
            printf("diff / his %.2e > %.2e\n", diff / his, tolerance);
            return 1;
        }
    }
}
*/
