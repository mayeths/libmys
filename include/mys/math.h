/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
// (fdlibm 5.3) netlib-math
#pragma once

#include <stdint.h>
#include "_config.h"

MYS_PUBLIC double mys_math_copysign(double x, double y);
MYS_PUBLIC double mys_math_fabs(double x);
MYS_PUBLIC double mys_math_log(double x);
MYS_PUBLIC double mys_math_log10(double x);
MYS_PUBLIC double mys_math_sqrt(double x);
MYS_PUBLIC double mys_math_scalbn(double x, int n);
MYS_PUBLIC double mys_math_pow(double x, double y);
MYS_PUBLIC double mys_math_trunc(double x);


/*
int main()
{
    double lo = 1e-6;
    double hi = 100;
    double step = 0.1;
    for (double i = lo; i < hi; i += step) {
        double my = mys_math_pow(i, i);
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
