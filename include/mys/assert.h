#pragma once

#include <stdint.h>

#include "_config.h"
#include "log.h"


/* Compile-time Assertion */

#ifndef STATIC_ASSERT
#if defined(__cplusplus) || defined(static_assert)
#define STATIC_ASSERT(expr, diagnostic) static_assert(expr, diagnostic)
#else
/** glibc: misc/sys/cdefs.h
 * [commit] 3999d26ead93990b244ada078073fb58fb8bb5be
 * > Add ersatz _Static_assert on older C hosts
 * Define a substitute, if on a pre-C11 C platform
 * that is not known to support _Static_assert.
 */
#define STATIC_ASSERT(expr, diagnostic) \
    extern int (*__Static_assert_function (void)) \
      [!!sizeof (struct { int emit_error_if_static_assert_failed: (expr) ? 2 : -1; })]
#endif
#endif

/* Runtime Assertion */

#define ASSERT(exp, fmt, ...) do {     \
    if (!(exp)) {                      \
        int myrank = mys_myrank();     \
        mys_log(myrank, MYS_LOG_FATAL, \
          MYS_LOG_FNAME, __LINE__,     \
          (fmt), ##__VA_ARGS__);       \
        exit(1);                       \
    }                                  \
} while(0)
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

#define ABORT(code) ASSERT(false, "Abort by demand. (code %d)", code)
#define FAILED(fmt, ...) ASSERT(false, fmt, ##__VA_ARGS__)

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

#define AS_INT_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d","") // Assert exp1 == exp2 (int)
#define AS_INT_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d","") // Assert exp1 != exp2 (int)
#define AS_INT_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d","") // Assert exp1 < exp2 (int)
#define AS_INT_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d","") // Assert exp1 <= exp2 (int)
#define AS_INT_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d","") // Assert exp1 > exp2 (int)
#define AS_INT_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d","") // Assert exp1 >= exp2 (int)
#define AS_INT_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d","") // Assert low <= exp < high (int)
#define AS_INT_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d","") // Assert low <= exp <= high (int)
#define AS_I8_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhd","") // Assert exp1 == exp2 (int8_t)
#define AS_I8_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhd","") // Assert exp1 != exp2 (int8_t)
#define AS_I8_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhd","") // Assert exp1 < exp2 (int8_t)
#define AS_I8_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhd","") // Assert exp1 <= exp2 (int8_t)
#define AS_I8_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhd","") // Assert exp1 > exp2 (int8_t)
#define AS_I8_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhd","") // Assert exp1 >= exp2 (int8_t)
#define AS_I8_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhd","") // Assert low <= exp < high (int8_t)
#define AS_I8_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhd","") // Assert low <= exp <= high (int8_t)
#define AS_I16_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hd","") // Assert exp1 == exp2 (int16_t)
#define AS_I16_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hd","") // Assert exp1 != exp2 (int16_t)
#define AS_I16_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hd","") // Assert exp1 < exp2 (int16_t)
#define AS_I16_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hd","") // Assert exp1 <= exp2 (int16_t)
#define AS_I16_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hd","") // Assert exp1 > exp2 (int16_t)
#define AS_I16_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hd","") // Assert exp1 >= exp2 (int16_t)
#define AS_I16_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hd","") // Assert low <= exp < high (int16_t)
#define AS_I16_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hd","") // Assert low <= exp <= high (int16_t)
#define AS_I32_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d","") // Assert exp1 == exp2 (int32_t)
#define AS_I32_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d","") // Assert exp1 != exp2 (int32_t)
#define AS_I32_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d","") // Assert exp1 < exp2 (int32_t)
#define AS_I32_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d","") // Assert exp1 <= exp2 (int32_t)
#define AS_I32_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d","") // Assert exp1 > exp2 (int32_t)
#define AS_I32_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d","") // Assert exp1 >= exp2 (int32_t)
#define AS_I32_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d","") // Assert low <= exp < high (int32_t)
#define AS_I32_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d","") // Assert low <= exp <= high (int32_t)
#define AS_I64_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%lld","") // Assert exp1 == exp2 (int64_t)
#define AS_I64_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%lld","") // Assert exp1 != exp2 (int64_t)
#define AS_I64_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%lld","") // Assert exp1 < exp2 (int64_t)
#define AS_I64_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%lld","") // Assert exp1 <= exp2 (int64_t)
#define AS_I64_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%lld","") // Assert exp1 > exp2 (int64_t)
#define AS_I64_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%lld","") // Assert exp1 >= exp2 (int64_t)
#define AS_I64_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%lld","") // Assert low <= exp < high (int64_t)
#define AS_I64_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%lld","") // Assert low <= exp <= high (int64_t)
#define AS_U8_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhu","") // Assert exp1 == exp2 (uint8_t)
#define AS_U8_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhu","") // Assert exp1 != exp2 (uint8_t)
#define AS_U8_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhu","") // Assert exp1 < exp2 (uint8_t)
#define AS_U8_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhu","") // Assert exp1 <= exp2 (uint8_t)
#define AS_U8_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhu","") // Assert exp1 > exp2 (uint8_t)
#define AS_U8_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhu","") // Assert exp1 >= exp2 (uint8_t)
#define AS_U8_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhu","") // Assert low <= exp < high (uint8_t)
#define AS_U8_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhu","") // Assert low <= exp <= high (uint8_t)
#define AS_U16_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hu","") // Assert exp1 == exp2 (uint16_t)
#define AS_U16_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hu","") // Assert exp1 != exp2 (uint16_t)
#define AS_U16_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hu","") // Assert exp1 < exp2 (uint16_t)
#define AS_U16_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hu","") // Assert exp1 <= exp2 (uint16_t)
#define AS_U16_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hu","") // Assert exp1 > exp2 (uint16_t)
#define AS_U16_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hu","") // Assert exp1 >= exp2 (uint16_t)
#define AS_U16_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hu","") // Assert low <= exp < high (uint16_t)
#define AS_U16_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hu","") // Assert low <= exp <= high (uint16_t)
#define AS_U32_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%u","") // Assert exp1 == exp2 (uint32_t)
#define AS_U32_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%u","") // Assert exp1 != exp2 (uint32_t)
#define AS_U32_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%u","") // Assert exp1 < exp2 (uint32_t)
#define AS_U32_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%u","") // Assert exp1 <= exp2 (uint32_t)
#define AS_U32_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%u","") // Assert exp1 > exp2 (uint32_t)
#define AS_U32_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%u","") // Assert exp1 >= exp2 (uint32_t)
#define AS_U32_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%u","") // Assert low <= exp < high (uint32_t)
#define AS_U32_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%u","") // Assert low <= exp <= high (uint32_t)
#define AS_U64_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%llu","") // Assert exp1 == exp2 (uint64_t)
#define AS_U64_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%llu","") // Assert exp1 != exp2 (uint64_t)
#define AS_U64_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%llu","") // Assert exp1 < exp2 (uint64_t)
#define AS_U64_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%llu","") // Assert exp1 <= exp2 (uint64_t)
#define AS_U64_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%llu","") // Assert exp1 > exp2 (uint64_t)
#define AS_U64_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%llu","") // Assert exp1 >= exp2 (uint64_t)
#define AS_U64_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%llu","") // Assert low <= exp < high (uint64_t)
#define AS_U64_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%llu","") // Assert low <= exp <= high (uint64_t)
#define AS_PTR_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%p","") // Assert exp1 == exp2 (void *)
#define AS_PTR_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%p","") // Assert exp1 != exp2 (void *)
#define AS_PTR_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%p","") // Assert exp1 < exp2 (void *)
#define AS_PTR_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%p","") // Assert exp1 <= exp2 (void *)
#define AS_PTR_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%p","") // Assert exp1 > exp2 (void *)
#define AS_PTR_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%p","") // Assert exp1 >= exp2 (void *)
#define AS_PTR_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%p","") // Assert low <= exp < high (void *)
#define AS_PTR_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%p","") // Assert low <= exp <= high (void *)
#define AS_SIZET_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zu","") // Assert exp1 == exp2 (size_t)
#define AS_SIZET_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zu","") // Assert exp1 != exp2 (size_t)
#define AS_SIZET_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zu","") // Assert exp1 < exp2 (size_t)
#define AS_SIZET_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zu","") // Assert exp1 <= exp2 (size_t)
#define AS_SIZET_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zu","") // Assert exp1 > exp2 (size_t)
#define AS_SIZET_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zu","") // Assert exp1 >= exp2 (size_t)
#define AS_SIZET_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zu","") // Assert low <= exp < high (size_t)
#define AS_SIZET_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zu","") // Assert low <= exp <= high (size_t)
#define AS_SSIZET_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zd","") // Assert exp1 == exp2 (ssize_t)
#define AS_SSIZET_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zd","") // Assert exp1 != exp2 (ssize_t)
#define AS_SSIZET_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zd","") // Assert exp1 < exp2 (ssize_t)
#define AS_SSIZET_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zd","") // Assert exp1 <= exp2 (ssize_t)
#define AS_SSIZET_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zd","") // Assert exp1 > exp2 (ssize_t)
#define AS_SSIZET_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zd","") // Assert exp1 >= exp2 (ssize_t)
#define AS_SSIZET_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zd","") // Assert low <= exp < high (ssize_t)
#define AS_SSIZET_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zd","") // Assert low <= exp <= high (ssize_t)
#define AS_F32_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f","") // Assert exp1 == exp2 (float)
#define AS_F32_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f","") // Assert exp1 != exp2 (float)
#define AS_F32_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f","") // Assert exp1 < exp2 (float)
#define AS_F32_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f","") // Assert exp1 <= exp2 (float)
#define AS_F32_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f","") // Assert exp1 > exp2 (float)
#define AS_F32_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f","") // Assert exp1 >= exp2 (float)
#define AS_F32_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f","") // Assert low <= exp < high (float)
#define AS_F32_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f","") // Assert low <= exp <= high (float)
#define AS_F64_EQ(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f","") // Assert exp1 == exp2 (double)
#define AS_F64_NE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f","") // Assert exp1 != exp2 (double)
#define AS_F64_LT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f","") // Assert exp1 < exp2 (double)
#define AS_F64_LE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f","") // Assert exp1 <= exp2 (double)
#define AS_F64_GT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f","") // Assert exp1 > exp2 (double)
#define AS_F64_GE(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f","") // Assert exp1 >= exp2 (double)
#define AS_F64_BETWEEN_IE(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f","") // Assert low <= exp < high (double)
#define AS_F64_BETWEEN_II(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f","") // Assert low <= exp <= high (double)
#define ASX_INT_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int)
#define ASX_INT_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int)
#define ASX_INT_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int)
#define ASX_INT_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int)
#define ASX_INT_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int)
#define ASX_INT_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int)
#define ASX_INT_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp < high (int)
#define ASX_INT_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int)
#define ASX_I8_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int8_t)
#define ASX_I8_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int8_t)
#define ASX_I8_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int8_t)
#define ASX_I8_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int8_t)
#define ASX_I8_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int8_t)
#define ASX_I8_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int8_t)
#define ASX_I8_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhd",fmt,##__VA_ARGS__) // Assert low <= exp < high (int8_t)
#define ASX_I8_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int8_t)
#define ASX_I16_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int16_t)
#define ASX_I16_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int16_t)
#define ASX_I16_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int16_t)
#define ASX_I16_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int16_t)
#define ASX_I16_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int16_t)
#define ASX_I16_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int16_t)
#define ASX_I16_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp < high (int16_t)
#define ASX_I16_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int16_t)
#define ASX_I32_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int32_t)
#define ASX_I32_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int32_t)
#define ASX_I32_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int32_t)
#define ASX_I32_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int32_t)
#define ASX_I32_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int32_t)
#define ASX_I32_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int32_t)
#define ASX_I32_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp < high (int32_t)
#define ASX_I32_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int32_t)
#define ASX_I64_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int64_t)
#define ASX_I64_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int64_t)
#define ASX_I64_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int64_t)
#define ASX_I64_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int64_t)
#define ASX_I64_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int64_t)
#define ASX_I64_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int64_t)
#define ASX_I64_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp < high (int64_t)
#define ASX_I64_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int64_t)
#define ASX_U8_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint8_t)
#define ASX_U8_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint8_t)
#define ASX_U8_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint8_t)
#define ASX_U8_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint8_t)
#define ASX_U8_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint8_t)
#define ASX_U8_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint8_t)
#define ASX_U8_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint8_t)
#define ASX_U8_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint8_t)
#define ASX_U16_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint16_t)
#define ASX_U16_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint16_t)
#define ASX_U16_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint16_t)
#define ASX_U16_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint16_t)
#define ASX_U16_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint16_t)
#define ASX_U16_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint16_t)
#define ASX_U16_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint16_t)
#define ASX_U16_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint16_t)
#define ASX_U32_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint32_t)
#define ASX_U32_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint32_t)
#define ASX_U32_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint32_t)
#define ASX_U32_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint32_t)
#define ASX_U32_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint32_t)
#define ASX_U32_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint32_t)
#define ASX_U32_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint32_t)
#define ASX_U32_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint32_t)
#define ASX_U64_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint64_t)
#define ASX_U64_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint64_t)
#define ASX_U64_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint64_t)
#define ASX_U64_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint64_t)
#define ASX_U64_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint64_t)
#define ASX_U64_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint64_t)
#define ASX_U64_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint64_t)
#define ASX_U64_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint64_t)
#define ASX_PTR_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (void *)
#define ASX_PTR_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (void *)
#define ASX_PTR_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (void *)
#define ASX_PTR_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (void *)
#define ASX_PTR_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (void *)
#define ASX_PTR_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (void *)
#define ASX_PTR_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%p",fmt,##__VA_ARGS__) // Assert low <= exp < high (void *)
#define ASX_PTR_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%p",fmt,##__VA_ARGS__) // Assert low <= exp <= high (void *)
#define ASX_SIZET_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (size_t)
#define ASX_SIZET_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (size_t)
#define ASX_SIZET_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (size_t)
#define ASX_SIZET_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (size_t)
#define ASX_SIZET_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (size_t)
#define ASX_SIZET_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (size_t)
#define ASX_SIZET_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zu",fmt,##__VA_ARGS__) // Assert low <= exp < high (size_t)
#define ASX_SIZET_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (size_t)
#define ASX_SSIZET_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (ssize_t)
#define ASX_SSIZET_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (ssize_t)
#define ASX_SSIZET_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (ssize_t)
#define ASX_SSIZET_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (ssize_t)
#define ASX_SSIZET_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (ssize_t)
#define ASX_SSIZET_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (ssize_t)
#define ASX_SSIZET_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zd",fmt,##__VA_ARGS__) // Assert low <= exp < high (ssize_t)
#define ASX_SSIZET_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (ssize_t)
#define ASX_F32_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (float)
#define ASX_F32_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (float)
#define ASX_F32_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (float)
#define ASX_F32_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (float)
#define ASX_F32_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (float)
#define ASX_F32_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (float)
#define ASX_F32_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp < high (float)
#define ASX_F32_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp <= high (float)
#define ASX_F64_EQ(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (double)
#define ASX_F64_NE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (double)
#define ASX_F64_LT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (double)
#define ASX_F64_LE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (double)
#define ASX_F64_GT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (double)
#define ASX_F64_GE(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (double)
#define ASX_F64_BETWEEN_IE(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp < high (double)
#define ASX_F64_BETWEEN_II(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp <= high (double)
/*
categories = [("AS", False), ("ASX", True)]
types = [
    ("int", "INT", "%d"),
    ("int8_t", "I8", "%hhd"),
    ("int16_t", "I16", "%hd"),
    ("int32_t", "I32", "%d"),
    ("int64_t", "I64", "%lld"),
    ("uint8_t", "U8", "%hhu"),
    ("uint16_t", "U16", "%hu"),
    ("uint32_t", "U32", "%u"),
    ("uint64_t", "U64", "%llu"),
    ("void *", "PTR", "%p"),
    ("size_t", "SIZET", "%zu"),
    ("ssize_t", "SSIZET", "%zd"),
    ("float", "F32", "%f"),
    ("double", "F64", "%f"),
]
ops = [("EQ", 1, "=="),("NE", 1, "!="),("LT", 1, "<"),("LE", 1, "<="),("GT", 1, ">"),("GE", 1, ">="),("BETWEEN_IE", 2, "<=", "<"),("BETWEEN_II", 2, "<=", "<=")]

for category in categories:
    for typ in types:
        max_prefix = 0
        lines = []
        for op in ops:
            prefix = f"#define {category[0]}_{typ[1]}_{op[0]}"
            if op[1] == 1:
                prefix += f"(exp1, exp2"
            elif op[1] == 2:
                prefix += f"(low, exp, high"
            if category[1] == False:
                prefix += f")"
            else:
                prefix += f", fmt, ...)"
            suffix = ""
            if op[1] == 1:
                suffix += f'__ASSERTX_ONE_OP__(exp1,{op[2]:2s},exp2,"{typ[2]}",'
            elif op[1] == 2:
                suffix += f'__ASSERTX_TWO_OP__(low,{op[2]:2s},exp,{op[3]:2s},high,"{typ[2]}",'
            if category[1] == False:
                suffix += f'"")'
            else:
                suffix += f'fmt,##__VA_ARGS__)'
            if op[1] == 1:
                suffix += f" // Assert exp1 {op[2]} exp2 ({typ[0]})"
            elif op[1] == 2:
                suffix += f" // Assert low {op[2]} exp {op[3]} high ({typ[0]})"
            max_prefix = max(max_prefix, len(prefix))
            lines.append((prefix, suffix))
        for prefix, suffix in lines:
            print(f"{prefix.ljust(max_prefix)} {suffix}")
*/
