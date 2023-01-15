#pragma once

#include <stdint.h>

#include "config.h"
#include "log.h"

/* Runtime Assertion */

#define ASSERT(exp, fmt, ...) do {       \
    if (!(exp)) {                        \
        int myrank = __mys_myrank();     \
        __mys_log(myrank, MYS_LOG_FATAL, \
          __MYS_LOG_FNAME__, __LINE__,   \
          (fmt), ##__VA_ARGS__);         \
        exit(1);                         \
    }                                    \
} while(0)
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

/* Validate return value */
#define CHKRET(fncall) do {              \
    int val = (int)(fncall);             \
    ASSERT(val == 0,                     \
        "Expect (%s) return %d but %d.", \
        #fncall, 0, val);                \
} while (0)

/* Validate pointer */
#define CHKPTR(fncall) do {               \
    size_t val = (size_t)(fncall);        \
    ASSERT(val != 0,                      \
        "Expect (%s) no returning NULL.", \
        #fncall);                         \
} while (0)
