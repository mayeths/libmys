/**
 * @file headers.h
 * @author mayeths (wow@mayeths.com)
 * @version 1.0
 * @brief Basic C header of libmys. (Require C99)
 * 
 * Includes necessary C headers and defines fundamental macros here.
 * No other *.h and *.hpp headers in libmys should be include here.
 */
#pragma once

#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
#error Require at least c99 to parse *.h in libmys
#endif

/* C headers */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Essential libraries headers. */
/* 1. MPI - Currently I only use libmys under MPI application. */
#include <mpi.h>

/* Optional libraries headers. void-out the functions we used if not presented. */
/* 1. OpenMP */
#ifndef _OPENMP
#define omp_get_thread_num(...) (0)
#define omp_get_num_threads(...) (1)
#define omp_get_max_threads(...) (1)
#else
#include <omp.h>
#endif


#define IDX2(x, y, nx, ny) ((x) + (y) * (nx))
#define IDX3(x, y, z, nx, ny, nz) ((x) + (y) * (nx) + (z) * (nx) * (ny))
