// Do not add pragma once here because we have to support re-enterable uthash

// [container] based on uthash
#include "uthash_hash.h"
#include "uthash_list.h"

// [math] based on fdlibm & musl
#include "fdlibm_copysign.c"
#include "fdlibm_fabs.c"
#include "fdlibm_log.c"
#include "fdlibm_log10.c"
#include "fdlibm_sqrt.c"
#include "fdlibm_scalbn.c"
#include "fdlibm_pow.c"
#include "musl_trunc.c"

// [mpi] based on hypre/utilities/mpistubs.c
#ifdef MYS_NO_MPI
#include "mpi_seq.c"
#else
#include "mpi_par.c"
#endif
