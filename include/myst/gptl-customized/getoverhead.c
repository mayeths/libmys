/*
** getoverhead.c
**
** Author: Jim Rosinski
** 
** Estimate overhead of various GPTL routines
*/

#include "config.h"    // Must be first include
#include "private.h"
#include "gptl_papi.h"

#ifdef HAVE_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#endif

#ifdef HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>  // for free()

// static bool initialized = true;
// static bool disabled = false;

// Local prototypes
static int gptlstart_sim (const char *, int);
static Timer *getentry_instr_sim (const Hashentry *,void *, unsigned int *, const int);
static void misc_sim (Nofalse *, Timer ***, int);

// All routines in this file are non-public

/*
** GPTLget_overhead: return current status info about a timer. If certain stats are not enabled, 
** they should just have zeros in them. If PAPI is not enabled, input counter info is ignored.
** 
** Input args:
**   fp:            File descriptor to write to
**   ptr2wtimefunc: Underlying timing routine
**   getentry:      From gptl.c, finds the entry in the hash table
**   genhashidx:    From gptl.c, generates the hash index
**   GPTLget_thread_num:    From thread*.c, gets the thread number
**   hashtable:     hashtable for thread 0
**   tablesize:     size of hashtable
**   dousepapi:     whether or not PAPI is enabled
**
** Output args:
**   self_ohd:      Estimate of GPTL-induced overhead in the timer itself (included in "Wallclock")
**   parent_ohd:    Estimate of GPTL-induced overhead for the timer which appears in its parents
*/
int GPTLget_overhead (FILE *fp,
		      double (*ptr2wtimefunc)(void), 
		      Timer *getentry (const Hashentry *, const char *, unsigned int),
		      unsigned int genhashidx (const char *),
		      int GPTLget_thread_num(void),
		      Nofalse *stackidx,
		      Timer ***callstack,
		      const Hashentry *hashtable, 
		      const int tablesize,
		      bool dousepapi,
		      int imperfect_nest,
		      double *self_ohd,
		      double *parent_ohd)
{
  double t1, t2;             // Initial, final timer values
  double ftn_ohd;            // Fortran-callable layer
  double get_thread_num_ohd; // Getting my thread index
  double genhashidx_ohd;     // Generating hash index
  double getentry_ohd;       // Finding entry in hash table
  double utr_ohd;            // Underlying timing routine
  double papi_ohd;           // Reading PAPI counters
  double total_ohd;          // Sum of overheads
  double getentry_instr_ohd; // Finding entry in hash table for auto-instrumented calls
  double addr2name_ohd;      // Invoking libunwind or backtrace routines from __cyg*enter
  double misc_ohd;           // misc. calcs within start/stop
  int i, n;
  // int ret;
  // int mythread;              // which thread are we
  unsigned int hashidx;      // Hash index
  int randomvar;             // placeholder for taking the address of a variable
  // Timer *entry;              // placeholder for return from "getentry()"
  static const char *thisfunc = "GPTLget_overhead";

  // Gather timings by running kernels 1000 times each. First: Fortran wrapper overhead
  t1 = (*ptr2wtimefunc)();
  for (i = 0; i < 1000; ++i) {
    // 9 is the number of characters in "timername"
    gptlstart_sim ("timername", 9);
  }
  t2 = (*ptr2wtimefunc)();
  ftn_ohd = 0.001 * (t2 - t1);

  // GPTLget_thread_num() overhead
  t1 = (*ptr2wtimefunc)();
  for (i = 0; i < 1000; ++i) {
    GPTLget_thread_num ();
  }
  t2 = (*ptr2wtimefunc)();
  get_thread_num_ohd = 0.001 * (t2 - t1);

  // genhashidx overhead
  t1 = (*ptr2wtimefunc)();
  for (i = 0; i < 1000; ++i) {
    hashidx = genhashidx ("timername");
  }
  t2 = (*ptr2wtimefunc)();
  genhashidx_ohd = 0.001 * (t2 - t1);

  // getentry overhead
  // Find the first hashtable entry with a valid name. Start at 1 because 0 is not a valid hash
  for (n = 1; n < tablesize; ++n) {
    if (hashtable[n].nument > 0 && strlen (hashtable[n].entries[0]->name) > 0) {
      hashidx = genhashidx (hashtable[n].entries[0]->name);
      t1 = (*ptr2wtimefunc)();
      for (i = 0; i < 1000; ++i)
	getentry (hashtable, hashtable[n].entries[0]->name, hashidx);
      t2 = (*ptr2wtimefunc)();
      fprintf (fp, "%s: using hash entry %d=%s for getentry estimate\n", 
	       thisfunc, n, hashtable[n].entries[0]->name);
      break;
    }
  }
  if (n == tablesize) {
    fprintf (fp, "%s: hash table empty: Using alternate means to find getentry time\n", thisfunc);
    t1 = (*ptr2wtimefunc)();
    for (i = 0; i < 1000; ++i)
      getentry (hashtable, "timername", hashidx);
    t2 = (*ptr2wtimefunc)();
  }
  getentry_ohd = 0.001 * (t2 - t1);

  // utr overhead
  t1 = (*ptr2wtimefunc)();
  for (i = 0; i < 1000; ++i) {
    t2 = (*ptr2wtimefunc)();
  }
  utr_ohd = 0.001 * (t2 - t1);

  // PAPI overhead
#ifdef HAVE_PAPI
  if (dousepapi) {
    t1 = (*ptr2wtimefunc)();
    GPTLread_counters1000 ();
    t2 = (*ptr2wtimefunc)();
  } else {
    t1 = 0.;
    t2 = 0.;
  }
  papi_ohd = 0.001 * (t2 - t1);
#else
  papi_ohd = 0.;
#endif

#ifdef HAVE_LIBUNWIND
  // libunwind overhead
  unw_cursor_t cursor;
  unw_context_t context;
  unw_word_t offset, pc;
  char symbol[MAX_SYMBOL_NAME+1];

  t1 = (*ptr2wtimefunc)();
  for (i = 0; i < 1000; ++i) {
    // Initialize cursor to current frame for local unwinding.
    unw_getcontext (&context);
    unw_init_local (&cursor, &context);

    (void) unw_step (&cursor);
    unw_get_reg (&cursor, UNW_REG_IP, &pc);
    (void) unw_get_proc_name (&cursor, symbol, sizeof(symbol), &offset);
  }
  t2 = (*ptr2wtimefunc)();
  addr2name_ohd = 0.001 * (t2 - t1);
#endif

#ifdef HAVE_BACKTRACE
  void *buffer[2];
  int nptrs;
  char **strings;

  t1 = (*ptr2wtimefunc)();
  for (i = 0; i < 1000; ++i) {
    nptrs = backtrace (buffer, 2);
    strings = backtrace_symbols (buffer, nptrs);
    free (strings);
  }
  t2 = (*ptr2wtimefunc)();
  addr2name_ohd = 0.001 * (t2 - t1);
#endif

  // getentry_instr overhead
  t1 = (*ptr2wtimefunc)();
  for (i = 0; i < 1000; ++i) {
    getentry_instr_sim (hashtable, &randomvar, &hashidx, tablesize);
  }
  t2 = (*ptr2wtimefunc)();
  getentry_instr_ohd = 0.001 * (t2 - t1);

  // misc start/stop overhead
  if (imperfect_nest) {
    fprintf (fp, "Imperfect nesting detected: setting misc_ohd=0\n");
    misc_ohd = 0.;
  } else {
    t1 = (*ptr2wtimefunc)();
    for (i = 0; i < 1000; ++i) {
      misc_sim (stackidx, callstack, 0);
    }
    t2 = (*ptr2wtimefunc)();
    misc_ohd = 0.001 * (t2 - t1);
  }

  total_ohd = ftn_ohd + get_thread_num_ohd + genhashidx_ohd + getentry_ohd + 
              utr_ohd + misc_ohd + papi_ohd;
  fprintf (fp, "Total overhead of 1 GPTL start or GPTLstop call=%g seconds\n", total_ohd);
  fprintf (fp, "Components are as follows:\n");
  fprintf (fp, "Fortran layer:             %7.1e = %5.1f%% of total\n", 
	  ftn_ohd, ftn_ohd / total_ohd * 100.);
  fprintf (fp, "Get thread number:         %7.1e = %5.1f%% of total\n", 
	  get_thread_num_ohd, get_thread_num_ohd / total_ohd * 100.);
  fprintf (fp, "Generate hash index:       %7.1e = %5.1f%% of total\n", 
	  genhashidx_ohd, genhashidx_ohd / total_ohd * 100.);
  fprintf (fp, "Find hashtable entry:      %7.1e = %5.1f%% of total\n", 
	  getentry_ohd, getentry_ohd / total_ohd * 100.);
  fprintf (fp, "Underlying timing routine: %7.1e = %5.1f%% of total\n", 
	  utr_ohd, utr_ohd / total_ohd * 100.);
  fprintf (fp, "Misc start/stop functions: %7.1e = %5.1f%% of total\n", 
	  misc_ohd, misc_ohd / total_ohd * 100.);
#ifdef HAVE_PAPI
  if (dousepapi) {
    fprintf (fp, "Read PAPI counters:        %7.1e = %5.1f%% of total\n", 
	    papi_ohd, papi_ohd / total_ohd * 100.);
  }
#endif
  fprintf (fp, "\n");
#ifdef HAVE_LIBUNWIND
  fprintf (fp, "Overhead of libunwind (invoked once per auto-instrumented start entry)=%g seconds\n", addr2name_ohd);
#elif defined HAVE_BACKTRACE
  fprintf (fp, "Overhead of backtrace (invoked once per auto-instrumented start entry)=%g seconds\n", addr2name_ohd);
#endif
  fprintf (fp, "NOTE: If GPTL is called from C not Fortran, the 'Fortran layer' overhead is zero\n");
  fprintf (fp, "NOTE: For calls to GPTLstart_handle()/GPTLstop_handle(), the 'Generate hash index' overhead is zero\n");
  fprintf (fp, "NOTE: For auto-instrumented calls, the cost of generating the hash index plus finding\n"
	  "      the hashtable entry is %7.1e not the %7.1e portion taken by GPTLstart\n", 
	  getentry_instr_ohd, genhashidx_ohd + getentry_ohd);
  fprintf (fp, "NOTE: Each hash collision roughly doubles the 'Find hashtable entry' cost of that timer\n");
  *self_ohd   = ftn_ohd + utr_ohd; // In GPTLstop() fortran wrapper is called before utr
  *parent_ohd = ftn_ohd + utr_ohd + misc_ohd +
                2.*(get_thread_num_ohd + genhashidx_ohd + getentry_ohd + papi_ohd);
  return 0;
}

/*
** GPTLstart_sim: Simulate the cost of Fortran wrapper layer "gptlstart()"
** 
** Input args:
**   name: timer name
**   nc:  number of characters in "name"
*/
static int gptlstart_sim (const char *name, int nc)
{
  char cname[nc+1];

  strncpy (cname, name, nc);
  cname[nc] = '\0';
  return 0;
}

/*
** getentry_instr_sim: Simulate the cost of getentry_instr(), which is invoked only when
** auto-instrumentation is enabled on non-AIX platforms
** 
** Input args:
**   hashtable: hashtable for thread 0
**   self:      address of function
**   indx:      hashtable index
**   tablesize: size of hashtable
*/
static Timer *getentry_instr_sim (const Hashentry *hashtable, void *self, unsigned int *indx,
				  const int tablesize)
{
  Timer *ptr = 0;
  *indx = (((unsigned long) self) >> 4) % tablesize;
  if (hashtable[*indx].nument > 0 && hashtable[*indx].entries[0]->address == self) {
    ptr = hashtable[*indx].entries[0];
  }
  return ptr;
}

/*
** misc_sim: Simulate the cost of miscellaneous computations in start/stop
** 
** Input args:
**   stackidx:  stack index
**   callstack: call stack
**   t:         thread index
*/
static void misc_sim (Nofalse *stackidx, Timer ***callstack, int t)
{
  int bidx;
  Timer *bptr;
  static Timer *ptr = 0;
  static const char *thisfunc = "misc_sim";

  if (disabled)
    printf ("GPTL: %s: should never print disabled\n", thisfunc);

  if (! initialized)
    printf ("GPTL: %s: should never print ! initialized\n", thisfunc);

  bidx = stackidx[t].val;
  bptr = callstack[t][bidx];
  if (ptr == bptr)
    printf ("GPTL: %s: should never print ptr=bptr\n", thisfunc);

  --stackidx[t].val;
  if (stackidx[t].val < -2)
    printf ("GPTL: %s: should never print stackidxt < -2\n", thisfunc);

  if (++stackidx[t].val > MAX_STACK-1)
    printf ("GPTL: %s: should never print stackidxt > MAX_STACK-1\n", thisfunc);

  return;
}
