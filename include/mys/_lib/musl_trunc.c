// from musl-1.2.4/src/math/trunc.c
#pragma once

#include "musl.h"

//-----------------------------

MYS_STATIC double _mys_math_trunc(double x)
{
	union {double f; uint64_t i;} u = {x};
	int e = (int)(u.i >> 52 & 0x7ff) - 0x3ff + 12;
	uint64_t m;

	if (e >= 52 + 12)
		return x;
	if (e < 12)
		e = 1;
	m = -1ULL >> e;
	if ((u.i & m) == 0)
		return x;
	_MUSL_FORCE_EVAL(x + 0x1p120f);
	u.i &= ~m;
	return u.f;
}
