// musl-1.2.4
#pragma once

#include <stdint.h>
#include "../_config.h"

//-----------------------------

#define __musl_fp_force_evalf(x) do { volatile float y; y = x; (void)y; } while (0)
#define __musl_fp_force_eval(x) do { volatile double y; y = x; (void)y; } while (0)
#define __musl_fp_force_evall(x) do { volatile long double y; y = x; (void)y; } while (0)
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
