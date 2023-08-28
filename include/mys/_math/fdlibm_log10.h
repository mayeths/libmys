// from netlib-math/e_log10.c
#pragma once

#include "../_config.h"
#include "fdlibm.h"

//-----------------------------

MYS_STATIC double _mys_math_log10(double x)
{
	double y,z;
	int i,k,hx;
	unsigned lx;

	hx = _FDLIBM_HI(x);	/* high word of x */
	lx = _FDLIBM_LO(x);	/* low word of x */

        k=0;
        if (hx < 0x00100000) {                  /* x < 2**-1022  */
            if (((hx&0x7fffffff)|lx)==0)
                return -__fdlibm_two54/__fdlibm_zero;             /* log(+-0)=-inf */
            if (hx<0) return (x-x)/__fdlibm_zero;        /* log(-#) = NaN */
            k -= 54; x *= __fdlibm_two54; /* subnormal number, scale up x */
            hx = _FDLIBM_HI(x);                /* high word of x */
        }
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	i  = ((unsigned)k&0x80000000)>>31;
        hx = (hx&0x000fffff)|((0x3ff-i)<<20);
        y  = (double)(k+i);
        x = _FDLIBM_FORM_DOUBLE(hx, lx);
	z  = y*__fdlibm_log10_2lo + __fdlibm_ivln10*_mys_math_log(x);
	return  z+y*__fdlibm_log10_2hi;
}
