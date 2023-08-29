// from netlib-math/s_fabs.c
#pragma once

#include "_fdlibm.h"

//-----------------------------

MYS_STATIC double _mys_math_fabs(double x)
{
    x = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(x)&0x7fffffff, _FDLIBM_LO(x));
        return x;
}
