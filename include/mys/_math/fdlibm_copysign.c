// from netlib-math/s_copysign.c
#pragma once

#include "_fdlibm.h"

//-----------------------------

MYS_STATIC double _mys_math_copysign(double x, double y)
{
    x = _FDLIBM_FORM_DOUBLE((_FDLIBM_HI(x)&0x7fffffff)|(_FDLIBM_HI(y)&0x80000000), _FDLIBM_LO(x));
        return x;
}
