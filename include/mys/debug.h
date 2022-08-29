#pragma once

#include <stdio.h>
#include <mpi.h>

#define ENSURE_MPI_INIT() do {               \
    int i;                                   \
    MPI_Initialized(&i);                     \
    if (!i) {                                \
        MPI_Init_thread(NULL,NULL,           \
        MPI_THREAD_SINGLE,&i);               \
        fprintf(stdout,                      \
            ">>>>>--- Nevel let libmys init" \
            " MPI you dumbass ---<<<<<\n");  \
        fflush(stdout);                      \
    }                                        \
}while(0)
static inline int MYRANK()
{
    ENSURE_MPI_INIT();
    int myrank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    return myrank;
}
static inline int NRANKS()
{
    ENSURE_MPI_INIT();
    int nranks = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);
    return nranks;
}
static inline void BARRIER()
{
    ENSURE_MPI_INIT();
    MPI_Barrier(MPI_COMM_WORLD);
}

#ifdef DEBUG
#warning DEBUG was predefined, skipped definition from libmys
#else
#define DEBUG(who, fmt, ...) do {                          \
    int nranks = NRANKS(), myrank = MYRANK();              \
    int nprefix = trunc(log10(nranks)) + 1;                \
    _Pragma("omp critical (mys)")                          \
    if ((myrank) == (who)) {                               \
        fprintf(stdout, "[DEBUG::%0*d %s:%03u] " fmt "\n", \
            nprefix, myrank,                               \
            __FILE__, __LINE__, ##__VA_ARGS__ );           \
        fflush(stdout);                                    \
    }                                                      \
} while (0)
#endif /* DEBUG */

#ifdef DEBUG_ORDERED
#warning DEBUG_ORDERED was predefined, skipped definition from libmys
#else
#define DEBUG_ORDERED(fmt, ...) do {   \
    int nranks = NRANKS();             \
    for (int i = 0; i < nranks; i++) { \
        DEBUG(i, fmt, ##__VA_ARGS__);  \
        BARRIER();                     \
    }                                  \
} while (0)
#endif /* DEBUG_ORDERED */

#ifdef PRINTF
#warning PRINTF was predefined, skipped definition from libmys
#else
#define PRINTF(who, fmt, ...) do {           \
    int myrank = MYRANK();                   \
    _Pragma("omp critical (mys)")            \
    if ((myrank) == (who)) {                 \
        fprintf(stdout, fmt, ##__VA_ARGS__); \
        fflush(stdout);                      \
    }                                        \
} while (0)
#endif /* PRINTF */


#ifdef ASSERT
#warning ASSERT was predefined, skipped definition from libmys
#else
#define ASSERT(exp, fmt, ...) do {                          \
    _Pragma("omp critical (mys)")                           \
    if (!(exp)) {                                           \
        int nranks = NRANKS(), myrank = MYRANK();           \
        int nprefix = trunc(log10(nranks)) + 1;             \
        fprintf(stdout, "[ASSERT::%0*d %s:%03u] " fmt "\n", \
            nprefix, myrank,                                \
            __FILE__, __LINE__, ##__VA_ARGS__);             \
        fflush(stdout);                                     \
        exit(1);                                            \
    }                                                       \
} while (0)
#endif /* ASSERT */

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


#ifdef FAILED
#warning FAILED was predefined, skipped definition from libmys
#else
#define FAILED(fmt, ...) do {                               \
    int myrank = MYRANK();                                  \
    _Pragma("omp critical (mys)")                           \
    fprintf(stdout, "[FAILED %s:%03u rank=%03d] " fmt "\n", \
        __FILE__, __LINE__, myrank, ##__VA_ARGS__           \
    );                                                      \
    fflush(stdout);                                         \
    exit(1);                                                \
} while (0)
#endif /* FAILED */


#ifdef THROW_NOT_IMPL
#warning THROW_NOT_IMPL was predefined, skipped definition from libmys
#else
#define THROW_NOT_IMPL() FAILED("Not implemented.")
#endif /* THROW_NOT_IMPL */


#ifdef WAIT_FLAG
#warning WAIT_FLAG was predefined, skipped definition from libmys
#else
#include <unistd.h>
#include <sys/stat.h>
#define WAIT_FLAG(file) do {                      \
    int myrank = MYRANK();                        \
    _Pragma("omp critical (mys)")                 \
    if (myrank == 0) {                            \
        fprintf(stdout, "[WAIT %s:%03u] "         \
            "Use \"touch %s\" to continue... ",   \
            __FILE__, __LINE__, file);            \
        fflush(stdout);                           \
    }                                             \
    struct stat fstat;                            \
    time_t mod_time;                              \
    if (stat(file, &fstat) == 0) {                \
        mod_time = fstat.st_mtime;                \
    } else {                                      \
        mod_time = 0;                             \
    }                                             \
    while (true) {                                \
        if (stat(file, &fstat) == 0) {            \
            if (mod_time < fstat.st_mtime) break; \
        }                                         \
        sleep(1);                                 \
    }                                             \
    MPI_Barrier(MPI_COMM_WORLD);                  \
    _Pragma("omp critical (mys)")                 \
    if (myrank == 0) {                            \
        fprintf(stdout, "OK\n");                  \
        fflush(stdout);                           \
    }                                             \
} while (0)
#endif /*  WAIT_FLAG */
