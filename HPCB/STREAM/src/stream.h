#ifndef _STREAM_H_
#define _STREAM_H_

#define _XOPEN_SOURCE 600
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#ifdef _OPENMP
#include <omp.h>
#endif
#include <mpi.h>

#include "common.h"

#define HLINE "--------------------------------------------------------------\n"

/* Testcase Routines
 *   desc_function_t:  Testcase description
 *   init_function_t:  Testcase initializer
 *   fini_function_t:  Testcase finalizer
 *   copy_function_t:  Perform copy c[:] = a[:]
 *   scale_function_t: Perform scale c[:] = scalar * a[:]
 *   add_function_t:   Perform add c[:] = a[:] + b[:]
 *   triad_function_t: Perform triad c[:] = scalar * a[:] + b[:]
 */
typedef const char * (*desc_function_t)();
typedef void (*init_function_t)(MPI_Comm comm, int narr, const STREAM_TYPE scalar, void **metadata);
typedef void (*fini_function_t)(void **metadata);
typedef void (*copy_function_t)(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a);
typedef void (*scale_function_t)(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a);
typedef void (*add_function_t)(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a, const STREAM_TYPE *b);
typedef void (*triad_function_t)(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a, const STREAM_TYPE *b);

/* Naive version contains openmp pragma (Currently not enabling openmp). */
const char *naive_d();
void naive_i(MPI_Comm comm, int narr, const STREAM_TYPE scalar, void **metadata);
void naive_f(void **metadata);
void naive_c(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a);
void naive_s(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a);
void naive_a(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a, const STREAM_TYPE *b);
void naive_t(void *metadata, STREAM_TYPE *c, const STREAM_TYPE *a, const STREAM_TYPE *b);

#ifdef TEST_SUNWAY_GLD
const char *swgld_d();
void swgld_i(MPI_Comm comm, int narr, const double scalar, void **metadata);
void swgld_f(void **metadata);
void swgld_c(void *metadata, double *c, const double *a);
void swgld_s(void *metadata, double *c, const double *a);
void swgld_a(void *metadata, double *c, const double *a, const double *b);
void swgld_t(void *metadata, double *c, const double *a, const double *b);
#endif

#ifdef TEST_SUNWAY_DMA
const char *swdma_d();
void swdma_i(MPI_Comm comm, int narr, const double scalar, void **metadata);
void swdma_f(void **metadata);
void swdma_c(void *metadata, double *c, const double *a);
void swdma_s(void *metadata, double *c, const double *a);
void swdma_a(void *metadata, double *c, const double *a, const double *b);
void swdma_t(void *metadata, double *c, const double *a, const double *b);
#endif


typedef struct testsuite_t {
	const char *name;
	desc_function_t d;
	init_function_t i;
	fini_function_t f;
	copy_function_t c;
	scale_function_t s;
	add_function_t a;
	triad_function_t t;
} testsuite_t;

static const testsuite_t testsuites[] = {
	{.name = "Naive", .d=naive_d, .i=naive_i, .f=naive_f, .c=naive_c, .s=naive_s, .a=naive_a, .t=naive_t},
#ifdef TEST_SUNWAY_GLD
	{.name = "Sunway:GLD", .d=swgld_d, .i=swgld_i, .f=swgld_f, .c=swgld_c, .s=swgld_s, .a=swgld_a, .t=swgld_t},
#endif
#ifdef TEST_SUNWAY_DMA
	{.name = "Sunway:DMA", .d=swdma_d, .i=swdma_i, .f=swdma_f, .c=swdma_c, .s=swdma_s, .a=swdma_a, .t=swdma_t},
#endif
};

static const size_t ntest = sizeof(testsuites) / sizeof(testsuite_t);

#endif /*_STREAM_H_*/