// from netlib-math/s_scalbn.c
#pragma once

#include "_fdlibm.h"

//-----------------------------

MYS_STATIC double _mys_math_scalbn (double x, int n)
{
	int  k,hx,lx;
	hx = _FDLIBM_HI(x);
	lx = _FDLIBM_LO(x);
        k = (hx&0x7ff00000)>>20;		/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
	    x *= __fdlibm_two54; 
	    hx = _FDLIBM_HI(x);
	    k = ((hx&0x7ff00000)>>20) - 54; 
            if (n< -50000) return __fdlibm_tiny*x; 	/*underflow*/
	    }
        if (k==0x7ff) return x+x;		/* NaN or Inf */
        k = k+n; 
        if (k >  0x7fe) return __fdlibm_huge*_mys_math_copysign(__fdlibm_huge,x); /* overflow  */
        if (k > 0) 				/* normal result */
	    {x = _FDLIBM_FORM_DOUBLE((hx&0x800fffff)|(k<<20), _FDLIBM_LO(x)); return x;}
        if (k <= -54) {
            if (n > 50000) 	/* in case integer overflow in n+k */
		{return __fdlibm_huge*_mys_math_copysign(__fdlibm_huge,x);	/*overflow*/}
	    else {return __fdlibm_tiny*_mys_math_copysign(__fdlibm_tiny,x); 	/*underflow*/}
        }
        k += 54;				/* subnormal result */
        x = _FDLIBM_FORM_DOUBLE((hx&0x800fffff)|(k<<20), _FDLIBM_LO(x));
        return x*__fdlibm_twom54;
}
