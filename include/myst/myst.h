#pragma once
// #include "./gptl-customized/gptl.h"

/*
** Options settable by a call to MYSTsetoption() (default in parens)
** These numbers need to be small integers because MYSTsetoption can
** be passed PAPI counters, and we need to avoid collisions in that
** integer space. PAPI presets are big negative integers, and PAPI
** native events are big positive integers.
*/

typedef enum {
  MYSTsync_mpi        = 0,  // Synchronize before certain MPI calls (PMPI-mode only)
  MYSTwall            = 1,  // Collect wallclock stats (true)
  MYSTcpu             = 2,  // Collect CPU stats (false)*/
  MYSTabort_on_error  = 3,  // Abort on failure (false)
  MYSToverhead        = 4,  // Estimate overhead of underlying timing routine (true)
  MYSTdepthlimit      = 5,  // Only print timers this depth or less in the tree (inf)
  MYSTverbose         = 6,  // Verbose output (false)
  MYSTnarrowprint     = 7,  // Print PAPI and derived stats in 8 columns not 16 (true)
  MYSTpercent         = 9,  // Add a column for percent of first timer (false)
  MYSTpersec          = 10, // Add a PAPI column that prints "per second" stats (true)
  MYSTmultiplex       = 11, // Allow PAPI multiplexing (false)
  MYSTdopr_preamble   = 12, // Print preamble info (true)
  MYSTdopr_threadsort = 13, // Print sorted thread stats (true)
  MYSTdopr_multparent = 14, // Print multiple parent info (true)
  MYSTdopr_collision  = 15, // Print hastable collision info (true)
  MYSTdopr_memusage   = 27, // Print memory usage stats when growth exceeds some threshhold %
  MYSTprint_method    = 16, // Tree print method: first parent, last parent,
			    //   most frequent, or full tree (most frequent)
  MYSTtablesize       = 50, // per-thread size of hash table
  MYSTmaxthreads      = 51, // maximum number of threads
  MYSTonlyprint_rank0 = 52, // Restrict printout to rank 0 when MPI enabled
  MYSTmem_growth      = 53, // Print info when mem usage (RSS) has grown by more than some percent

  // These are derived counters based on PAPI counters. All default to false
  MYST_IPC           = 17, // Instructions per cycle
  MYST_LSTPI         = 21, // Load-store instruction fraction
  MYST_DCMRT         = 22, // L1 miss rate (fraction)
  MYST_LSTPDCM       = 23, // Load-stores per L1 miss
  MYST_L2MRT         = 24, // L2 miss rate (fraction)
  MYST_LSTPL2M       = 25, // Load-stores per L2 miss
  MYST_L3MRT         = 26  // L3 read miss rate (fraction)
} MYSToption;

/*
** Underlying wallclock timer: optimize for best granularity with least overhead.
** These numbers need not be distinct from the above because these are passed
** to MYSTsetutr() and the above are passed to MYSTsetoption()
*/
typedef enum {
  MYSTgettimeofday   = 1, // ubiquitous but slow
  MYSTnanotime       = 2, // only available on x86
  MYSTmpiwtime       = 4, // MPI_Wtime
  MYSTclockgettime   = 5, // clock_gettime
  MYSTplacebo        = 7, // do-nothing
  MYSTread_real_time = 3  // AIX only
} MYSTFuncoption;

// How to report parent/child relationships at print time (for children with multiple parents)
typedef enum {
  MYSTfirst_parent  = 1,  // first parent found
  MYSTlast_parent   = 2,  // last parent found
  MYSTmost_frequent = 3,  // most frequent parent (default)
  MYSTfull_tree     = 4   // complete call tree
} MYSTMethod;

// User-callable function prototypes: all require C linkage
#ifdef __cplusplus
extern "C" {
#endif

extern int MYSTsetoption (const int, const int);
extern int MYSTinitialize (void);
extern int MYSTstart (const char *);
extern int MYSTinit_handle (const char *, int *);
extern int MYSTstart_handle (const char *, int *);
extern int MYSTstop (const char *);
extern int MYSTstop_handle (const char *, int *);
extern int MYSTstamp (double *, double *, double *);
extern int MYSTpr (const int);
extern int MYSTpr_file (const char *);
extern int MYSTreset (void);
extern int MYSTreset_timer (const char *);
extern int MYSTfinalize (void);
extern int MYSTget_memusage (float *);
extern int MYSTprint_memusage (const char *);
extern int MYSTprint_rusage (const char *);
extern int MYSTget_procsiz (float *, float *);
extern int MYSTenable (void);
extern int MYSTdisable (void);
extern int MYSTsetutr (const int);
extern int MYSTquery (const char *, int, int *, int *, double *, double *, double *,
		      long long *, const int);
extern int MYSTget_wallclock (const char *, int, double *);
extern int MYSTget_wallclock_latest (const char *, int, double *);
extern int MYSTget_threadwork (const char *, double *, double *);
extern int MYSTstartstop_val (const char *, double);
extern int MYSTget_nregions (int, int *);
extern int MYSTget_regionname (int, int, char *, int);
extern int MYST_PAPIlibraryinit (void);
extern int MYSTevent_name_to_code (const char *, int *);
extern int MYSTevent_code_to_name (const int, char *);
extern int MYSTget_eventvalue (const char *, const char *, int, double *);
extern int MYSTnum_errors (void);
extern int MYSTnum_warn (void);
extern int MYSTget_count (const char *, int, int *);

__attribute__((format(printf, 1, 2)))
extern int MYSTtick(const char *, ...);
__attribute__((format(printf, 1, 2)))
extern int MYSTtock(const char *, ...);

#ifdef MYS_IMPL
#include "myst.impl.h"
#endif

#ifdef __cplusplus
}
#endif
