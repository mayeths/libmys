#pragma once

// This file is used to include declare of private functions that internally used by libmys.
// We don't expect users to use them.

#include "../_config.h"
#include "../mpi.h"

// mpi.c

MYS_STATIC int _mys_MPI_Initialized(int *flag);
MYS_STATIC int _mys_MPI_Init_thread(int *argc, char ***argv, int required, int *provided);
MYS_STATIC int _mys_MPI_Comm_rank(_mys_MPI_Comm comm, int *rank);
MYS_STATIC int _mys_MPI_Comm_size(_mys_MPI_Comm comm, int *size);
MYS_STATIC int _mys_MPI_Recv(void *buf, int count, _mys_MPI_Datatype datatype, int source, int tag, _mys_MPI_Comm comm, _mys_MPI_Status *status);
MYS_STATIC int _mys_MPI_Send(const void *buf, int count, _mys_MPI_Datatype datatype, int dest, int tag, _mys_MPI_Comm comm);
MYS_STATIC int _mys_MPI_Barrier(_mys_MPI_Comm comm);
MYS_STATIC int _mys_MPI_Allreduce(void *sendbuf, void *recvbuf, int count, _mys_MPI_Datatype datatype, _mys_MPI_Op op, _mys_MPI_Comm comm);
MYS_STATIC int _mys_MPI_Probe(int source, int tag, _mys_MPI_Comm comm, _mys_MPI_Status *status);
MYS_STATIC int _mys_MPI_Get_count(_mys_MPI_Status *status, _mys_MPI_Datatype datatype, int *count);
MYS_STATIC double _mys_MPI_Wtime();


// math.c

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
