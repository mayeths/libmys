/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* void pointer is 8 bytes */
#define BIT64 1

/* Include stats on extra comparisons due to hash collisions */
/* #undef COLLIDE */

/* set debug ifdef */
/* #undef DEBUG */

/* nested omp capability enabled */
/* #undef ENABLE_NESTEDOMP */

/* enable MPI auto-profiling */
/* #undef ENABLE_PMPI */

/* Fortran name mangling uses double underscores e.g. g95 */
/* #undef FORTRANDOUBLEUNDERSCORE */

/* Use single underscore for Fortran wrappers: The usual case */
#define FORTRANUNDERSCORE 1

/* use configure for version info */
#define GPTL_VERSIONINFO "8.0.3"

/* backtrace will be used */
#define HAVE_BACKTRACE 1

/* Hopefully MPI_Comm_f2c is present */
#define HAVE_COMM_F2C 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `getrusage' function. */
#define HAVE_GETRUSAGE 1

/* gettimeofday() is available */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `m' library (-lm). */
#define HAVE_LIBM 1

/* Found MPI library */
/* #undef HAVE_LIBMPI */

/* Define to 1 if you have the `papi' library (-lpapi). */
/* #undef HAVE_LIBPAPI */

/* Define to 1 if you have the `pthread' library (-lpthread). */
/* #undef HAVE_LIBPTHREAD */

/* librt found */
/* #undef HAVE_LIBRT */

/* libunwind will be used */
/* #undef HAVE_LIBUNWIND */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* x86 nanotime capability is present */
/* #undef HAVE_NANOTIME */

/* PAPI library is present and usable */
/* #undef HAVE_PAPI */

/* /proc exists. Memory checking via /proc enabled */
/* #undef HAVE_SLASHPROC */

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* times() is available */
#define HAVE_TIMES 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Use \#include to inline threading routines into gptl.c */
#define INLINE_THREADING 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* size of status in MPI */
/* #undef MPI_STATUS_SIZE_IN_INTS */

/* Name of package */
#define PACKAGE "gptl"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "jmrosinski@gmail.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "GPTL"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "GPTL 8.0.3"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "gptl"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "8.0.3"

/* The size of `int', as computed by sizeof. */
/* #undef SIZEOF_INT */

/* The size of `MPI_Status', as computed by sizeof. */
/* #undef SIZEOF_MPI_STATUS */

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 8

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* openmp support is present */
// #define THREADED_OMP 1

/* use openmp for underlying threading */
// #define UNDERLYING_OPENMP 1

/* use pthreads library for underlying threading */
/* #undef UNDERLYING_PTHREADS */

/* Version number of package */
#define VERSION "8.0.3"

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif
