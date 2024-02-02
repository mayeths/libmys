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
#include "stream.h"

static const char *usage =
    HLINE
    "STREAM version 5.10 (last modified by Mayeths on %s)\n"
    HLINE
    "Usage\n"
    "    mpirun -n nproc  %s ncores min_size\n"
    "    * Use the size of LLC (LLC_size) as min_size if it's larger than 32MB.\n"
    "    [Kunpeng920] ./stream 128 48MB (for 128 cores and 48MB shared L3)\n"
    "    [Apple M1]   ./stream 8  128MB (for 8 cores and 16MB shared L2)\n"
    HLINE
    "Size Algorithm (by Mayeths)\n"
    "    local_array_size = min_size * (log2(ncores)+1) / (log2(nranks)+1)\n"
    "    - local_array_size is min_size * log2(ncores) when utilizing one core.\n"
    "    - local_array_size is min_size when utilizing all cores.\n"
    "    - global_array_size ranges from [min_size * (log2(ncores)+1), min_size * ncores]\n"
    HLINE
    "Note (from original STREAM)\n"
    "    - local_array_size must be large enough that the traversal cost > 20 clock-ticks.\n"
    "    - Ensure global_array_size >= 4 * LLC_size. So min_size > 4 * LLC_size if ncores <= 8.\n"
    HLINE
	"STREAM early exit. (reason: %s)\n"
    HLINE
;

size_t array_elements;
size_t array_bytes;
size_t array_alignment;
STREAM_TYPE * restrict a;
STREAM_TYPE * restrict b;
STREAM_TYPE * restrict c;

static char *label[4]    = {"Copy", "Scale", "Add", "Triad"};
static double avgtime[4] = {0, 0, 0, 0};
static double maxtime[4] = {0, 0, 0, 0};
static double mintime[4] = {FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX};

static int check_results(STREAM_TYPE *AvgErrByRank, int numranks, double epsilon);
static void compute_errors(STREAM_TYPE *aAvgErr, STREAM_TYPE *bAvgErr, STREAM_TYPE *cAvgErr);
static double check_timer_granularity();
static double reset_arrays(STREAM_TYPE *a, STREAM_TYPE *b, STREAM_TYPE *c, size_t narr);
static size_t naive_strnlen(const char *str, size_t max);
static void exit_with_usage(FILE *fd, const char *prog, const char *reason, ...);

static inline int64_t str_to_i64(const char *str, int64_t default_val);
static inline int32_t str_to_i32(const char *str, int32_t default_val);
static void to_readable_size(char **ptr, size_t bytes, size_t precision);
static ssize_t from_readable_size(const char *text);

int main(int argc, char **argv)
    {
    double		quantum = 0;
    int			i,k;
    ssize_t		j;
	size_t passed = 0;
    double		times[4][NTIMES];
	double		*TimesByRank = NULL;
	double		t0,t1,tmin;
	STREAM_TYPE	AvgError[3] = {0.0,0.0,0.0};
	STREAM_TYPE *AvgErrByRank = NULL;
	double *MinTimesByRank = NULL;

    /* --- SETUP --- call MPI_Init() before anything else! --- */
    if (MPI_Init(NULL, NULL) != MPI_SUCCESS) {
       printf("[ERROR] MPI Initialization failed\n");
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
		exit_with_usage(stderr, argv[0], "no arguments provided", NULL);

	int call_help = 0;
	call_help |= strncmp(argv[1], "-h", sizeof("-h")) == 0;
	call_help |= strncmp(argv[1], "--help", sizeof("--help")) == 0;
	if (call_help) {
		exit_with_usage(stderr, argv[0], "print help message", NULL);
	}

	ssize_t min_size = from_readable_size(argv[2]);
	if (min_size == -1) {
		exit_with_usage(stderr, argv[0], "invalid min_size \"%s\"", argv[2]);
	}

	// Use log2() to prevent numranks=1 got too large array
	int ncores = str_to_i32(argv[1], -1);
	if (ncores == -1) {
		exit_with_usage(stderr, argv[0], "invalid ncores \"%s\"", argv[1]);
	}
	size_t local_array_size = (size_t)((double)min_size * (log2(ncores) + 1) / (log2(numranks) + 1));
	// size_t minsize_threshold = from_readable_size(MINSIZE_THRESHOLD);
	// size_t local_array_size = 0;
	// t = estimate(myrank, numranks, MINTIME_THRESHOLD, minsize_threshold, min_size, &local_array_size);

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
		printf("[WARNING] sizeof(STREAM_TYPE) = %lu\n",sizeof(STREAM_TYPE));
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
        printf("[ERROR] Rank %d: Allocation of array a failed, return code is %d\n",myrank,k);
		MPI_Abort(MPI_COMM_WORLD, 2);
        exit(1);
    }
    k = posix_memalign((void **)&b, array_alignment, array_bytes);
    if (k != 0) {
        printf("[ERROR] Rank %d: Allocation of array b failed, return code is %d\n",myrank,k);
		MPI_Abort(MPI_COMM_WORLD, 2);
        exit(1);
    }
    k = posix_memalign((void **)&c, array_alignment, array_bytes);
    if (k != 0) {
        printf("[ERROR] Rank %d: Allocation of array c failed, return code is %d\n",myrank,k);
		MPI_Abort(MPI_COMM_WORLD, 2);
        exit(1);
    }
	memset(a, 0, array_bytes);
	memset(b, 0, array_bytes);
	memset(c, 0, array_bytes);

	quantum = check_timer_granularity();

    /* --- SETUP --- initialize arrays and estimate precision of timer --- */

	// Initial informational printouts -- rank 0 handles all the output
	if (myrank == 0) {
		printf(HLINE);
		printf("STREAM version 5.10 (last modified by Mayeths on %s)\n", MYS_MODIFIED_ON);

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

		printf("Repeat: %d (report the best run excluding the first one)\n", NTIMES);
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
		printf("Each rank uses 3 arrays to perform COPY, SCALE, ADD and TRIAD.\n");
		printf("Local array length: %zu (%s, %s per proc)\n", array_elements, s_local_array_size, s_local_rank_size);
		printf("Dist array length: %zu (%s, %s total allocated)\n", STREAM_ARRAY_SIZE, s_global_array_size, s_global_rank_size);
		free(s_local_array_size);
		free(s_local_rank_size);
		free(s_global_array_size);
		free(s_global_rank_size);
		printf("Performing %zu test:\n", ntest);
		for (size_t itest = 0; itest < ntest; itest++) {
			const testsuite_t *test = &testsuites[itest];
			printf("-  %s", test->name);
			const char *desc = test->d();
			if (desc != NULL)
				printf(" (%s)\n", desc);
			else
				printf("\n");
		}
		printf(HLINE);
	}

	// Rank 0 needs to allocate arrays to hold error data and timing data from
	// all ranks for analysis and output.
	// Allocate and instantiate the arrays here -- after the primary arrays 
	// have been instantiated -- so there is no possibility of having these 
	// auxiliary arrays mess up the NUMA placement of the primary arrays.

	if (myrank == 0) {
		// There are 3 average error values for each rank (using STREAM_TYPE).
		AvgErrByRank = (double *)malloc(3 * sizeof(double) * numranks);
		TimesByRank = (double *)malloc(4 * NTIMES * sizeof(double) * numranks);
		MinTimesByRank = (double *)malloc(sizeof(double) * 4 * numranks);
		if (AvgErrByRank == NULL) {
			printf("Ooops -- allocation of arrays to collect errors on MPI rank 0 failed\n");
			MPI_Abort(MPI_COMM_WORLD, 2);
		}
		if (TimesByRank == NULL) {
			printf("Ooops -- allocation of arrays to collect timing data on MPI rank 0 failed\n");
			MPI_Abort(MPI_COMM_WORLD, 3);
		}
		if (MinTimesByRank == NULL) {
			printf("Ooops -- allocation of arrays to collect timing data on MPI rank 0 failed\n");
			MPI_Abort(MPI_COMM_WORLD, 3);
		}
		memset(AvgErrByRank,0,3*sizeof(double)*numranks);
		memset(TimesByRank,0,4*NTIMES*sizeof(double)*numranks);
		memset(MinTimesByRank,0,sizeof(double) * 4 * numranks);
	}

    /*	--- MAIN LOOP --- repeat test cases NTIMES times --- */

    // This code has more barriers and timing calls than are actually needed, but
    // this should not cause a problem for arrays that are large enough to satisfy
    // the STREAM run rules.
	// MAJOR FIX!!!  Version 1.7 had the start timer for each loop *after* the
	// MPI_Barrier(), when it should have been *before* the MPI_Barrier().
    // 

	for (size_t itest = 0; itest < ntest; itest++) {
		const testsuite_t *test = &testsuites[itest];
		void *metadata = NULL;

		reset_arrays(a, b, c, array_elements);
		test->i(MPI_COMM_WORLD, array_elements, SCALAR, &metadata);

#define TICK(t) do { MPI_Barrier(MPI_COMM_WORLD); t = MPI_Wtime(); } while (0)
#define TOCK(t) do { t = MPI_Wtime(); MPI_Barrier(MPI_COMM_WORLD); } while (0)

		for (k=0; k<NTIMES; k++) {
			// Running kernels: Copy, Scale, Add, Triad
			// Propagate possible errors by using output of last kernel as input of next one.
			TICK(t0); test->c(metadata, c, a);    TOCK(t1); times[0][k] = t1 - t0;
			TICK(t0); test->s(metadata, b, c);    TOCK(t1); times[1][k] = t1 - t0;
			TICK(t0); test->a(metadata, c, a, b); TOCK(t1); times[2][k] = t1 - t0;
			TICK(t0); test->t(metadata, a, c, b); TOCK(t1); times[3][k] = t1 - t0;
		}

#undef TICK
#undef TOCK

		test->f(&metadata);

		/*	--- SUMMARY --- */

		// Because of the MPI_Barrier() calls, the timings from any thread are equally valid. 
		// The best estimate of the maximum performance is the minimum of the "outside the barrier"
		// timings across all the MPI ranks.

		// Gather all timing data to MPI rank 0
		MPI_Gather(times, 4*NTIMES, MPI_DOUBLE, TimesByRank, 4*NTIMES, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		// Rank 0 processes all timing data
		if (myrank == 0) {
			for (i = 0; i < 4 * numranks; i++)
				MinTimesByRank[i] = FLT_MAX;
			// for each iteration and each kernel, collect the minimum time across all MPI ranks
			// and overwrite the rank 0 "times" variable with the minimum so the original post-
			// processing code can still be used.
			for (k=0; k<NTIMES; k++) {
				for (j=0; j<4; j++) {
					tmin = 1.0e36;
					for (i=0; i<numranks; i++) {
						double t = TimesByRank[4*NTIMES*i+j*NTIMES+k];
						MinTimesByRank[4 * i + j] = MIN(MinTimesByRank[4 * i + j], t);
						tmin = MIN(tmin, t);
					}
					// printf("DEBUG: Final Timing: iter %d, kernel %lu, final tmin %f\n",k,j,tmin);
					times[j][k] = tmin;
				}
			}

			// Back to the original code, but now using the minimum global timing across all ranks
			for (int j = 0; j < 4; j++) {
				mintime[j] = FLT_MAX;
				avgtime[j] = FLT_MIN;
				maxtime[j] = FLT_MIN;
			}
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
			printf("%-13.13s Best MB/s     Avg time     Min time     Max time"
				"   Rank 0 MB/s  Rank best MB/s  Rank worst MB/s\n", test->name);
			for (j=0; j<4; j++) {
				avgtime[j] = avgtime[j]/(double)(NTIMES-1);
				int len = naive_strnlen(label[j], 10);
				printf("%s:%*c %11.1f  %11.6f  %11.6f  %11.6f  %12.1f  %14.1f  %15.1f\n"
					, label[j], 10-len, ' ', 1.0E-06 * bytes[j]/mintime[j]
					, avgtime[j], mintime[j], maxtime[j]
					, 1.0E-06 * (bytes[j] / numranks) / MinTimesByRank[4 * 0 + j]
					, 1.0E-06 * (bytes[j] / numranks) / mintime[j]
					, 1.0E-06 * (bytes[j] / numranks) / maxtime[j]
				);
			}
			// printf(HLINE);
		}

		/* --- Every Rank Checks its Results --- */
#ifdef INJECTERROR
	a[11] = 100.0 * a[11];
#endif
		compute_errors(&AvgError[0], &AvgError[1], &AvgError[2]);
		/* --- Collect the Average Errors for Each Array on Rank 0 --- */
		MPI_Gather(AvgError, 3, MPI_DOUBLE, AvgErrByRank, 3, MPI_DOUBLE, 0, MPI_COMM_WORLD);

		////// print WARNING or ERROR if needed
		if (myrank == 0) {
			// int bad = 0;
			if (quantum >= (double)TIMER_THRESHOLD) {
				if (TIMER_THRESHOLD < 1e-6)
					printf("[WARNING] inappropriate timer granularity (%.0f ns > %.0f ns)\n",
						quantum * 1e9, TIMER_THRESHOLD * 1e9);
				else if (TIMER_THRESHOLD < 1e-3)
					printf("[WARNING] inappropriate timer granularity (%.0f us > %.0f us)\n",
						quantum * 1e6, TIMER_THRESHOLD * 1e6);
				else if (TIMER_THRESHOLD < 1)
					printf("[WARNING] inappropriate timer granularity (%.0f ms > %.0f ms)\n",
						quantum * 1e3, TIMER_THRESHOLD * 1e3);
				else
					printf("[WARNING] inappropriate timer granularity (> %.2f sec)\n", TIMER_THRESHOLD);
				// bad = 1;
			}

			for (j = 0; j < 4; j++) {
				if (mintime[j] < MINTIME_THRESHOLD) {
					// We don't say anything about whether it's accurate. just print warning.
					printf("[WARNING] %s is too fast (%.1f ms < %.1f ms)\n",
						label[j], mintime[j] * 1e3, MINTIME_THRESHOLD * 1e3
					);
					// bad = 2;
				}
			}

			for (i = 0; i < numranks; i++) {
				for (j = 0; j < 4; j++) {
					double rel = fabs(MinTimesByRank[4 * i + j] - mintime[j]) / mintime[j];
					if (rel >= IMBALANCE_THRESHOLD)
						break;
				}
				if (j != 4)
					break;
			}
			if (i != numranks) {
				printf("[WARNING] significant variance of min time (> %.0f%%)\n", IMBALANCE_THRESHOLD * 100);
				printf("              ");
				for (j = 0; j < 4; j++)
					printf(" %11s", label[j]);
				printf("\n      Min time");
				for (j = 0; j < 4; j++)
					printf(" %11.6f", mintime[0]);
				printf("\n Rank variance -----------------------------------------------\n");
				int digits = trunc(log10(numranks)) + 1;
				for (i = 0; i < numranks; i++) {
					printf("%*c%*d", 14-digits, ' ', digits, i);
					for (j = 0; j < 4; j++) {
						double rel = (MinTimesByRank[4 * i + j] - mintime[j]) / mintime[j];
						char mark = fabs(rel) >= IMBALANCE_THRESHOLD ? 'x' /*: rel == 0 ? 'o'*/ : ' ';
						int digits = ceil(log10(rel * 100));
						digits = digits < 1 ? 1 : digits;
						printf("     %c %+3.*f%%", mark, 3-digits, rel * 100);
					}
					printf("\n");
				}
				// bad = 3;
			}

			// if (bad != 0)
			// 	printf(HLINE);
		}

		/* -- Combined averaged errors and report on Rank 0 only --- */
		if (myrank == 0) {
			if (check_results(AvgErrByRank,numranks, epsilon) == 0)
				passed += 1;
			printf(HLINE);
		}

	}

	if (myrank == 0) {
		printf("TOTAL %zu tests, %zu PASSED, %zu FAILED.\n", ntest, passed, ntest - passed);
		printf(HLINE);
	}

	free(a);
	free(b);
	free(c);
	if (myrank == 0) {
		free(TimesByRank);
		free(MinTimesByRank);
		free(AvgErrByRank);
	}

    MPI_Finalize();
	return(0);
}

#define M 32
static double check_timer_granularity() {
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
static void compute_errors(STREAM_TYPE *aAvgErr, STREAM_TYPE *bAvgErr, STREAM_TYPE *cAvgErr)
{
	STREAM_TYPE aj,bj,cj,scalar;
	STREAM_TYPE aSumErr,bSumErr,cSumErr;
	size_t	j;
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



static int check_results (STREAM_TYPE *AvgErrByRank, int numranks, double epsilon)
{
	STREAM_TYPE aj,bj,cj,scalar;
	STREAM_TYPE aSumErr,bSumErr,cSumErr;
	STREAM_TYPE aAvgErr,bAvgErr,cAvgErr;
	size_t	j;
	int	k,ierr = 0;

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
		printf ("[ERROR] Failed Validation on array a[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
		printf ("     Expected Value: %e, AvgAbsErr: %e, AvgRelAbsErr: %e\n",aj,aAvgErr,abs(aAvgErr)/aj);
		ierr = 0;
		for (j=0; j<array_elements; j++) {
			if (abs(a[j]/aj-1.0) > epsilon) {
				ierr++;
#ifdef VERBOSE
				if (ierr < 10) {
					printf("         array a: index: %zu, expected: %e, observed: %e, relative error: %e\n",
						j,aj,a[j],abs((aj-a[j])/aAvgErr));
				}
#endif
			}
		}
		printf("     For array a[], %d errors were found.\n",ierr);
	}
	if (abs(bAvgErr/bj) > epsilon) {
		printf ("[ERROR] Failed Validation on array b[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
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
		printf ("[ERROR] Failed Validation on array c[], AvgRelAbsErr > epsilon (%e)\n",epsilon);
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
	return ierr;
}

double hrtime() {
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t t = (uint64_t)ts.tv_sec * (uint64_t)1000000000 + (uint64_t)ts.tv_nsec;
    return (double)t / (double)1000000000;
#else
    struct timeval ts;
    gettimeofday(&ts, NULL);
    uint64_t t = (uint64_t)t.tv_sec * (uint64_t)1000000 + (uint64_t)t.tv_usec;
    return (double)t / (double)1000000;
#endif
}

// Use this if strnlen is missing.
static size_t naive_strnlen(const char *str, size_t max)
{
    const char *end = memchr(str, 0, max);
    return end ? (size_t)(end - str) : max;
}

static double reset_arrays(STREAM_TYPE *a, STREAM_TYPE *b, STREAM_TYPE *c, size_t narr)
{
	double t1 = MPI_Wtime();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (size_t j = 0; j < narr; j++) {
		a[j] = 2.0;
		b[j] = 2.0;
		c[j] = 0.0;
	}
	double t2 = MPI_Wtime();
	return t2 - t1;
}


static void to_readable_size(char **ptr, size_t bytes, size_t precision)
{
    int i = 0;
    const char* units[] = {"Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double size = bytes;
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    int len = snprintf(NULL, 0, "%.*f %s", (int)precision, size, units[i]) + 1; /*%.*f*/
    *ptr = (char *)malloc(sizeof(char) * len);
    snprintf(*ptr, len, "%.*f %s", (int)precision, size, units[i]);
}

static ssize_t from_readable_size(const char *text) {
    typedef struct {
        const char *suffix;
        size_t base;
    } unit_t;
    static const size_t Bbase = 1ULL;
    static const size_t Kbase = 1024ULL * Bbase;
    static const size_t Mbase = 1024ULL * Kbase;
    static const size_t Gbase = 1024ULL * Mbase;
    static const size_t Tbase = 1024ULL * Gbase;
    static const size_t Pbase = 1024ULL * Tbase;
    static const size_t Ebase = 1024ULL * Pbase;
    static const size_t Zbase = 1024ULL * Ebase;
    unit_t units[] = {
        { .suffix = "Bytes",  .base = Bbase },
        { .suffix = "Byte",   .base = Bbase },
        { .suffix = "B",      .base = Bbase },
        { .suffix = "KBytes", .base = Kbase },
        { .suffix = "KB",     .base = Kbase },
        { .suffix = "K",      .base = Kbase },
        { .suffix = "MBytes", .base = Mbase },
        { .suffix = "MB",     .base = Mbase },
        { .suffix = "M",      .base = Mbase },
        { .suffix = "GBytes", .base = Gbase },
        { .suffix = "GB",     .base = Gbase },
        { .suffix = "G",      .base = Gbase },
        { .suffix = "TBytes", .base = Tbase },
        { .suffix = "TB",     .base = Tbase },
        { .suffix = "T",      .base = Tbase },
        { .suffix = "PBytes", .base = Pbase },
        { .suffix = "PB",     .base = Pbase },
        { .suffix = "P",      .base = Pbase },
        { .suffix = "EBytes", .base = Ebase },
        { .suffix = "EB",     .base = Ebase },
        { .suffix = "E",      .base = Ebase },
        { .suffix = "ZBytes", .base = Zbase },
        { .suffix = "ZB",     .base = Zbase },
        { .suffix = "Z",      .base = Zbase },
    };

    char *endptr = NULL;
    errno = 0;
    double dnum = strtod(text, &endptr);
    int error = errno;
    errno = 0;

    if (endptr == text)
        return -1; /* contains with non-number */
    if (error == ERANGE)
        return -1; /* number out of range for double */
    if (dnum != dnum)
        return -1; /* not a number */

    ssize_t num = (ssize_t)dnum;

    while (*endptr == ' ')
        endptr++;
    if (*endptr == '\0')
        return (ssize_t)dnum; /* no suffix */

    for (size_t i = 0; i < sizeof(units) / sizeof(unit_t); i++) {
        unit_t unit = units[i];
        int matched = strncmp(endptr, unit.suffix, 32) == 0;
        if (matched)
            return num * unit.base;
    }

    return -1;
}

static inline int64_t str_to_i64(const char *str, int64_t default_val)
{
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    int64_t num = strtoll(str, &stop, 10);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if ((num == LLONG_MAX || num == LLONG_MIN) && error == ERANGE)
        return default_val; /* number out of range for LONG */
    return num;
}

static inline int32_t str_to_i32(const char *str, int32_t default_val)
{
    int64_t num = str_to_i64(str, (int64_t)default_val);
    if ((num < INT_MIN) || (num > INT_MAX))
        return default_val; /* number out of range for INT */
    return (int32_t)num;
}

static void exit_with_usage(FILE *fd, const char *prog, const char *reason, ...) {
	int myrank = -1;
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	if (myrank == 0) {
		char reason_str[4096] = "unknown";
		if (reason != NULL) {
			va_list opt;
			va_start(opt, reason);
			vsnprintf(reason_str, sizeof(reason_str), reason, opt);
			va_end(opt);
		}

		int len = snprintf(NULL, 0, usage, MYS_MODIFIED_ON, prog, reason_str) + 1;
		char *buffer = (char *)malloc(sizeof(char) * len);
		snprintf(buffer, len, usage, MYS_MODIFIED_ON, prog, reason_str);
		fprintf(fd, "%s", buffer);
		fflush(fd);
		free(buffer);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	exit(0);
}
