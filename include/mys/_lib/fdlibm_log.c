// from netlib-math/e_log.c
#pragma once

#include "fdlibm.h"

//-----------------------------

MYS_STATIC double _mys_math_log(double x)
{
	double hfsq,f,s,z,R,w,t1,t2,dk;
	int k,hx,i,j;
	unsigned lx;
    static double zero = 0.0;

	hx = _FDLIBM_HI(x);		/* high word of x */
	lx = _FDLIBM_LO(x);		/* low  word of x */

	k=0;
	if (hx < 0x00100000) {			/* x < 2**-1022  */
	    if (((hx&0x7fffffff)|lx)==0) 
		return -__fdlibm_two54/zero;		/* log(+-0)=-inf */
	    if (hx<0) return (x-x)/zero;	/* log(-#) = NaN */
	    k -= 54; x *= __fdlibm_two54; /* subnormal number, scale up x */
	    hx = _FDLIBM_HI(x);		/* high word of x */
	} 
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	hx &= 0x000fffff;
	i = (hx+0x95f64)&0x100000;
	x = _FDLIBM_FORM_DOUBLE(hx|(i^0x3ff00000), _FDLIBM_LO(x)); /* normalize x or x/2 */
	k += (i>>20);
	f = x-1.0;
	if((0x000fffff&(2+hx))<3) {	/* |f| < 2**-20 */
	    if(f==__fdlibm_zero) {if(k==0) return __fdlibm_zero;  else {dk=(double)k;
				 return dk*__fdlibm_ln2_hi+dk*__fdlibm_ln2_lo;}}
	    R = f*f*(0.5-0.33333333333333333*f);
	    if(k==0) return f-R; else {dk=(double)k;
	    	     return dk*__fdlibm_ln2_hi-((R-dk*__fdlibm_ln2_lo)-f);}
	}
 	s = f/(2.0+f); 
	dk = (double)k;
	z = s*s;
	i = hx-0x6147a;
	w = z*z;
	j = 0x6b851-hx;
	t1= w*(__fdlibm_Lg2+w*(__fdlibm_Lg4+w*__fdlibm_Lg6)); 
	t2= z*(__fdlibm_Lg1+w*(__fdlibm_Lg3+w*(__fdlibm_Lg5+w*__fdlibm_Lg7))); 
	i |= j;
	R = t2+t1;
	if(i>0) {
	    hfsq=0.5*f*f;
	    if(k==0) return f-(hfsq-s*(hfsq+R)); else
		     return dk*__fdlibm_ln2_hi-((hfsq-(s*(hfsq+R)+dk*__fdlibm_ln2_lo))-f);
	} else {
	    if(k==0) return f-s*(f-R); else
		     return dk*__fdlibm_ln2_hi-((s*(f-R)-dk*__fdlibm_ln2_lo)-f);
	}
}

