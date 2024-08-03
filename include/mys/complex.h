/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once
// Compatibility for C99 and C++ complex. This header
// can be included by either C99 or ANSI C++ programs to
// allow complex arithmetic to be written in a common subset.

#ifdef __cplusplus
#include <complex>
#include <cmath>
typedef std::complex<double> mys_complex_t;
#define MYS_COMPLEX_MAKE(r, i) mys_complex_t(r, i)
#define MYS_COMPLEX_I(x) mys_complex_t(0.0, 1.0)
#define MYS_COMPLEX_REAL(x) x.real()
#define MYS_COMPLEX_IMAG(x) x.imag()
#define MYS_COMPLEX_ABS(x) std::abs(x)
#define MYS_COMPLEX_ARG(x) std::arg(x)
#else
#include <complex.h>
#include <math.h>
// https://stackoverflow.com/a/56190442
// Q: Is there a way to declare a complex number without including <complex.h>?
// A: Notwithstanding the provisions of 7.1.3,
// a program may undefine and perhaps then
// redefine the macros complex, imaginary, and I.
#ifdef complex
#undef complex
#endif
#ifdef imaginary
#undef imaginary
#endif
#ifdef I
#undef I
#endif
typedef double _Complex mys_complex_t;
#define MYS_COMPLEX_MAKE(r, i) CMPLX(r, i)
#define MYS_COMPLEX_I(x) _Complex_I
#define MYS_COMPLEX_REAL(x) creal(x)
#define MYS_COMPLEX_IMAG(x) cimag(x)
#define MYS_COMPLEX_ABS(x) fabs(x)
#define MYS_COMPLEX_ARG(x) carg(x)
#endif // #ifdef __cplusplus
