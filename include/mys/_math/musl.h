// musl-1.2.4
#pragma once

#include <stdint.h>
#include "../_config.h"

//-----------------------------

MYS_STATIC inline void __musl_fp_force_evalf(float x)
{
	volatile float y;
	y = x;
	(void)y;
}

MYS_STATIC inline void __musl_fp_force_eval(double x)
{
	volatile double y;
	y = x;
	(void)y;
}

MYS_STATIC inline void __musl_fp_force_evall(long double x)
{
	volatile long double y;
	y = x;
	(void)y;
}

#define _MUSL_FORCE_EVAL(x) do {              \
	if (sizeof(x) == sizeof(float)) {         \
		__musl_fp_force_evalf(x);             \
	} else if (sizeof(x) == sizeof(double)) { \
		__musl_fp_force_eval(x);              \
	} else {                                  \
		__musl_fp_force_evall(x);             \
	}                                         \
} while(0)

MYS_STATIC double _mys_math_trunc(double x);
