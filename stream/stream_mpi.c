/*-----------------------------------------------------------------------*/
/* Program: STREAM MPI variant (forked from version 5.10)                */
/* See https://www.cs.virginia.edu/stream/FTP/Code/Versions/stream_mpi.c */
/* Original code developed by John D. McCalpin                           */
/* Programmers: John D. McCalpin                                         */
/*              Joe R. Zagar                                             */
/*              Mayeths                                                  */
/* This program measures memory transfer rates in MB/s for simple        */
/* computational kernels coded in C.                                     */
/*-----------------------------------------------------------------------*/
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <mpi.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "os.h"

static const char *usage =
	"=== STREAM version 5.10 (modified by Mayeths)\n"
	"=== Usage\n"
	"    %s ncores min_size\n"
	"    * Use the size of LLC (LLC_size) as min_size if it's larger than 32MB.\n"
	"    [Kunpeng920] ./stream_mpi.exe 128 48MB (for 128 cores and 48MB shared L3)\n"
	"    [Apple M1] ./stream_mpi.exe 8 128MB (for 8 cores and 16MB shared L2)\n"
	"=== Size Algorithm (by Mayeths)\n"
	"    local_array_size = min_size * (log2(ncores)+1) / (log2(nranks)+1)\n"
	"    * local_array_size is min_size * log2(ncores) when utilizing one core.\n"
	"    * local_array_size is min_size when utilizing all cores.\n"
	"    * global_array_size ranges from [min_size * (log2(ncores)+1), min_size * ncores]\n"
	"=== Note (from original STREAM)\n"
	"    * local_array_size must be large enough that the traversal cost > 20 clock-ticks.\n"
	"    * Ensure global_array_size >= 4 * LLC_size. So min_size > 4 * LLC_size if ncores <= 8.\n"
;

// Run each kernel "NTIMES" times and reports the best result for any
// iteration after the firsttherefore the minimum value for NTIMES is 2.
#ifdef NTIMES
#if NTIMES <= 1
#define NTIMES 10
#endif
#endif
#ifndef NTIMES
#define NTIMES 10
#endif

// Use "STREAM_TYPE" as the element type
#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

// The "SCALAR" 0.42 allows over 2000 iterations for 32-bit IEEE arithmetic
// and over 18000 iterations for 64-bit IEEE arithmetic.
#ifndef SCALAR
#define SCALAR 0.42
#endif

# define HLINE "--------------------------------------------------------------\n"

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

static void exit_with_usage(FILE *fd, const char *prog, const char *reason, ...) {
	int myrank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	if (myrank == 0) {
		if (reason != NULL) {
			va_list opt;
			va_start(opt, reason);
			vfprintf(fd, reason, opt);
			va_end(opt);
		}

		int len = snprintf(NULL, 0, usage, prog) + 1;
		char *buffer = (char *)malloc(sizeof(char) * len);
		snprintf(buffer, len, usage, prog);
		fprintf(fd, "%s", buffer);
		fflush(fd);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	exit(0);
}

// Some compilers require an extra keyword to recognize the "restrict" qualifier.
double * restrict a, * restrict b, * restrict c;

size_t		array_elements, array_bytes, array_alignment;
static double	avgtime[4] = {0}, maxtime[4] = {0},
		mintime[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};

static char	*label[4] = {"Copy:      ", "Scale:     ",
    "Add:       ", "Triad:     "};

extern void checkSTREAMresults(STREAM_TYPE *AvgErrByRank, int numranks, double epsilon);
extern void computeSTREAMerrors(STREAM_TYPE *aAvgErr, STREAM_TYPE *bAvgErr, STREAM_TYPE *cAvgErr);
extern double checktick();

int
main(int argc, char **argv)
    {
    double		quantum = 0;
    int			i,k;
    ssize_t		j;
    STREAM_TYPE		scalar;
    double		t, times[4][NTIMES];
	double		*TimesByRank = NULL;
	double		t0,t1,tmin;
	STREAM_TYPE	AvgError[3] = {0.0,0.0,0.0};
	STREAM_TYPE *AvgErrByRank = NULL;

    /* --- SETUP --- call MPI_Init() before anything else! --- */
    if (MPI_Init(NULL, NULL) != MPI_SUCCESS) {
       printf("ERROR: MPI Initialization failed\n");
       exit(1);
    }
	// if either of these fail there is something really screwed up!
	int numranks, myrank;
	MPI_Comm_size(MPI_COMM_WORLD, &numranks);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
#ifdef _OPENMP
	int numthreads = omp_get_max_threads();
#endif

	if (argc != 3)
		exit_with_usage(stderr, argv[0], NULL, NULL);

	int call_help = 0;
	call_help |= strncmp(argv[1], "-h", sizeof("-h")) == 0;
	call_help |= strncmp(argv[1], "--help", sizeof("--help")) == 0;
	if (call_help) {
		exit_with_usage(stderr, argv[0], NULL, NULL);
	}

	int ncores = str_to_i32(argv[1], -1);
	if (ncores == -1) {
		exit_with_usage(stderr, argv[0], "ERROR invalid ncores \"%s\"\n", argv[1]);
	}

	ssize_t min_size = from_readable_size(argv[2]);
	if (min_size == -1) {
		exit_with_usage(stderr, argv[0], "ERROR invalid min_size \"%s\"\n", argv[2]);
	}

	// Use log2() to prevent numranks=1 got too large array
	// size_t local_array_size = (size_t)((double)min_size * (double)ncores / (double)numranks);
	size_t local_array_size = (size_t)((double)min_size * (log2(ncores) + 1) / (log2(numranks) + 1));
	size_t global_array_size = local_array_size * numranks;
	size_t STREAM_ARRAY_SIZE = global_array_size / sizeof(STREAM_TYPE);

	double bytes[4] = {
		2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
		2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
		3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
		3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE
	};

	double epsilon = -1;
	if (sizeof(STREAM_TYPE) == 4)
		epsilon = 1.e-6;
	else if (sizeof(STREAM_TYPE) == 8)
		epsilon = 1.e-13;
	else {
		printf("WEIRD: sizeof(STREAM_TYPE) = %lu\n",sizeof(STREAM_TYPE));
		epsilon = 1.e-6;
	}

	t0 = MPI_Wtime();
    /* --- NEW FEATURE --- distribute requested storage across MPI ranks --- */
	array_elements = STREAM_ARRAY_SIZE / numranks;		// don't worry about rounding vs truncation
    array_alignment = 64;						// Can be modified -- provides partial support for adjusting relative alignment

	// Dynamically allocate the three arrays using "posix_memalign()"
    array_bytes = array_elements * sizeof(STREAM_TYPE);
    k = posix_memalign((void **)&a, array_alignment, array_bytes);
    if (k != 0) {
        printf("Rank %d: Allocation of array a failed, return code is %d\n",myrank,k);
		MPI_Abort(MPI_COMM_WORLD, 2);
        exit(1);
    }
    k = posix_memalign((void **)&b, array_alignment, array_bytes);
    if (k != 0) {
        printf("Rank %d: Allocation of array b failed, return code is %d\n",myrank,k);
		MPI_Abort(MPI_COMM_WORLD, 2);
        exit(1);
    }
    k = posix_memalign((void **)&c, array_alignment, array_bytes);
    if (k != 0) {
        printf("Rank %d: Allocation of array c failed, return code is %d\n",myrank,k);
		MPI_Abort(MPI_COMM_WORLD, 2);
        exit(1);
    }

    /* --- SETUP --- initialize arrays and estimate precision of timer --- */

#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (j=0; j<array_elements; j++) {
	    a[j] = 1.0;
	    b[j] = 2.0;
	    c[j] = 0.0;
	}

    /* Get initial timing estimate to compare to timer granularity. */
	/* All ranks need to run this code since it changes the values in array a */
    t = MPI_Wtime();
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for (j = 0; j < array_elements; j++)
		a[j] = 2.0E0 * a[j];
    t = MPI_Wtime() - t;
	quantum = checktick();

	// Initial informational printouts -- rank 0 handles all the output
	if (myrank == 0) {
		printf(HLINE);
		printf("STREAM version 5.10 (modified by Mayeths)\n");

		printf(HLINE);
		printf("MPI ranks: %d\n", numranks);
#ifdef _OPENMP
		printf("OpenMP threads: %d\n", numthreads);
#else
		printf("OpenMP threads: disabled\n");
#endif
		printf("Scalar value: %f\n", SCALAR);
		printf("Validation epsilon: %e\n", epsilon);
		if (quantum * 1e3 >= 1)
			printf("Timer granularity: ~ %d ms\n", (int)(quantum * 1e3));
		else if (quantum * 1e6 >= 1)
			printf("Timer granularity: ~ %d us\n", (int)(quantum * 1e6));
		else if (quantum * 1e9 >= 1)
			printf("Timer granularity: ~ %d ns\n", (int)(quantum * 1e9));
		else
			printf("Timer granularity: %.1f ns\n", quantum * 1e9);

		size_t nticks = (size_t)(t/quantum);
		if (t * 1e3 >= 1)
			printf("Array traversal cost: ~ %zu ms (%zu ticks)\n", (size_t)(t * 1e3), nticks);
		else if (t * 1e6 >= 1)
			printf("Array traversal cost: ~ %zu us (%zu ticks)\n", (size_t)(t * 1e6), nticks);
		else if (t * 1e9 >= 1)
			printf("Array traversal cost: ~ %zu ns (%zu ticks)\n", (size_t)(t * 1e9), nticks);
		else
			printf("Array traversal cost: %.1f ns (%zu ticks)\n", t * 1e9, nticks);

		if (nticks < 20)
			printf("====== WARNING: Increase the size for at least 20 ticks ======\n");

		printf("Repeat: %d (Report the best run excluding the first one)\n", NTIMES);
		printf(HLINE);
		size_t bytes_per_word = sizeof(STREAM_TYPE);
		char *s_local_array_size = NULL;
		char *s_local_rank_size = NULL;
		char *s_global_array_size = NULL;
		char *s_global_rank_size = NULL;
		to_readable_size(&s_local_array_size, local_array_size, 1);
		to_readable_size(&s_local_rank_size, local_array_size * 3, 1);
		to_readable_size(&s_global_array_size, global_array_size, 1);
		to_readable_size(&s_global_rank_size, global_array_size * 3, 1);
		printf("Element size: %zu bytes\n", bytes_per_word);
		printf("Local array length: %zu (%s, %s per rank)\n", array_elements, s_local_array_size, s_local_rank_size);
		printf("Global array length: %zu (%s, %s for all ranks)\n", STREAM_ARRAY_SIZE, s_global_array_size, s_global_rank_size);
		printf("Each rank uses 3 arrays to perform COPY, SCALE, ADD and TRIAD.\n");
		free(s_local_array_size);
		free(s_local_rank_size);
		free(s_global_array_size);
		free(s_global_rank_size);
	}

	// Rank 0 needs to allocate arrays to hold error data and timing data from
	// all ranks for analysis and output.
	// Allocate and instantiate the arrays here -- after the primary arrays 
	// have been instantiated -- so there is no possibility of having these 
	// auxiliary arrays mess up the NUMA placement of the primary arrays.

	if (myrank == 0) {
		// There are 3 average error values for each rank (using STREAM_TYPE).
		AvgErrByRank = (double *) malloc(3 * sizeof(STREAM_TYPE) * numranks);
		if (AvgErrByRank == NULL) {
			printf("Ooops -- allocation of arrays to collect errors on MPI rank 0 failed\n");
			MPI_Abort(MPI_COMM_WORLD, 2);
		}
		memset(AvgErrByRank,0,3*sizeof(STREAM_TYPE)*numranks);

		// There are 4*NTIMES timing values for each rank (always doubles)
		TimesByRank = (double *) malloc(4 * NTIMES * sizeof(double) * numranks);
		if (TimesByRank == NULL) {
			printf("Ooops -- allocation of arrays to collect timing data on MPI rank 0 failed\n");
			MPI_Abort(MPI_COMM_WORLD, 3);
		}
		memset(TimesByRank,0,4*NTIMES*sizeof(double)*numranks);
	}

    /*	--- MAIN LOOP --- repeat test cases NTIMES times --- */

    // This code has more barriers and timing calls than are actually needed, but
    // this should not cause a problem for arrays that are large enough to satisfy
    // the STREAM run rules.
	// MAJOR FIX!!!  Version 1.7 had the start timer for each loop *after* the
	// MPI_Barrier(), when it should have been *before* the MPI_Barrier().
    // 

	MPI_Barrier(MPI_COMM_WORLD);
    scalar = SCALAR;
    for (k=0; k<NTIMES; k++)
	{
		// kernel 1: Copy
		t0 = MPI_Wtime();
		MPI_Barrier(MPI_COMM_WORLD);
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (j=0; j<array_elements; j++)
			c[j] = a[j];
		MPI_Barrier(MPI_COMM_WORLD);
		t1 = MPI_Wtime();
		times[0][k] = t1 - t0;

		// kernel 2: Scale
		t0 = MPI_Wtime();
		MPI_Barrier(MPI_COMM_WORLD);
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (j=0; j<array_elements; j++)
			b[j] = scalar*c[j];
		MPI_Barrier(MPI_COMM_WORLD);
		t1 = MPI_Wtime();
		times[1][k] = t1-t0;
	
		// kernel 3: Add
		t0 = MPI_Wtime();
		MPI_Barrier(MPI_COMM_WORLD);
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (j=0; j<array_elements; j++)
			c[j] = a[j]+b[j];
		MPI_Barrier(MPI_COMM_WORLD);
		t1 = MPI_Wtime();
		times[2][k] = t1-t0;
	
		// kernel 4: Triad
		t0 = MPI_Wtime();
		MPI_Barrier(MPI_COMM_WORLD);
#ifdef _OPENMP
#pragma omp parallel for
#endif
		for (j=0; j<array_elements; j++)
			a[j] = b[j]+scalar*c[j];
		MPI_Barrier(MPI_COMM_WORLD);
		t1 = MPI_Wtime();
		times[3][k] = t1-t0;
	}

	t0 = MPI_Wtime();

    /*	--- SUMMARY --- */

	// Because of the MPI_Barrier() calls, the timings from any thread are equally valid. 
    // The best estimate of the maximum performance is the minimum of the "outside the barrier"
    // timings across all the MPI ranks.

	// Gather all timing data to MPI rank 0
	MPI_Gather(times, 4*NTIMES, MPI_DOUBLE, TimesByRank, 4*NTIMES, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// Rank 0 processes all timing data
	if (myrank == 0) {
		// for each iteration and each kernel, collect the minimum time across all MPI ranks
		// and overwrite the rank 0 "times" variable with the minimum so the original post-
		// processing code can still be used.
		for (k=0; k<NTIMES; k++) {
			for (j=0; j<4; j++) {
				tmin = 1.0e36;
				for (i=0; i<numranks; i++) {
					// printf("DEBUG: Timing: iter %d, kernel %lu, rank %d, tmin %f, TbyRank %f\n",k,j,i,tmin,TimesByRank[4*NTIMES*i+j*NTIMES+k]);
					tmin = MIN(tmin, TimesByRank[4*NTIMES*i+j*NTIMES+k]);
				}
				// printf("DEBUG: Final Timing: iter %d, kernel %lu, final tmin %f\n",k,j,tmin);
				times[j][k] = tmin;
			}
		}

	// Back to the original code, but now using the minimum global timing across all ranks
		for (k=1; k<NTIMES; k++) /* note -- skip first iteration */
		{
		for (j=0; j<4; j++)
			{
			avgtime[j] = avgtime[j] + times[j][k];
			mintime[j] = MIN(mintime[j], times[j][k]);
			maxtime[j] = MAX(maxtime[j], times[j][k]);
			}
		}
    
		// note that "bytes[j]" is the global array size, so no "numranks" is needed here
		printf(HLINE);
		printf("Function      Best MB/s     Avg time     Min time     Max time\n");
		for (j=0; j<4; j++) {
			avgtime[j] = avgtime[j]/(double)(NTIMES-1);

			printf("%s %11.1f  %11.6f  %11.6f  %11.6f\n", label[j],
			   1.0E-06 * bytes[j]/mintime[j],
			   avgtime[j],
			   mintime[j],
			   maxtime[j]);
		}
	}

    /* --- Every Rank Checks its Results --- */
#ifdef INJECTERROR
	a[11] = 100.0 * a[11];
#endif
	computeSTREAMerrors(&AvgError[0], &AvgError[1], &AvgError[2]);
	/* --- Collect the Average Errors for Each Array on Rank 0 --- */
	MPI_Gather(AvgError, 3, MPI_DOUBLE, AvgErrByRank, 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	/* -- Combined averaged errors and report on Rank 0 only --- */
	if (myrank == 0) {
#ifdef VERBOSE
		for (k=0; k<numranks; k++) {
			printf("VERBOSE: rank %d, AvgErrors %e %e %e\n",k,AvgErrByRank[3*k+0],
				AvgErrByRank[3*k+1],AvgErrByRank[3*k+2]);
		}
#endif
		printf(HLINE);
		checkSTREAMresults(AvgErrByRank,numranks, epsilon);
	}

#ifdef VERBOSE
	if (myrank == 0) {
		t1 = MPI_Wtime();
		printf("VERBOSE: total shutdown time for rank %d = %f seconds\n",myrank,t1-t0);
	}
#endif

	free(a);
	free(b);
	free(c);
	if (myrank == 0) {
		free(TimesByRank);
		free(AvgErrByRank);
	}

    MPI_Finalize();
	return(0);
}

#define M 32
double checktick() {
	double ticks[M];

	for (int i = 0; i < M; i++) {
		double t1 = MPI_Wtime();
		double t2 = 0;
		size_t count = 0;
		while ((t2 = MPI_Wtime()) - t1 < 1e-6)
			count++;
		ticks[i] = (t2 - t1) / (double)count;
	}

	double tick = FLT_MAX;
	for (int i = 1; i < M; i++)
		tick = MIN(tick, ticks[i]);

	return tick;
}


// ----------------------------------------------------------------------------------
// For the MPI code I separate the computation of errors from the error
// reporting output functions (which are handled by MPI rank 0).
// ----------------------------------------------------------------------------------
#ifndef abs
#define abs(a) ((a) >= 0 ? (a) : -(a))
#endif
void computeSTREAMerrors(STREAM_TYPE *aAvgErr, STREAM_TYPE *bAvgErr, STREAM_TYPE *cAvgErr)
{
	STREAM_TYPE aj,bj,cj,scalar;
	STREAM_TYPE aSumErr,bSumErr,cSumErr;
	ssize_t	j;
	int	k;

    /* reproduce initialization */
	aj = 1.0;
	bj = 2.0;
	cj = 0.0;
    /* a[] is modified during timing check */
	aj = 2.0E0 * aj;
    /* now execute timing loop */
	scalar = SCALAR;
	for (k=0; k<NTIMES; k++)
        {
            cj = aj;
            bj = scalar*cj;
            cj = aj+bj;
            aj = bj+scalar*cj;
        }

    /* accumulate deltas between observed and expected results */
	aSumErr = 0.0;
	bSumErr = 0.0;
	cSumErr = 0.0;
	for (j=0; j<array_elements; j++) {
		aSumErr += abs(a[j] - aj);
		bSumErr += abs(b[j] - bj);
		cSumErr += abs(c[j] - cj);
	}
	*aAvgErr = aSumErr / (STREAM_TYPE) array_elements;
	*bAvgErr = bSumErr / (STREAM_TYPE) array_elements;
	*cAvgErr = cSumErr / (STREAM_TYPE) array_elements;
}



void checkSTREAMresults (STREAM_TYPE *AvgErrByRank, int numranks, double epsilon)
{
	STREAM_TYPE aj,bj,cj,scalar;
	STREAM_TYPE aSumErr,bSumErr,cSumErr;
	STREAM_TYPE aAvgErr,bAvgErr,cAvgErr;
	ssize_t	j;
	int	k,ierr;

	// Repeat the computation of aj, bj, cj because I am lazy
    /* reproduce initialization */
	aj = 1.0;
	bj = 2.0;
	cj = 0.0;
    /* a[] is modified during timing check */
	aj = 2.0E0 * aj;
    /* now execute timing loop */
	scalar = SCALAR;
	for (k=0; k<NTIMES; k++)
        {
            cj = aj;
            bj = scalar*cj;
            cj = aj+bj;
            aj = bj+scalar*cj;
        }

	// Compute the average of the average errors contributed by each MPI rank
	aSumErr = 0.0;
	bSumErr = 0.0;
	cSumErr = 0.0;
	for (k=0; k<numranks; k++) {
		aSumErr += AvgErrByRank[3*k + 0];
		bSumErr += AvgErrByRank[3*k + 1];
		cSumErr += AvgErrByRank[3*k + 2];
	}
	aAvgErr = aSumErr / (STREAM_TYPE) numranks;
	bAvgErr = bSumErr / (STREAM_TYPE) numranks;
	cAvgErr = cSumErr / (STREAM_TYPE) numranks;

	if (abs(aAvgErr/aj) > epsilon) {
		printf ("Failed Validation on array a[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
		printf ("     Expected Value: %e, AvgAbsErr: %e, AvgRelAbsErr: %e\n",aj,aAvgErr,abs(aAvgErr)/aj);
		ierr = 0;
		for (j=0; j<array_elements; j++) {
			if (abs(a[j]/aj-1.0) > epsilon) {
				ierr++;
#ifdef VERBOSE
				if (ierr < 10) {
					printf("         array a: index: %ld, expected: %e, observed: %e, relative error: %e\n",
						j,aj,a[j],abs((aj-a[j])/aAvgErr));
				}
#endif
			}
		}
		printf("     For array a[], %d errors were found.\n",ierr);
	}
	if (abs(bAvgErr/bj) > epsilon) {
		printf ("Failed Validation on array b[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
		printf ("     Expected Value: %e, AvgAbsErr: %e, AvgRelAbsErr: %e\n",bj,bAvgErr,abs(bAvgErr)/bj);
		printf ("     AvgRelAbsErr > Epsilon (%e)\n",epsilon);
		ierr = 0;
		for (j=0; j<array_elements; j++) {
			if (abs(b[j]/bj-1.0) > epsilon) {
				ierr++;
#ifdef VERBOSE
				if (ierr < 10) {
					printf("         array b: index: %ld, expected: %e, observed: %e, relative error: %e\n",
						j,bj,b[j],abs((bj-b[j])/bAvgErr));
				}
#endif
			}
		}
		printf("     For array b[], %d errors were found.\n",ierr);
	}
	if (abs(cAvgErr/cj) > epsilon) {
		printf ("Failed Validation on array c[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
		printf ("     Expected Value: %e, AvgAbsErr: %e, AvgRelAbsErr: %e\n",cj,cAvgErr,abs(cAvgErr)/cj);
		printf ("     AvgRelAbsErr > Epsilon (%e)\n",epsilon);
		ierr = 0;
		for (j=0; j<array_elements; j++) {
			if (abs(c[j]/cj-1.0) > epsilon) {
				ierr++;
#ifdef VERBOSE
				if (ierr < 10) {
					printf("         array c: index: %ld, expected: %e, observed: %e, relative error: %e\n",
						j,cj,c[j],abs((cj-c[j])/cAvgErr));
				}
#endif
			}
		}
		printf("     For array c[], %d errors were found.\n",ierr);
	}
#ifdef VERBOSE
	printf ("Results Validation Verbose Results: \n");
	printf ("    Expected a(1), b(1), c(1): %f %f %f \n",aj,bj,cj);
	printf ("    Observed a(1), b(1), c(1): %f %f %f \n",a[1],b[1],c[1]);
	printf ("    Rel Errors on a, b, c:     %e %e %e \n",abs(aAvgErr/aj),abs(bAvgErr/bj),abs(cAvgErr/cj));
#endif
}
