#include "_private.h"
#include "../math.h"

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

union _fdlibm_num_t {
    double f64;
    uint64_t u64;
    struct {          int l;          int h; } i32; // Do not change order of l and h
    struct { unsigned int l; unsigned int h; } u32; // Do not change order of l and h
    struct {        float l;        float h; } f32; // Do not change order of l and h
};
static uint64_t _FDLIBM_D2U(double x)   { union _fdlibm_num_t a; a.f64 = x; return a.u64; }
static double   _FDLIBM_U2D(uint64_t x) { union _fdlibm_num_t a; a.u64 = x; return a.f64; }
static int32_t  _FDLIBM_HI(double x)    { return (int32_t)(_FDLIBM_D2U(x) >> 32); }
static uint32_t _FDLIBM_LO(double x)    { return (uint32_t)(_FDLIBM_D2U(x)); }
static double   _FDLIBM_FORM_DOUBLE(int32_t h, uint32_t l) { return _FDLIBM_U2D(((int64_t)(h) << 32) | (uint64_t)(l)); }

static const double
    __fdlibm_ln2_hi  =  6.93147180369123816490e-01,	/* 3fe62e42 fee00000 */
    __fdlibm_ln2_lo  =  1.90821492927058770002e-10,	/* 3dea39ef 35793c76 */
    __fdlibm_two53   =  9007199254740992.0,	/* 0x43400000, 0x00000000 */
    __fdlibm_two54   =  1.80143985094819840000e+16,  /* 43500000 00000000 */
    __fdlibm_twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
    __fdlibm_Lg1 = 6.666666666666735130e-01,  /* 3FE55555 55555593 */
    __fdlibm_Lg2 = 3.999999999940941908e-01,  /* 3FD99999 9997FA04 */
    __fdlibm_Lg3 = 2.857142874366239149e-01,  /* 3FD24924 94229359 */
    __fdlibm_Lg4 = 2.222219843214978396e-01,  /* 3FCC71C5 1D8E78AF */
    __fdlibm_Lg5 = 1.818357216161805012e-01,  /* 3FC74664 96CB03DE */
    __fdlibm_Lg6 = 1.531383769920937332e-01,  /* 3FC39A09 D078C69F */
    __fdlibm_Lg7 = 1.479819860511658591e-01,  /* 3FC2F112 DF3E5244 */
    __fdlibm_ivln10     =  4.34294481903251816668e-01, /* 0x3FDBCB7B, 0x1526E50E */
    __fdlibm_log10_2hi  =  3.01029995663611771306e-01, /* 0x3FD34413, 0x509F6000 */
    __fdlibm_log10_2lo  =  3.69423907715893078616e-13, /* 0x3D59FEF3, 0x11F12B36 */
    __fdlibm_zero = 0.0,
    __fdlibm_one  = 1.0,
    __fdlibm_two  = 2.0,
    __fdlibm_huge = 1.0e+300,
    __fdlibm_tiny = 1.0e-300,
    __fdlibm_dp_h[] = { 0.0, 5.84962487220764160156e-01,}, /* 0x3FE2B803, 0x40000000 */
    __fdlibm_dp_l[] = { 0.0, 1.35003920212974897128e-08,}, /* 0x3E4CFDEB, 0x43CFD006 */
    __fdlibm_bp[]   = {1.0, 1.5,},
    __fdlibm_L1   =  5.99999999999994648725e-01, /* 0x3FE33333, 0x33333303 */
    __fdlibm_L2   =  4.28571428578550184252e-01, /* 0x3FDB6DB6, 0xDB6FABFF */
    __fdlibm_L3   =  3.33333329818377432918e-01, /* 0x3FD55555, 0x518F264D */
    __fdlibm_L4   =  2.72728123808534006489e-01, /* 0x3FD17460, 0xA91D4101 */
    __fdlibm_L5   =  2.30660745775561754067e-01, /* 0x3FCD864A, 0x93C9DB65 */
    __fdlibm_L6   =  2.06975017800338417784e-01, /* 0x3FCA7E28, 0x4A454EEF */
    __fdlibm_P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
    __fdlibm_P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
    __fdlibm_P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
    __fdlibm_P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
    __fdlibm_P5   =  4.13813679705723846039e-08, /* 0x3E663769, 0x72BEA4D0 */
    __fdlibm_lg2    =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
    __fdlibm_lg2_h  =  6.93147182464599609375e-01, /* 0x3FE62E43, 0x00000000 */
    __fdlibm_lg2_l  = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
    __fdlibm_ovt    =  8.0085662595372944372e-0017, /* -(1024-log2(ovfl+.5ulp)) */
    __fdlibm_cp     =  9.61796693925975554329e-01, /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
    __fdlibm_cp_h   =  9.61796700954437255859e-01, /* 0x3FEEC709, 0xE0000000 =(float)__fdlibm_cp */
    __fdlibm_cp_l   = -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 =tail of __fdlibm_cp_h*/
    __fdlibm_ivln2    =  1.44269504088896338700e+00, /* 0x3FF71547, 0x652B82FE =1/ln2 */
    __fdlibm_ivln2_h  =  1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
    __fdlibm_ivln2_l  =  1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/


// from netlib-math/s_copysign.c
MYS_PUBLIC double mys_math_copysign(double x, double y)
{
    x = _FDLIBM_FORM_DOUBLE((_FDLIBM_HI(x)&0x7fffffff)|(_FDLIBM_HI(y)&0x80000000), _FDLIBM_LO(x));
        return x;
}

// from netlib-math/s_fabs.c
MYS_PUBLIC double mys_math_fabs(double x)
{
    x = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(x)&0x7fffffff, _FDLIBM_LO(x));
        return x;
}

// from netlib-math/e_log.c
MYS_PUBLIC double mys_math_log(double x)
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

// from netlib-math/e_log10.c
MYS_PUBLIC double mys_math_log10(double x)
{
	double y,z;
	int i,k,hx;
	unsigned lx;
    static double zero = 0.0;

	hx = _FDLIBM_HI(x);	/* high word of x */
	lx = _FDLIBM_LO(x);	/* low word of x */

        k=0;
        if (hx < 0x00100000) {                  /* x < 2**-1022  */
            if (((hx&0x7fffffff)|lx)==0)
                return -__fdlibm_two54/zero;             /* log(+-0)=-inf */
            if (hx<0) return (x-x)/zero;        /* log(-#) = NaN */
            k -= 54; x *= __fdlibm_two54; /* subnormal number, scale up x */
            hx = _FDLIBM_HI(x);                /* high word of x */
        }
	if (hx >= 0x7ff00000) return x+x;
	k += (hx>>20)-1023;
	i  = ((unsigned)k&0x80000000)>>31;
        hx = (hx&0x000fffff)|((0x3ff-i)<<20);
        y  = (double)(k+i);
        x = _FDLIBM_FORM_DOUBLE(hx, lx);
	z  = y*__fdlibm_log10_2lo + __fdlibm_ivln10*mys_math_log(x);
	return  z+y*__fdlibm_log10_2hi;
}

// from netlib-math/e_pow.c
MYS_PUBLIC double mys_math_pow(double x, double y)
{
	double z,ax,z_h,z_l,p_h,p_l;
	double y1,t1,t2,r,s,t,u,v,w;
	int i,j,k,yisint,n;
	int hx,hy,ix,iy;
	unsigned lx,ly;

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
		return mys_math_sqrt(x);	
	    }
	}

	ax   = mys_math_fabs(x);
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
	if((j>>20)<=0) z = mys_math_scalbn(z,n);	/* subnormal output */
	else z = _FDLIBM_FORM_DOUBLE(_FDLIBM_HI(z) + (n<<20), _FDLIBM_LO(z));
	return s*z;
}

// from netlib-math/s_scalbn.c
MYS_PUBLIC double mys_math_scalbn (double x, int n)
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
        if (k >  0x7fe) return __fdlibm_huge*mys_math_copysign(__fdlibm_huge,x); /* overflow  */
        if (k > 0) 				/* normal result */
	    {x = _FDLIBM_FORM_DOUBLE((hx&0x800fffff)|(k<<20), _FDLIBM_LO(x)); return x;}
        if (k <= -54) {
            if (n > 50000) 	/* in case integer overflow in n+k */
		{return __fdlibm_huge*mys_math_copysign(__fdlibm_huge,x);	/*overflow*/}
	    else {return __fdlibm_tiny*mys_math_copysign(__fdlibm_tiny,x); 	/*underflow*/}
        }
        k += 54;				/* subnormal result */
        x = _FDLIBM_FORM_DOUBLE((hx&0x800fffff)|(k<<20), _FDLIBM_LO(x));
        return x*__fdlibm_twom54;
}

// from netlib-math/e_sqrt.c
MYS_PUBLIC double mys_math_sqrt(double x)
{
	double z;
	int 	sign = (int)0x80000000; 
	unsigned r,t1,s1,ix1,q1;
	int ix0,s0,q,m,t,i;

	ix0 = _FDLIBM_HI(x);			/* high word of x */
	ix1 = _FDLIBM_LO(x);		/* low word of x */

    /* take care of Inf and NaN */
	if((ix0&0x7ff00000)==0x7ff00000) {			
	    return x*x+x;		/* sqrt(NaN)=NaN, sqrt(+inf)=+inf
					   sqrt(-inf)=sNaN */
	} 
    /* take care of zero */
	if(ix0<=0) {
	    if(((ix0&(~sign))|ix1)==0) return x;/* sqrt(+-0) = +-0 */
	    else if(ix0<0)
		return (x-x)/(x-x);		/* sqrt(-ve) = sNaN */
	}
    /* normalize x */
	m = (ix0>>20);
	if(m==0) {				/* subnormal x */
	    while(ix0==0) {
		m -= 21;
		ix0 |= (ix1>>11); ix1 <<= 21;
	    }
	    for(i=0;(ix0&0x00100000)==0;i++) ix0<<=1;
	    m -= i-1;
	    ix0 |= (ix1>>(32-i));
	    ix1 <<= i;
	}
	m -= 1023;	/* unbias exponent */
	ix0 = (ix0&0x000fffff)|0x00100000;
	if(m&1){	/* odd m, double x to make it even */
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	}
	m >>= 1;	/* m = [m/2] */

    /* generate sqrt(x) bit by bit */
	ix0 += ix0 + ((ix1&sign)>>31);
	ix1 += ix1;
	q = q1 = s0 = s1 = 0;	/* [q,q1] = sqrt(x) */
	r = 0x00200000;		/* r = moving bit from right to left */

	while(r!=0) {
	    t = s0+r; 
	    if(t<=ix0) { 
		s0   = t+r; 
		ix0 -= t; 
		q   += r; 
	    } 
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	    r>>=1;
	}

	r = sign;
	while(r!=0) {
	    t1 = s1+r; 
	    t  = s0;
	    if((t<ix0)||((t==ix0)&&(t1<=ix1))) { 
		s1  = t1+r;
		if(((t1&sign)==(unsigned)sign)&&(s1&sign)==0) s0 += 1;
		ix0 -= t;
		if (ix1 < t1) ix0 -= 1;
		ix1 -= t1;
		q1  += r;
	    }
	    ix0 += ix0 + ((ix1&sign)>>31);
	    ix1 += ix1;
	    r>>=1;
	}

    /* use floating add to find out rounding direction */
	if((ix0|ix1)!=0) {
	    z = __fdlibm_one-__fdlibm_tiny; /* trigger inexact flag */
	    if (z>=__fdlibm_one) {
	        z = __fdlibm_one+__fdlibm_tiny;
	        if (q1==(unsigned)0xffffffff) { q1=0; q += 1;}
		else if (z>__fdlibm_one) {
		    if (q1==(unsigned)0xfffffffe) q+=1;
		    q1+=2; 
		} else
	            q1 += (q1&1);
	    }
	}
	ix0 = (q>>1)+0x3fe00000;
	ix1 =  q1>>1;
	if ((q&1)==1) ix1 |= sign;
	ix0 += (m <<20);
    z = _FDLIBM_FORM_DOUBLE(ix0, ix1);
	return z;
}

// from musl-1.2.4/src/math/trunc.c
MYS_PUBLIC double mys_math_trunc(double x)
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

#undef __musl_fp_force_evalf
#undef __musl_fp_force_eval
#undef __musl_fp_force_evall
#undef _MUSL_FORCE_EVAL
