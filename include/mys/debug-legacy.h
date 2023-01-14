#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "config.h"
#include "myspi.h"

#if defined(_OPENMP) && !defined(MYS_OMP_CRITICAL)
#define MYS_OMP_CRITICAL _Pragma("omp critical (mys)")
#else
#define MYS_OMP_CRITICAL
#endif

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
#define ASSERT_FLOAT_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%E","")
#define ASSERT_FLOAT_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%E","")
#define ASSERT_FLOAT_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%E","")
#define ASSERT_FLOAT_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%E","")
#define ASSERT_FLOAT_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%E","")
#define ASSERT_FLOAT_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%E","")
#define ASSERTX_FLOAT_EQ(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,==,exp2,"%E",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_NE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,!=,exp2,"%E",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_LT(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,< ,exp2,"%E",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_LE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,<=,exp2,"%E",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_GT(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,> ,exp2,"%E",fmt,##__VA_ARGS__)
#define ASSERTX_FLOAT_GE(exp1, exp2, fmt, ...)  __ASSERTX_ONE_OP__(exp1,>=,exp2,"%E",fmt,##__VA_ARGS__)
#define ASSERT_FLOAT_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%E","")
#define ASSERT_FLOAT_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%E","")
#endif /* __ASSERT_SUGAR__ */

#define FAILED(fmt, ...) __mys_failed(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define THROW_NOT_IMPL() FAILED("Not implemented.")
#define WAIT_FLAG(flagfile) __mys_wait_flag(__FILE__, __LINE__, flagfile)

/* Validate return value */
#define CHKRET(fncall)       \
do {                         \
    int val = (int)(fncall); \
    ASSERT_EQ(val, 0);       \
} while (0)

/* Validate pointer */
#define CHKPTR(fncall)             \
do {                               \
    size_t val = (size_t)(fncall); \
    ASSERT_NE(val, 0);             \
} while (0)

/*********************************************/
/* Implement */
/*********************************************/

__attribute__((format(printf, 2, 3)))
static inline void __mys_printf(int who, const char *fmt, ...)
{
    int myrank = __mys_myrank();
    MYS_OMP_CRITICAL
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
    MYS_OMP_CRITICAL
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
    MYS_OMP_CRITICAL
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
        exit(1);
    }
}

__attribute__((format(printf, 3, 4)))
static inline void __mys_failed(const char *file, int line, const char *fmt, ...)
{
    MYS_OMP_CRITICAL
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
#include <sys/time.h>
static inline void __mys_wait_flag(const char *file, int line, const char *flagfile) {
    int myrank = __mys_myrank();
    MYS_OMP_CRITICAL
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
    MYS_OMP_CRITICAL
    if (myrank == 0) {
        fprintf(stdout, "OK\n");
        fflush(stdout);
    }
}
