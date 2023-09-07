// from netlib-math/e_pow.c
#pragma once

#include "_fdlibm.h"

//-----------------------------

MYS_STATIC double _mys_math_pow(double x, double y)
{
	double z,ax,z_h,z_l,p_h,p_l;
	double y1,t1,t2,r,s,t,u,v,w;
	int i0,i1,i,j,k,yisint,n;
	int hx,hy,ix,iy;
	unsigned lx,ly;

	(void)i1;
	union _fdlibm_num_t num;
	num.f64 = __fdlibm_one;
	i0 = (num.i32.l>>29)^1; i1=1-i0;
	hx = _FDLIBM_HI(x); lx = _FDLIBM_LO(x);
	hy = _FDLIBM_HI(y); ly = _FDLIBM_LO(y);
	ix = hx&0x7fffffff;  iy = hy&0x7fffffff;

    /* y==__fdlibm_zero: x**0 = 1 */
	if((iy|ly)==0) return __fdlibm_one; 	

    /* +-NaN return x+y */
	if(ix > 0x7ff00000 || ((ix==0x7ff00000)&&(lx!=0)) ||
	   iy > 0x7ff00000 || ((iy==0x7ff00000)&&(ly!=0))) 
		return x+y;	

    /* determine if y is an odd int when x < 0
     * yisint = 0	... y is not an integer
     * yisint = 1	... y is an odd int
     * yisint = 2	... y is an even int
     */
	yisint  = 0;
	if(hx<0) {	
	    if(iy>=0x43400000) yisint = 2; /* even integer y */
	    else if(iy>=0x3ff00000) {
		k = (iy>>20)-0x3ff;	   /* exponent */
		if(k>20) {
		    j = ly>>(52-k);
		    if((long long int)(j<<(52-k))==(long long int)ly) yisint = 2-(j&1);
		} else if(ly==0) {
		    j = iy>>(20-k);
		    if((j<<(20-k))==iy) yisint = 2-(j&1);
		}
	    }		
	} 

    /* special value of y */
	if(ly==0) { 	
	    if (iy==0x7ff00000) {	/* y is +-inf */
	        if(((ix-0x3ff00000)|lx)==0)
		    return  y - y;	/* inf**+-1 is NaN */
	        else if (ix >= 0x3ff00000)/* (|x|>1)**+-inf = inf,0 */
		    return (hy>=0)? y: __fdlibm_zero;
	        else			/* (|x|<1)**-,+inf = inf,0 */
		    return (hy<0)?-y: __fdlibm_zero;
	    } 
	    if(iy==0x3ff00000) {	/* y is  +-1 */
		if(hy<0) return __fdlibm_one/x; else return x;
	    }
	    if(hy==0x40000000) return x*x; /* y is  2 */
	    if(hy==0x3fe00000) {	/* y is  0.5 */
		if(hx>=0)	/* x >= +0 */
		return _mys_math_sqrt(x);	
	    }
	}

	ax   = _mys_math_fabs(x);
    /* special value of x */
	if(lx==0) {
	    if(ix==0x7ff00000||ix==0||ix==0x3ff00000){
		z = ax;			/*x is +-0,+-inf,+-1*/
		if(hy<0) z = __fdlibm_one/z;	/* z = (1/|x|) */
		if(hx<0) {
		    if(((ix-0x3ff00000)|yisint)==0) {
			z = (z-z)/(z-z); /* (-1)**non-int is NaN */
		    } else if(yisint==1) 
			z = -z;		/* (x<0)**odd = -(|x|**odd) */
		}
		return z;
	    }
	}
    
	n = (hx>>31)+1;

    /* (x<0)**(non-int) is NaN */
	if((n|yisint)==0) return (x-x)/(x-x);

	s = __fdlibm_one; /* s (sign of result -ve**odd) = -1 else = 1 */
	if((n|(yisint-1))==0) s = -__fdlibm_one;/* (-ve)**(odd int) */

    /* |y| is __fdlibm_huge */
	if(iy>0x41e00000) { /* if |y| > 2**31 */
	    if(iy>0x43f00000){	/* if |y| > 2**64, must o/uflow */
		if(ix<=0x3fefffff) return (hy<0)? __fdlibm_huge*__fdlibm_huge:__fdlibm_tiny*__fdlibm_tiny;
		if(ix>=0x3ff00000) return (hy>0)? __fdlibm_huge*__fdlibm_huge:__fdlibm_tiny*__fdlibm_tiny;
	    }
	/* over/underflow if x is not close to __fdlibm_one */
	    if(ix<0x3fefffff) return (hy<0)? s*__fdlibm_huge*__fdlibm_huge:s*__fdlibm_tiny*__fdlibm_tiny;
	    if(ix>0x3ff00000) return (hy>0)? s*__fdlibm_huge*__fdlibm_huge:s*__fdlibm_tiny*__fdlibm_tiny;
	/* now |1-x| is __fdlibm_tiny <= 2**-20, suffice to compute 
	   log(x) by x-x^2/2+x^3/3-x^4/4 */
	    t = ax-__fdlibm_one;		/* t has 20 trailing zeros */
	    w = (t*t)*(0.5-t*(0.3333333333333333333333-t*0.25));
	    u = __fdlibm_ivln2_h*t;	/* __fdlibm_ivln2_h has 21 sig. bits */
	    v = t*__fdlibm_ivln2_l-w*__fdlibm_ivln2;
	    t1 = u+v;
        t1 = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(t1), 0);
	    t2 = v-(t1-u);
	} else {
	    double ss,s2,s_h,s_l,t_h,t_l;
	    n = 0;
	/* take care subnormal number */
	    if(ix<0x00100000)
		{ax *= __fdlibm_two53; n -= 53; ix = _FDLIBM_HI(ax); }
	    n  += ((ix)>>20)-0x3ff;
	    j  = ix&0x000fffff;
	/* determine interval */
	    ix = j|0x3ff00000;		/* normalize ix */
	    if(j<=0x3988E) k=0;		/* |x|<sqrt(3/2) */
	    else if(j<0xBB67A) k=1;	/* |x|<sqrt(3)   */
	    else {k=0;n+=1;ix -= 0x00100000;}
        ax = _FDLIBM_FORM_DOUBLE(ix, _FDLIBM_LO(ax));

	/* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
	    u = ax-__fdlibm_bp[k];		/* __fdlibm_bp[0]=1.0, __fdlibm_bp[1]=1.5 */
	    v = __fdlibm_one/(ax+__fdlibm_bp[k]);
	    ss = u*v;
	    s_h = ss;
        s_h = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(s_h), 0);
	/* t_h=ax+__fdlibm_bp[k] High */
	    t_h = __fdlibm_zero;
        t_h = _FDLIBM_FORM_DOUBLE(((ix>>1)|0x20000000)+0x00080000+(k<<18), _FDLIBM_LO(t_h));
	    t_l = ax - (t_h-__fdlibm_bp[k]);
	    s_l = v*((u-s_h*t_h)-s_h*t_l);
	/* compute log(ax) */
	    s2 = ss*ss;
	    r = s2*s2*(__fdlibm_L1+s2*(__fdlibm_L2+s2*(__fdlibm_L3+s2*(__fdlibm_L4+s2*(__fdlibm_L5+s2*__fdlibm_L6)))));
	    r += s_l*(s_h+ss);
	    s2  = s_h*s_h;
	    t_h = 3.0+s2+r;
        t_h = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(t_h), 0);
	    t_l = r-((t_h-3.0)-s2);
	/* u+v = ss*(1+...) */
	    u = s_h*t_h;
	    v = s_l*t_h+t_l*ss;
	/* 2/(3log2)*(ss+...) */
	    p_h = u+v;
        p_h = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(p_h), 0);
	    p_l = v-(p_h-u);
	    z_h = __fdlibm_cp_h*p_h;		/* __fdlibm_cp_h+__fdlibm_cp_l = 2/(3*log2) */
	    z_l = __fdlibm_cp_l*p_h+p_l*__fdlibm_cp+__fdlibm_dp_l[k];
	/* log2(ax) = (ss+..)*2/(3*log2) = n + __fdlibm_dp_h + z_h + z_l */
	    t = (double)n;
	    t1 = (((z_h+z_l)+__fdlibm_dp_h[k])+t);
        t1 = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(t1), 0);
	    t2 = z_l-(((t1-t)-__fdlibm_dp_h[k])-z_h);
	}

    /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
	y1  = y;
    y1 = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(y1), 0);
	p_l = (y-y1)*t1+y*t2;
	p_h = y1*t1;
	z = p_l+p_h;
	j = _FDLIBM_HI(z);
	i = _FDLIBM_LO(z);
	if (j>=0x40900000) {				/* z >= 1024 */
	    if(((j-0x40900000)|i)!=0)			/* if z > 1024 */
		return s*__fdlibm_huge*__fdlibm_huge;			/* overflow */
	    else {
		if(p_l+__fdlibm_ovt>z-p_h) return s*__fdlibm_huge*__fdlibm_huge;	/* overflow */
	    }
	} else if((j&0x7fffffff)>=0x4090cc00 ) {	/* z <= -1075 */
	    if(((j-0xc090cc00)|i)!=0) 		/* z < -1075 */
		return s*__fdlibm_tiny*__fdlibm_tiny;		/* underflow */
	    else {
		if(p_l<=z-p_h) return s*__fdlibm_tiny*__fdlibm_tiny;	/* underflow */
	    }
	}
    /*
     * compute 2**(p_h+p_l)
     */
	i = j&0x7fffffff;
	k = (i>>20)-0x3ff;
	n = 0;
	if(i>0x3fe00000) {		/* if |z| > 0.5, set n = [z+0.5] */
	    n = j+(0x00100000>>(k+1));
	    k = ((n&0x7fffffff)>>20)-0x3ff;	/* new k for n */
	    t = __fdlibm_zero;
        t = _FDLIBM_FORM_DOUBLE((n&~(0x000fffff>>k)), _FDLIBM_LO(t));
	    n = ((n&0x000fffff)|0x00100000)>>(20-k);
	    if(j<0) n = -n;
	    p_h -= t;
	} 
	t = p_l+p_h;
    t = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(t), 0);
	u = t*__fdlibm_lg2_h;
	v = (p_l-(t-p_h))*__fdlibm_lg2+t*__fdlibm_lg2_l;
	z = u+v;
	w = v-(z-u);
	t  = z*z;
	t1  = z - t*(__fdlibm_P1+t*(__fdlibm_P2+t*(__fdlibm_P3+t*(__fdlibm_P4+t*__fdlibm_P5))));
	r  = (z*t1)/(t1-__fdlibm_two)-(w+z*w);
	z  = __fdlibm_one-(r-z);
	j  = _FDLIBM_HI(z);
	j += (n<<20);
	if((j>>20)<=0) z = _mys_math_scalbn(z,n);	/* subnormal output */
	else z = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(z) + (n<<20), _FDLIBM_LO(z));
	return s*z;
}
