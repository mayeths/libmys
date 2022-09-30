#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <mpi.h>
#include <math.h>

#define MYRANK() __mys_myrank()
#define NRANKS() __mys_nranks()
#define BARRIER() __mys_barrier()
#define PRINTF(who, fmt, ...) __mys_printf(who, fmt, ##__VA_ARGS__)


#define DEBUG(who, fmt, ...) __mys_debug(who, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define DEBUG_ORDERED(fmt, ...) do {   \
    int nranks = NRANKS();             \
    for (int i = 0; i < nranks; i++) { \
        DEBUG(i, fmt, ##__VA_ARGS__);  \
        BARRIER();                     \
    }                                  \
} while (0)


#define ASSERT(exp, fmt, ...) __mys_assert(exp, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#ifndef __ASSERT_SUGAR__
#define __ASSERT_SUGAR__
#define __ASSERTX_ZERO_OP__(exp, expect, actual, fmt, ...) do { \
    ASSERT(                                                     \
        (exp),                                                  \
        "Expect %s was %s but %s. " fmt,                        \
        #exp, expect, actual, ##__VA_ARGS__);                   \
} while (0)
#define __ASSERTX_ONE_OP__(exp1, op, exp2, spec, fmt, ...) do {        \
    ASSERT(                                                            \
        (exp1) op (exp2),                                              \
        "Expect (%s %s %s) but (" spec " %s " spec ") is false. " fmt, \
        #exp1, #op, #exp2,                                             \
        (exp1), #op, (exp2), ##__VA_ARGS__);                           \
} while (0)
#define __ASSERTX_TWO_OP__(exp1, op1, exp2, op2, exp3, spec, fmt, ...) do {              \
    ASSERT(                                                                              \
        ((exp1) op1 (exp2)) && ((exp2) op2 (exp3)),                                      \
        "Expect (%s %s %s %s %s) but (" spec " %s " spec " %s " spec ") is false. " fmt, \
        #exp1, #op1, #exp2, #op2, #exp3,                                                 \
        (exp1), #op1, (exp2), #op2, (exp3), ##__VA_ARGS__);                              \
} while (0)
#define ASSERT_TRUE(exp)                  __ASSERTX_ZERO_OP__(exp, "true", "false", "")
#define ASSERT_FALSE(exp)                 __ASSERTX_ZERO_OP__(exp, "false", "true", "")
#define ASSERT_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d","")
#define ASSERT_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d","")
#define ASSERT_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d","")
#define ASSERT_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d","")
#define ASSERT_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d","")
#define ASSERT_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d","")
#define ASSERT_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d","")
#define ASSERT_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d","")
#define ASSERTX_EQ(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,==,exp2,"%d",fmt,##__VA_ARGS__)
#define ASSERTX_NE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d",fmt,##__VA_ARGS__)
#define ASSERTX_LT(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d",fmt,##__VA_ARGS__)
#define ASSERTX_LE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d",fmt,##__VA_ARGS__)
#define ASSERTX_GT(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d",fmt,##__VA_ARGS__)
#define ASSERTX_GE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d",fmt,##__VA_ARGS__)
#define ASSERTX_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__)
#define ASSERTX_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__)
#define ASSERT_FLOAT_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f","")
#define ASSERT_FLOAT_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f","")
#define ASSERT_FLOAT_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f","")
#define ASSERT_FLOAT_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f","")
#define ASSERT_FLOAT_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f","")
#define ASSERT_FLOAT_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f","")
#define ASSERTX_FLOAT_EQ(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,==,exp2,"%f",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_NE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_LT(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_LE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_GT(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_GE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f",fmt,##__VA_ARGS__)
#define ASSERT_FLOAT_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f","")
#define ASSERT_FLOAT_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f","")
#endif /* __ASSERT_SUGAR__ */


#define CALL(fncall) do {        \
    int __call_result = fncall;  \
    ASSERT_EQ(__call_result, 0); \
} while (0)
#define FAILED(fmt, ...) __mys_failed(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define THROW_NOT_IMPL() FAILED("Not implemented.")
#define WAIT_FLAG(flagfile) __mys_wait_flag(__FILE__, __LINE__, flagfile)


/*********************************************/
/* Implement */
/*********************************************/


static inline void __mys_ensure_mpi_init()
{
    int inited;
    MPI_Initialized(&inited);
    if (inited) return;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_SINGLE, &inited);
    fprintf(stdout, ">>>>> ===================================== <<<<<\n");
    fprintf(stdout, ">>>>> Nevel let libmys init MPI you dumbass <<<<<\n");
    fprintf(stdout, ">>>>> ===================================== <<<<<\n");
    fflush(stdout);
}

static inline int __mys_myrank()
{
    __mys_ensure_mpi_init();
    int myrank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    return myrank;
}
static inline int __mys_nranks()
{
    __mys_ensure_mpi_init();
    int nranks = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);
    return nranks;
}
static inline void __mys_barrier()
{
    __mys_ensure_mpi_init();
    MPI_Barrier(MPI_COMM_WORLD);
}

__attribute__((format(printf, 2, 3)))
static inline void __mys_printf(int who, const char *fmt, ...)
{
    int myrank = __mys_myrank();
    #pragma omp critical (mys)
    if ((myrank) == (who)) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fflush(stdout);
    }
}

__attribute__((format(printf, 4, 5)))
static inline void __mys_debug(int who, const char *file, int line, const char *fmt, ...)
{
    int myrank = __mys_myrank();
    #pragma omp critical (mys)
    if ((myrank) == (who)) {
        int nranks = __mys_nranks();
        int nprefix = trunc(log10(nranks)) + 1;
        nprefix = nprefix > 3 ? nprefix : 3;
        fprintf(stdout, "[DEBUG::%0*d %s:%03d] ", nprefix, myrank, file, line);
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

__attribute__((format(printf, 4, 5)))
static inline void __mys_assert(int exp, const char *file, int line, const char *fmt, ...)
{
    #pragma omp critical (mys)
    if (!(exp)) {
        int myrank = __mys_myrank();
        int nranks = __mys_nranks();
        int nprefix = trunc(log10(nranks)) + 1;
        nprefix = nprefix > 3 ? nprefix : 3;
        fprintf(stdout, "[ASSERT::%0*d %s:%03d] ", nprefix, myrank, file, line);
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

__attribute__((format(printf, 3, 4)))
static inline void __mys_failed(const char *file, int line, const char *fmt, ...)
{
    #pragma omp critical (mys)
    {
        int myrank = __mys_myrank();
        int nranks = __mys_nranks();
        int nprefix = trunc(log10(nranks)) + 1;
        nprefix = nprefix > 3 ? nprefix : 3;
        fprintf(stdout, "[FAILED::%0*d %s:%03d] ", nprefix, myrank, file, line);
        va_list args;
        va_start(args, fmt);
        vfprintf(stdout, fmt, args);
        va_end(args);
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    exit(1);
}

#include <unistd.h>
#include <sys/stat.h>
static inline void __mys_wait_flag(const char *file, int line, const char *flagfile) {
    int myrank = __mys_myrank();
    #pragma omp critical (mys)
    {
        int nranks = __mys_nranks();
        int nprefix = trunc(log10(nranks)) + 1;
        nprefix = nprefix > 3 ? nprefix : 3;
        if (myrank == 0) {
            fprintf(stdout, "[WAIT::%0*d %s:%03d] Use \"touch %s\" to continue... ", nprefix, 0, file, line, flagfile);
            fflush(stdout);
        }
    }
    struct stat fstat;
    time_t mod_time;
    if (stat(flagfile, &fstat) == 0) {
        mod_time = fstat.st_mtime;
    } else {
        mod_time = 0;
    }
    while (1) {
        if (stat(flagfile, &fstat) == 0) {
            if (mod_time < fstat.st_mtime) break;
        }
        sleep(1);
    }
    __mys_barrier();
    #pragma omp critical (mys)
    if (myrank == 0) {
        fprintf(stdout, "OK\n");
        fflush(stdout);
    }
}
