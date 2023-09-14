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
// FIXME: We have to make sure exp1 and exp2, exp3 don't change in multi-threading environment.
// For example, We should force evaluate a data-race variable first, instead of simply use array[i] multiple times

#define ASSERT(exp, fmt, ...) do {     \
    if (!(exp)) {                      \
        int myrank = mys_mpi_myrank(); \
        mys_log(myrank, MYS_LOG_FATAL, \
          MYS_FNAME, __LINE__,         \
          (fmt), ##__VA_ARGS__);       \
        exit(1);                       \
    }                                  \
} while(0)
#define __ASSERTX_ZERO_OP__(exp, expect, actual, fmt, ...) ASSERT( \
        (exp),                                                     \
        "Expect %s was %s but %s. " fmt,                           \
        #exp, expect, actual, ##__VA_ARGS__                        \
    )
#define __ASSERTX_ONE_OP__(exp1, op, exp2, spec, fmt, ...) ASSERT(   \
        (exp1) op (exp2),                                            \
        "Expect (%s %s %s) but (" spec " %s " spec ") failed. " fmt, \
        #exp1, #op, #exp2,                                           \
        (exp1), #op, (exp2), ##__VA_ARGS__                           \
    )
#define __ASSERTX_TWO_OP__(exp1, op1, exp2, op2, exp3, spec, fmt, ...) ASSERT(         \
        ((exp1) op1 (exp2)) && ((exp2) op2 (exp3)),                                    \
        "Expect (%s %s %s %s %s) but (" spec " %s " spec " %s " spec ") failed. " fmt, \
        #exp1, #op1, #exp2, #op2, #exp3,                                               \
        (exp1), #op1, (exp2), #op2, (exp3), ##__VA_ARGS__                              \
    )
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

#define AS_EQ_INT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d","") // Assert exp1 == exp2 (int)
#define AS_NE_INT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d","") // Assert exp1 != exp2 (int)
#define AS_LT_INT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d","") // Assert exp1 < exp2 (int)
#define AS_LE_INT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d","") // Assert exp1 <= exp2 (int)
#define AS_GT_INT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d","") // Assert exp1 > exp2 (int)
#define AS_GE_INT(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d","") // Assert exp1 >= exp2 (int)
#define AS_BETWEEN_IE_INT(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d","") // Assert low <= exp < high (int)
#define AS_BETWEEN_II_INT(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d","") // Assert low <= exp <= high (int)
#define AS_EQ_I8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhd","") // Assert exp1 == exp2 (int8_t)
#define AS_NE_I8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhd","") // Assert exp1 != exp2 (int8_t)
#define AS_LT_I8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhd","") // Assert exp1 < exp2 (int8_t)
#define AS_LE_I8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhd","") // Assert exp1 <= exp2 (int8_t)
#define AS_GT_I8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhd","") // Assert exp1 > exp2 (int8_t)
#define AS_GE_I8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhd","") // Assert exp1 >= exp2 (int8_t)
#define AS_BETWEEN_IE_I8(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhd","") // Assert low <= exp < high (int8_t)
#define AS_BETWEEN_II_I8(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhd","") // Assert low <= exp <= high (int8_t)
#define AS_EQ_I16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hd","") // Assert exp1 == exp2 (int16_t)
#define AS_NE_I16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hd","") // Assert exp1 != exp2 (int16_t)
#define AS_LT_I16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hd","") // Assert exp1 < exp2 (int16_t)
#define AS_LE_I16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hd","") // Assert exp1 <= exp2 (int16_t)
#define AS_GT_I16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hd","") // Assert exp1 > exp2 (int16_t)
#define AS_GE_I16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hd","") // Assert exp1 >= exp2 (int16_t)
#define AS_BETWEEN_IE_I16(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hd","") // Assert low <= exp < high (int16_t)
#define AS_BETWEEN_II_I16(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hd","") // Assert low <= exp <= high (int16_t)
#define AS_EQ_I32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d","") // Assert exp1 == exp2 (int32_t)
#define AS_NE_I32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d","") // Assert exp1 != exp2 (int32_t)
#define AS_LT_I32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d","") // Assert exp1 < exp2 (int32_t)
#define AS_LE_I32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d","") // Assert exp1 <= exp2 (int32_t)
#define AS_GT_I32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d","") // Assert exp1 > exp2 (int32_t)
#define AS_GE_I32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d","") // Assert exp1 >= exp2 (int32_t)
#define AS_BETWEEN_IE_I32(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d","") // Assert low <= exp < high (int32_t)
#define AS_BETWEEN_II_I32(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d","") // Assert low <= exp <= high (int32_t)
#define AS_EQ_I64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%lld","") // Assert exp1 == exp2 (int64_t)
#define AS_NE_I64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%lld","") // Assert exp1 != exp2 (int64_t)
#define AS_LT_I64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%lld","") // Assert exp1 < exp2 (int64_t)
#define AS_LE_I64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%lld","") // Assert exp1 <= exp2 (int64_t)
#define AS_GT_I64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%lld","") // Assert exp1 > exp2 (int64_t)
#define AS_GE_I64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%lld","") // Assert exp1 >= exp2 (int64_t)
#define AS_BETWEEN_IE_I64(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%lld","") // Assert low <= exp < high (int64_t)
#define AS_BETWEEN_II_I64(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%lld","") // Assert low <= exp <= high (int64_t)
#define AS_EQ_U8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhu","") // Assert exp1 == exp2 (uint8_t)
#define AS_NE_U8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhu","") // Assert exp1 != exp2 (uint8_t)
#define AS_LT_U8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhu","") // Assert exp1 < exp2 (uint8_t)
#define AS_LE_U8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhu","") // Assert exp1 <= exp2 (uint8_t)
#define AS_GT_U8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhu","") // Assert exp1 > exp2 (uint8_t)
#define AS_GE_U8(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhu","") // Assert exp1 >= exp2 (uint8_t)
#define AS_BETWEEN_IE_U8(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhu","") // Assert low <= exp < high (uint8_t)
#define AS_BETWEEN_II_U8(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhu","") // Assert low <= exp <= high (uint8_t)
#define AS_EQ_U16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hu","") // Assert exp1 == exp2 (uint16_t)
#define AS_NE_U16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hu","") // Assert exp1 != exp2 (uint16_t)
#define AS_LT_U16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hu","") // Assert exp1 < exp2 (uint16_t)
#define AS_LE_U16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hu","") // Assert exp1 <= exp2 (uint16_t)
#define AS_GT_U16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hu","") // Assert exp1 > exp2 (uint16_t)
#define AS_GE_U16(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hu","") // Assert exp1 >= exp2 (uint16_t)
#define AS_BETWEEN_IE_U16(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hu","") // Assert low <= exp < high (uint16_t)
#define AS_BETWEEN_II_U16(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hu","") // Assert low <= exp <= high (uint16_t)
#define AS_EQ_U32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%u","") // Assert exp1 == exp2 (uint32_t)
#define AS_NE_U32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%u","") // Assert exp1 != exp2 (uint32_t)
#define AS_LT_U32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%u","") // Assert exp1 < exp2 (uint32_t)
#define AS_LE_U32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%u","") // Assert exp1 <= exp2 (uint32_t)
#define AS_GT_U32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%u","") // Assert exp1 > exp2 (uint32_t)
#define AS_GE_U32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%u","") // Assert exp1 >= exp2 (uint32_t)
#define AS_BETWEEN_IE_U32(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%u","") // Assert low <= exp < high (uint32_t)
#define AS_BETWEEN_II_U32(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%u","") // Assert low <= exp <= high (uint32_t)
#define AS_EQ_U64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%llu","") // Assert exp1 == exp2 (uint64_t)
#define AS_NE_U64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%llu","") // Assert exp1 != exp2 (uint64_t)
#define AS_LT_U64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%llu","") // Assert exp1 < exp2 (uint64_t)
#define AS_LE_U64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%llu","") // Assert exp1 <= exp2 (uint64_t)
#define AS_GT_U64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%llu","") // Assert exp1 > exp2 (uint64_t)
#define AS_GE_U64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%llu","") // Assert exp1 >= exp2 (uint64_t)
#define AS_BETWEEN_IE_U64(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%llu","") // Assert low <= exp < high (uint64_t)
#define AS_BETWEEN_II_U64(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%llu","") // Assert low <= exp <= high (uint64_t)
#define AS_EQ_PTR(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%p","") // Assert exp1 == exp2 (void *)
#define AS_NE_PTR(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%p","") // Assert exp1 != exp2 (void *)
#define AS_LT_PTR(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%p","") // Assert exp1 < exp2 (void *)
#define AS_LE_PTR(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%p","") // Assert exp1 <= exp2 (void *)
#define AS_GT_PTR(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%p","") // Assert exp1 > exp2 (void *)
#define AS_GE_PTR(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%p","") // Assert exp1 >= exp2 (void *)
#define AS_BETWEEN_IE_PTR(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%p","") // Assert low <= exp < high (void *)
#define AS_BETWEEN_II_PTR(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%p","") // Assert low <= exp <= high (void *)
#define AS_EQ_SIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zu","") // Assert exp1 == exp2 (size_t)
#define AS_NE_SIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zu","") // Assert exp1 != exp2 (size_t)
#define AS_LT_SIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zu","") // Assert exp1 < exp2 (size_t)
#define AS_LE_SIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zu","") // Assert exp1 <= exp2 (size_t)
#define AS_GT_SIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zu","") // Assert exp1 > exp2 (size_t)
#define AS_GE_SIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zu","") // Assert exp1 >= exp2 (size_t)
#define AS_BETWEEN_IE_SIZET(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zu","") // Assert low <= exp < high (size_t)
#define AS_BETWEEN_II_SIZET(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zu","") // Assert low <= exp <= high (size_t)
#define AS_EQ_SSIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zd","") // Assert exp1 == exp2 (ssize_t)
#define AS_NE_SSIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zd","") // Assert exp1 != exp2 (ssize_t)
#define AS_LT_SSIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zd","") // Assert exp1 < exp2 (ssize_t)
#define AS_LE_SSIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zd","") // Assert exp1 <= exp2 (ssize_t)
#define AS_GT_SSIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zd","") // Assert exp1 > exp2 (ssize_t)
#define AS_GE_SSIZET(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zd","") // Assert exp1 >= exp2 (ssize_t)
#define AS_BETWEEN_IE_SSIZET(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zd","") // Assert low <= exp < high (ssize_t)
#define AS_BETWEEN_II_SSIZET(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zd","") // Assert low <= exp <= high (ssize_t)
#define AS_EQ_F32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f","") // Assert exp1 == exp2 (float)
#define AS_NE_F32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f","") // Assert exp1 != exp2 (float)
#define AS_LT_F32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f","") // Assert exp1 < exp2 (float)
#define AS_LE_F32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f","") // Assert exp1 <= exp2 (float)
#define AS_GT_F32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f","") // Assert exp1 > exp2 (float)
#define AS_GE_F32(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f","") // Assert exp1 >= exp2 (float)
#define AS_BETWEEN_IE_F32(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f","") // Assert low <= exp < high (float)
#define AS_BETWEEN_II_F32(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f","") // Assert low <= exp <= high (float)
#define AS_EQ_F64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f","") // Assert exp1 == exp2 (double)
#define AS_NE_F64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f","") // Assert exp1 != exp2 (double)
#define AS_LT_F64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f","") // Assert exp1 < exp2 (double)
#define AS_LE_F64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f","") // Assert exp1 <= exp2 (double)
#define AS_GT_F64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f","") // Assert exp1 > exp2 (double)
#define AS_GE_F64(exp1, exp2)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f","") // Assert exp1 >= exp2 (double)
#define AS_BETWEEN_IE_F64(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f","") // Assert low <= exp < high (double)
#define AS_BETWEEN_II_F64(low, exp, high) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f","") // Assert low <= exp <= high (double)
#define ASX_EQ_INT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int)
#define ASX_NE_INT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int)
#define ASX_LT_INT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int)
#define ASX_LE_INT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int)
#define ASX_GT_INT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int)
#define ASX_GE_INT(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int)
#define ASX_BETWEEN_IE_INT(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp < high (int)
#define ASX_BETWEEN_II_INT(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int)
#define ASX_EQ_I8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int8_t)
#define ASX_NE_I8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int8_t)
#define ASX_LT_I8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int8_t)
#define ASX_LE_I8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int8_t)
#define ASX_GT_I8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int8_t)
#define ASX_GE_I8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int8_t)
#define ASX_BETWEEN_IE_I8(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhd",fmt,##__VA_ARGS__) // Assert low <= exp < high (int8_t)
#define ASX_BETWEEN_II_I8(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int8_t)
#define ASX_EQ_I16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int16_t)
#define ASX_NE_I16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int16_t)
#define ASX_LT_I16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int16_t)
#define ASX_LE_I16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int16_t)
#define ASX_GT_I16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int16_t)
#define ASX_GE_I16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int16_t)
#define ASX_BETWEEN_IE_I16(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp < high (int16_t)
#define ASX_BETWEEN_II_I16(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int16_t)
#define ASX_EQ_I32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int32_t)
#define ASX_NE_I32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int32_t)
#define ASX_LT_I32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int32_t)
#define ASX_LE_I32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int32_t)
#define ASX_GT_I32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int32_t)
#define ASX_GE_I32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int32_t)
#define ASX_BETWEEN_IE_I32(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp < high (int32_t)
#define ASX_BETWEEN_II_I32(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int32_t)
#define ASX_EQ_I64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int64_t)
#define ASX_NE_I64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int64_t)
#define ASX_LT_I64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int64_t)
#define ASX_LE_I64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int64_t)
#define ASX_GT_I64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int64_t)
#define ASX_GE_I64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int64_t)
#define ASX_BETWEEN_IE_I64(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp < high (int64_t)
#define ASX_BETWEEN_II_I64(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int64_t)
#define ASX_EQ_U8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint8_t)
#define ASX_NE_U8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint8_t)
#define ASX_LT_U8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint8_t)
#define ASX_LE_U8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint8_t)
#define ASX_GT_U8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint8_t)
#define ASX_GE_U8(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint8_t)
#define ASX_BETWEEN_IE_U8(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hhu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint8_t)
#define ASX_BETWEEN_II_U8(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hhu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint8_t)
#define ASX_EQ_U16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint16_t)
#define ASX_NE_U16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint16_t)
#define ASX_LT_U16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint16_t)
#define ASX_LE_U16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint16_t)
#define ASX_GT_U16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint16_t)
#define ASX_GE_U16(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint16_t)
#define ASX_BETWEEN_IE_U16(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint16_t)
#define ASX_BETWEEN_II_U16(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint16_t)
#define ASX_EQ_U32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint32_t)
#define ASX_NE_U32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint32_t)
#define ASX_LT_U32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint32_t)
#define ASX_LE_U32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint32_t)
#define ASX_GT_U32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint32_t)
#define ASX_GE_U32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint32_t)
#define ASX_BETWEEN_IE_U32(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint32_t)
#define ASX_BETWEEN_II_U32(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint32_t)
#define ASX_EQ_U64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint64_t)
#define ASX_NE_U64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint64_t)
#define ASX_LT_U64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint64_t)
#define ASX_LE_U64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint64_t)
#define ASX_GT_U64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint64_t)
#define ASX_GE_U64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint64_t)
#define ASX_BETWEEN_IE_U64(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint64_t)
#define ASX_BETWEEN_II_U64(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint64_t)
#define ASX_EQ_PTR(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (void *)
#define ASX_NE_PTR(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (void *)
#define ASX_LT_PTR(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (void *)
#define ASX_LE_PTR(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (void *)
#define ASX_GT_PTR(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (void *)
#define ASX_GE_PTR(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (void *)
#define ASX_BETWEEN_IE_PTR(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%p",fmt,##__VA_ARGS__) // Assert low <= exp < high (void *)
#define ASX_BETWEEN_II_PTR(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%p",fmt,##__VA_ARGS__) // Assert low <= exp <= high (void *)
#define ASX_EQ_SIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (size_t)
#define ASX_NE_SIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (size_t)
#define ASX_LT_SIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (size_t)
#define ASX_LE_SIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (size_t)
#define ASX_GT_SIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (size_t)
#define ASX_GE_SIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (size_t)
#define ASX_BETWEEN_IE_SIZET(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zu",fmt,##__VA_ARGS__) // Assert low <= exp < high (size_t)
#define ASX_BETWEEN_II_SIZET(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (size_t)
#define ASX_EQ_SSIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (ssize_t)
#define ASX_NE_SSIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (ssize_t)
#define ASX_LT_SSIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (ssize_t)
#define ASX_LE_SSIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (ssize_t)
#define ASX_GT_SSIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (ssize_t)
#define ASX_GE_SSIZET(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (ssize_t)
#define ASX_BETWEEN_IE_SSIZET(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%zd",fmt,##__VA_ARGS__) // Assert low <= exp < high (ssize_t)
#define ASX_BETWEEN_II_SSIZET(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%zd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (ssize_t)
#define ASX_EQ_F32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (float)
#define ASX_NE_F32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (float)
#define ASX_LT_F32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (float)
#define ASX_LE_F32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (float)
#define ASX_GT_F32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (float)
#define ASX_GE_F32(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (float)
#define ASX_BETWEEN_IE_F32(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp < high (float)
#define ASX_BETWEEN_II_F32(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp <= high (float)
#define ASX_EQ_F64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,==,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (double)
#define ASX_NE_F64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,!=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (double)
#define ASX_LT_F64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,< ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (double)
#define ASX_LE_F64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,<=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (double)
#define ASX_GT_F64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,> ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (double)
#define ASX_GE_F64(exp1, exp2, fmt, ...)             __ASSERTX_ONE_OP__(exp1,>=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (double)
#define ASX_BETWEEN_IE_F64(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp < high (double)
#define ASX_BETWEEN_II_F64(low, exp, high, fmt, ...) __ASSERTX_TWO_OP__(low,<=,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp <= high (double)
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
            prefix = f"#define {category[0]}_{op[0]}_{typ[1]}"
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
