/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

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

#define _ASX(exp, fmt, ...) if (!(exp)) { \
    int _rank_ = mys_mpi_myrank();        \
    mys_log(_rank_, MYS_LOG_FATAL,        \
        __FILE__, __LINE__,               \
        (fmt), ##__VA_ARGS__);            \
    exit(1);                              \
}

#define ASSERT(exp, fmt, ...) do { _ASX(exp, fmt, ##__VA_ARGS__);                   } while(0)
#define ABORT(code)           do { _ASX(false, "Abort by demand. (code %d)", code); } while(0)
#define FAILED(fmt, ...)      do { _ASX(false, fmt, ##__VA_ARGS__);                 } while(0)
#define THROW_NOT_IMPL()      do { _ASX(false, "Not implemented.");                 } while(0)
#define CHKRET(fncall)        do { const int   _v_ = fncall; _ASX(_v_ == 0,    "Expect (%s) return 0 but %d.",        #fncall, _v_); } while (0) /* Validate return value */
#define CHKPTR(fncall)        do { const void *_v_ = fncall; _ASX(_v_ != NULL, "Expect (%s) return non-NULL but %p.", #fncall, _v_); } while (0) /* Validate pointer */
#define CHKGOTO(exptrue, ret, errcode, label) do {   \
    if (!(exptrue)) { ret = (errcode); goto label; } \
} while (0)

#define _ASX_1(typ, exp, expect, actual, fmt, ...) do { \
    const typ _e0_ = exp;                               \
    _ASX(_e0_,                                          \
        "Expect %s was %s but %s. " fmt,                \
        #exp, expect, actual, ##__VA_ARGS__);           \
} while (0)
#define _ASX_2(typ, exp1, op, exp2, spec, fmt, ...) do {             \
    const typ _e1_ = exp1; const typ _e2_ = exp2;                    \
    _ASX(_e1_ op _e2_,                                               \
        "Expect (%s %s %s) but (" spec " %s " spec ") failed. " fmt, \
        #exp1, #op, #exp2, _e1_, #op, _e2_, ##__VA_ARGS__);          \
} while (0)
#define _ASX_3(typ, exp1, op1, exp2, op2, exp3, spec, fmt, ...) do {                   \
    const typ _e1_ = exp1; const typ _e2_ = exp2; const typ _e3_ = exp3;               \
    _ASX((_e1_ op1 _e2_) && (_e2_ op2 _e3_),                                           \
        "Expect (%s %s %s %s %s) but (" spec " %s " spec " %s " spec ") failed. " fmt, \
        #exp1, #op1, #exp2, #op2, #exp3,                                               \
        _e1_, #op1, _e2_, #op2, _e3_, ##__VA_ARGS__);                                  \
} while (0)

#define AS_TRUE(exp)             _ASX_1(bool, exp, "true", "false", "")
#define AS_FALSE(exp)            _ASX_1(bool, exp, "false", "true", "")
#define ASX_TRUE(exp, fmt, ...)  _ASX_1(bool, exp, "true", "false", fmt, ##__VA_ARGS__)
#define ASX_FALSE(exp, fmt, ...) _ASX_1(bool, exp, "false", "true", fmt, ##__VA_ARGS__)

#define AS_EQ_I8(exp1, exp2)             _ASX_2(int8_t,exp1,==,exp2,"%hhd","") // Assert exp1 == exp2 (int8_t)
#define AS_NE_I8(exp1, exp2)             _ASX_2(int8_t,exp1,!=,exp2,"%hhd","") // Assert exp1 != exp2 (int8_t)
#define AS_LT_I8(exp1, exp2)             _ASX_2(int8_t,exp1,< ,exp2,"%hhd","") // Assert exp1 < exp2 (int8_t)
#define AS_LE_I8(exp1, exp2)             _ASX_2(int8_t,exp1,<=,exp2,"%hhd","") // Assert exp1 <= exp2 (int8_t)
#define AS_GT_I8(exp1, exp2)             _ASX_2(int8_t,exp1,> ,exp2,"%hhd","") // Assert exp1 > exp2 (int8_t)
#define AS_GE_I8(exp1, exp2)             _ASX_2(int8_t,exp1,>=,exp2,"%hhd","") // Assert exp1 >= exp2 (int8_t)
#define AS_BETWEEN_IE_I8(low, exp, high) _ASX_3(int8_t,low,<=,exp,< ,high,"%hhd","") // Assert low <= exp < high (int8_t)
#define AS_BETWEEN_II_I8(low, exp, high) _ASX_3(int8_t,low,<=,exp,<=,high,"%hhd","") // Assert low <= exp <= high (int8_t)
#define AS_BETWEEN_EI_I8(low, exp, high) _ASX_3(int8_t,low,< ,exp,<=,high,"%hhd","") // Assert low < exp <= high (int8_t)
#define AS_BETWEEN_EE_I8(low, exp, high) _ASX_3(int8_t,low,< ,exp,< ,high,"%hhd","") // Assert low < exp < high (int8_t)
#define AS_EQ_I16(exp1, exp2)             _ASX_2(int16_t,exp1,==,exp2,"%hd","") // Assert exp1 == exp2 (int16_t)
#define AS_NE_I16(exp1, exp2)             _ASX_2(int16_t,exp1,!=,exp2,"%hd","") // Assert exp1 != exp2 (int16_t)
#define AS_LT_I16(exp1, exp2)             _ASX_2(int16_t,exp1,< ,exp2,"%hd","") // Assert exp1 < exp2 (int16_t)
#define AS_LE_I16(exp1, exp2)             _ASX_2(int16_t,exp1,<=,exp2,"%hd","") // Assert exp1 <= exp2 (int16_t)
#define AS_GT_I16(exp1, exp2)             _ASX_2(int16_t,exp1,> ,exp2,"%hd","") // Assert exp1 > exp2 (int16_t)
#define AS_GE_I16(exp1, exp2)             _ASX_2(int16_t,exp1,>=,exp2,"%hd","") // Assert exp1 >= exp2 (int16_t)
#define AS_BETWEEN_IE_I16(low, exp, high) _ASX_3(int16_t,low,<=,exp,< ,high,"%hd","") // Assert low <= exp < high (int16_t)
#define AS_BETWEEN_II_I16(low, exp, high) _ASX_3(int16_t,low,<=,exp,<=,high,"%hd","") // Assert low <= exp <= high (int16_t)
#define AS_BETWEEN_EI_I16(low, exp, high) _ASX_3(int16_t,low,< ,exp,<=,high,"%hd","") // Assert low < exp <= high (int16_t)
#define AS_BETWEEN_EE_I16(low, exp, high) _ASX_3(int16_t,low,< ,exp,< ,high,"%hd","") // Assert low < exp < high (int16_t)
#define AS_EQ_I32(exp1, exp2)             _ASX_2(int32_t,exp1,==,exp2,"%d","") // Assert exp1 == exp2 (int32_t)
#define AS_NE_I32(exp1, exp2)             _ASX_2(int32_t,exp1,!=,exp2,"%d","") // Assert exp1 != exp2 (int32_t)
#define AS_LT_I32(exp1, exp2)             _ASX_2(int32_t,exp1,< ,exp2,"%d","") // Assert exp1 < exp2 (int32_t)
#define AS_LE_I32(exp1, exp2)             _ASX_2(int32_t,exp1,<=,exp2,"%d","") // Assert exp1 <= exp2 (int32_t)
#define AS_GT_I32(exp1, exp2)             _ASX_2(int32_t,exp1,> ,exp2,"%d","") // Assert exp1 > exp2 (int32_t)
#define AS_GE_I32(exp1, exp2)             _ASX_2(int32_t,exp1,>=,exp2,"%d","") // Assert exp1 >= exp2 (int32_t)
#define AS_BETWEEN_IE_I32(low, exp, high) _ASX_3(int32_t,low,<=,exp,< ,high,"%d","") // Assert low <= exp < high (int32_t)
#define AS_BETWEEN_II_I32(low, exp, high) _ASX_3(int32_t,low,<=,exp,<=,high,"%d","") // Assert low <= exp <= high (int32_t)
#define AS_BETWEEN_EI_I32(low, exp, high) _ASX_3(int32_t,low,< ,exp,<=,high,"%d","") // Assert low < exp <= high (int32_t)
#define AS_BETWEEN_EE_I32(low, exp, high) _ASX_3(int32_t,low,< ,exp,< ,high,"%d","") // Assert low < exp < high (int32_t)
#define AS_EQ_I64(exp1, exp2)             _ASX_2(int64_t,exp1,==,exp2,"%lld","") // Assert exp1 == exp2 (int64_t)
#define AS_NE_I64(exp1, exp2)             _ASX_2(int64_t,exp1,!=,exp2,"%lld","") // Assert exp1 != exp2 (int64_t)
#define AS_LT_I64(exp1, exp2)             _ASX_2(int64_t,exp1,< ,exp2,"%lld","") // Assert exp1 < exp2 (int64_t)
#define AS_LE_I64(exp1, exp2)             _ASX_2(int64_t,exp1,<=,exp2,"%lld","") // Assert exp1 <= exp2 (int64_t)
#define AS_GT_I64(exp1, exp2)             _ASX_2(int64_t,exp1,> ,exp2,"%lld","") // Assert exp1 > exp2 (int64_t)
#define AS_GE_I64(exp1, exp2)             _ASX_2(int64_t,exp1,>=,exp2,"%lld","") // Assert exp1 >= exp2 (int64_t)
#define AS_BETWEEN_IE_I64(low, exp, high) _ASX_3(int64_t,low,<=,exp,< ,high,"%lld","") // Assert low <= exp < high (int64_t)
#define AS_BETWEEN_II_I64(low, exp, high) _ASX_3(int64_t,low,<=,exp,<=,high,"%lld","") // Assert low <= exp <= high (int64_t)
#define AS_BETWEEN_EI_I64(low, exp, high) _ASX_3(int64_t,low,< ,exp,<=,high,"%lld","") // Assert low < exp <= high (int64_t)
#define AS_BETWEEN_EE_I64(low, exp, high) _ASX_3(int64_t,low,< ,exp,< ,high,"%lld","") // Assert low < exp < high (int64_t)
#define AS_EQ_U8(exp1, exp2)             _ASX_2(uint8_t,exp1,==,exp2,"%hhu","") // Assert exp1 == exp2 (uint8_t)
#define AS_NE_U8(exp1, exp2)             _ASX_2(uint8_t,exp1,!=,exp2,"%hhu","") // Assert exp1 != exp2 (uint8_t)
#define AS_LT_U8(exp1, exp2)             _ASX_2(uint8_t,exp1,< ,exp2,"%hhu","") // Assert exp1 < exp2 (uint8_t)
#define AS_LE_U8(exp1, exp2)             _ASX_2(uint8_t,exp1,<=,exp2,"%hhu","") // Assert exp1 <= exp2 (uint8_t)
#define AS_GT_U8(exp1, exp2)             _ASX_2(uint8_t,exp1,> ,exp2,"%hhu","") // Assert exp1 > exp2 (uint8_t)
#define AS_GE_U8(exp1, exp2)             _ASX_2(uint8_t,exp1,>=,exp2,"%hhu","") // Assert exp1 >= exp2 (uint8_t)
#define AS_BETWEEN_IE_U8(low, exp, high) _ASX_3(uint8_t,low,<=,exp,< ,high,"%hhu","") // Assert low <= exp < high (uint8_t)
#define AS_BETWEEN_II_U8(low, exp, high) _ASX_3(uint8_t,low,<=,exp,<=,high,"%hhu","") // Assert low <= exp <= high (uint8_t)
#define AS_BETWEEN_EI_U8(low, exp, high) _ASX_3(uint8_t,low,< ,exp,<=,high,"%hhu","") // Assert low < exp <= high (uint8_t)
#define AS_BETWEEN_EE_U8(low, exp, high) _ASX_3(uint8_t,low,< ,exp,< ,high,"%hhu","") // Assert low < exp < high (uint8_t)
#define AS_EQ_U16(exp1, exp2)             _ASX_2(uint16_t,exp1,==,exp2,"%hu","") // Assert exp1 == exp2 (uint16_t)
#define AS_NE_U16(exp1, exp2)             _ASX_2(uint16_t,exp1,!=,exp2,"%hu","") // Assert exp1 != exp2 (uint16_t)
#define AS_LT_U16(exp1, exp2)             _ASX_2(uint16_t,exp1,< ,exp2,"%hu","") // Assert exp1 < exp2 (uint16_t)
#define AS_LE_U16(exp1, exp2)             _ASX_2(uint16_t,exp1,<=,exp2,"%hu","") // Assert exp1 <= exp2 (uint16_t)
#define AS_GT_U16(exp1, exp2)             _ASX_2(uint16_t,exp1,> ,exp2,"%hu","") // Assert exp1 > exp2 (uint16_t)
#define AS_GE_U16(exp1, exp2)             _ASX_2(uint16_t,exp1,>=,exp2,"%hu","") // Assert exp1 >= exp2 (uint16_t)
#define AS_BETWEEN_IE_U16(low, exp, high) _ASX_3(uint16_t,low,<=,exp,< ,high,"%hu","") // Assert low <= exp < high (uint16_t)
#define AS_BETWEEN_II_U16(low, exp, high) _ASX_3(uint16_t,low,<=,exp,<=,high,"%hu","") // Assert low <= exp <= high (uint16_t)
#define AS_BETWEEN_EI_U16(low, exp, high) _ASX_3(uint16_t,low,< ,exp,<=,high,"%hu","") // Assert low < exp <= high (uint16_t)
#define AS_BETWEEN_EE_U16(low, exp, high) _ASX_3(uint16_t,low,< ,exp,< ,high,"%hu","") // Assert low < exp < high (uint16_t)
#define AS_EQ_U32(exp1, exp2)             _ASX_2(uint32_t,exp1,==,exp2,"%u","") // Assert exp1 == exp2 (uint32_t)
#define AS_NE_U32(exp1, exp2)             _ASX_2(uint32_t,exp1,!=,exp2,"%u","") // Assert exp1 != exp2 (uint32_t)
#define AS_LT_U32(exp1, exp2)             _ASX_2(uint32_t,exp1,< ,exp2,"%u","") // Assert exp1 < exp2 (uint32_t)
#define AS_LE_U32(exp1, exp2)             _ASX_2(uint32_t,exp1,<=,exp2,"%u","") // Assert exp1 <= exp2 (uint32_t)
#define AS_GT_U32(exp1, exp2)             _ASX_2(uint32_t,exp1,> ,exp2,"%u","") // Assert exp1 > exp2 (uint32_t)
#define AS_GE_U32(exp1, exp2)             _ASX_2(uint32_t,exp1,>=,exp2,"%u","") // Assert exp1 >= exp2 (uint32_t)
#define AS_BETWEEN_IE_U32(low, exp, high) _ASX_3(uint32_t,low,<=,exp,< ,high,"%u","") // Assert low <= exp < high (uint32_t)
#define AS_BETWEEN_II_U32(low, exp, high) _ASX_3(uint32_t,low,<=,exp,<=,high,"%u","") // Assert low <= exp <= high (uint32_t)
#define AS_BETWEEN_EI_U32(low, exp, high) _ASX_3(uint32_t,low,< ,exp,<=,high,"%u","") // Assert low < exp <= high (uint32_t)
#define AS_BETWEEN_EE_U32(low, exp, high) _ASX_3(uint32_t,low,< ,exp,< ,high,"%u","") // Assert low < exp < high (uint32_t)
#define AS_EQ_U64(exp1, exp2)             _ASX_2(uint64_t,exp1,==,exp2,"%llu","") // Assert exp1 == exp2 (uint64_t)
#define AS_NE_U64(exp1, exp2)             _ASX_2(uint64_t,exp1,!=,exp2,"%llu","") // Assert exp1 != exp2 (uint64_t)
#define AS_LT_U64(exp1, exp2)             _ASX_2(uint64_t,exp1,< ,exp2,"%llu","") // Assert exp1 < exp2 (uint64_t)
#define AS_LE_U64(exp1, exp2)             _ASX_2(uint64_t,exp1,<=,exp2,"%llu","") // Assert exp1 <= exp2 (uint64_t)
#define AS_GT_U64(exp1, exp2)             _ASX_2(uint64_t,exp1,> ,exp2,"%llu","") // Assert exp1 > exp2 (uint64_t)
#define AS_GE_U64(exp1, exp2)             _ASX_2(uint64_t,exp1,>=,exp2,"%llu","") // Assert exp1 >= exp2 (uint64_t)
#define AS_BETWEEN_IE_U64(low, exp, high) _ASX_3(uint64_t,low,<=,exp,< ,high,"%llu","") // Assert low <= exp < high (uint64_t)
#define AS_BETWEEN_II_U64(low, exp, high) _ASX_3(uint64_t,low,<=,exp,<=,high,"%llu","") // Assert low <= exp <= high (uint64_t)
#define AS_BETWEEN_EI_U64(low, exp, high) _ASX_3(uint64_t,low,< ,exp,<=,high,"%llu","") // Assert low < exp <= high (uint64_t)
#define AS_BETWEEN_EE_U64(low, exp, high) _ASX_3(uint64_t,low,< ,exp,< ,high,"%llu","") // Assert low < exp < high (uint64_t)
#define AS_EQ_SIZET(exp1, exp2)             _ASX_2(size_t,exp1,==,exp2,"%zu","") // Assert exp1 == exp2 (size_t)
#define AS_NE_SIZET(exp1, exp2)             _ASX_2(size_t,exp1,!=,exp2,"%zu","") // Assert exp1 != exp2 (size_t)
#define AS_LT_SIZET(exp1, exp2)             _ASX_2(size_t,exp1,< ,exp2,"%zu","") // Assert exp1 < exp2 (size_t)
#define AS_LE_SIZET(exp1, exp2)             _ASX_2(size_t,exp1,<=,exp2,"%zu","") // Assert exp1 <= exp2 (size_t)
#define AS_GT_SIZET(exp1, exp2)             _ASX_2(size_t,exp1,> ,exp2,"%zu","") // Assert exp1 > exp2 (size_t)
#define AS_GE_SIZET(exp1, exp2)             _ASX_2(size_t,exp1,>=,exp2,"%zu","") // Assert exp1 >= exp2 (size_t)
#define AS_BETWEEN_IE_SIZET(low, exp, high) _ASX_3(size_t,low,<=,exp,< ,high,"%zu","") // Assert low <= exp < high (size_t)
#define AS_BETWEEN_II_SIZET(low, exp, high) _ASX_3(size_t,low,<=,exp,<=,high,"%zu","") // Assert low <= exp <= high (size_t)
#define AS_BETWEEN_EI_SIZET(low, exp, high) _ASX_3(size_t,low,< ,exp,<=,high,"%zu","") // Assert low < exp <= high (size_t)
#define AS_BETWEEN_EE_SIZET(low, exp, high) _ASX_3(size_t,low,< ,exp,< ,high,"%zu","") // Assert low < exp < high (size_t)
#define AS_EQ_SSIZET(exp1, exp2)             _ASX_2(ssize_t,exp1,==,exp2,"%zd","") // Assert exp1 == exp2 (ssize_t)
#define AS_NE_SSIZET(exp1, exp2)             _ASX_2(ssize_t,exp1,!=,exp2,"%zd","") // Assert exp1 != exp2 (ssize_t)
#define AS_LT_SSIZET(exp1, exp2)             _ASX_2(ssize_t,exp1,< ,exp2,"%zd","") // Assert exp1 < exp2 (ssize_t)
#define AS_LE_SSIZET(exp1, exp2)             _ASX_2(ssize_t,exp1,<=,exp2,"%zd","") // Assert exp1 <= exp2 (ssize_t)
#define AS_GT_SSIZET(exp1, exp2)             _ASX_2(ssize_t,exp1,> ,exp2,"%zd","") // Assert exp1 > exp2 (ssize_t)
#define AS_GE_SSIZET(exp1, exp2)             _ASX_2(ssize_t,exp1,>=,exp2,"%zd","") // Assert exp1 >= exp2 (ssize_t)
#define AS_BETWEEN_IE_SSIZET(low, exp, high) _ASX_3(ssize_t,low,<=,exp,< ,high,"%zd","") // Assert low <= exp < high (ssize_t)
#define AS_BETWEEN_II_SSIZET(low, exp, high) _ASX_3(ssize_t,low,<=,exp,<=,high,"%zd","") // Assert low <= exp <= high (ssize_t)
#define AS_BETWEEN_EI_SSIZET(low, exp, high) _ASX_3(ssize_t,low,< ,exp,<=,high,"%zd","") // Assert low < exp <= high (ssize_t)
#define AS_BETWEEN_EE_SSIZET(low, exp, high) _ASX_3(ssize_t,low,< ,exp,< ,high,"%zd","") // Assert low < exp < high (ssize_t)
#define AS_EQ_F32(exp1, exp2)             _ASX_2(float,exp1,==,exp2,"%f","") // Assert exp1 == exp2 (float)
#define AS_NE_F32(exp1, exp2)             _ASX_2(float,exp1,!=,exp2,"%f","") // Assert exp1 != exp2 (float)
#define AS_LT_F32(exp1, exp2)             _ASX_2(float,exp1,< ,exp2,"%f","") // Assert exp1 < exp2 (float)
#define AS_LE_F32(exp1, exp2)             _ASX_2(float,exp1,<=,exp2,"%f","") // Assert exp1 <= exp2 (float)
#define AS_GT_F32(exp1, exp2)             _ASX_2(float,exp1,> ,exp2,"%f","") // Assert exp1 > exp2 (float)
#define AS_GE_F32(exp1, exp2)             _ASX_2(float,exp1,>=,exp2,"%f","") // Assert exp1 >= exp2 (float)
#define AS_BETWEEN_IE_F32(low, exp, high) _ASX_3(float,low,<=,exp,< ,high,"%f","") // Assert low <= exp < high (float)
#define AS_BETWEEN_II_F32(low, exp, high) _ASX_3(float,low,<=,exp,<=,high,"%f","") // Assert low <= exp <= high (float)
#define AS_BETWEEN_EI_F32(low, exp, high) _ASX_3(float,low,< ,exp,<=,high,"%f","") // Assert low < exp <= high (float)
#define AS_BETWEEN_EE_F32(low, exp, high) _ASX_3(float,low,< ,exp,< ,high,"%f","") // Assert low < exp < high (float)
#define AS_EQ_F64(exp1, exp2)             _ASX_2(double,exp1,==,exp2,"%f","") // Assert exp1 == exp2 (double)
#define AS_NE_F64(exp1, exp2)             _ASX_2(double,exp1,!=,exp2,"%f","") // Assert exp1 != exp2 (double)
#define AS_LT_F64(exp1, exp2)             _ASX_2(double,exp1,< ,exp2,"%f","") // Assert exp1 < exp2 (double)
#define AS_LE_F64(exp1, exp2)             _ASX_2(double,exp1,<=,exp2,"%f","") // Assert exp1 <= exp2 (double)
#define AS_GT_F64(exp1, exp2)             _ASX_2(double,exp1,> ,exp2,"%f","") // Assert exp1 > exp2 (double)
#define AS_GE_F64(exp1, exp2)             _ASX_2(double,exp1,>=,exp2,"%f","") // Assert exp1 >= exp2 (double)
#define AS_BETWEEN_IE_F64(low, exp, high) _ASX_3(double,low,<=,exp,< ,high,"%f","") // Assert low <= exp < high (double)
#define AS_BETWEEN_II_F64(low, exp, high) _ASX_3(double,low,<=,exp,<=,high,"%f","") // Assert low <= exp <= high (double)
#define AS_BETWEEN_EI_F64(low, exp, high) _ASX_3(double,low,< ,exp,<=,high,"%f","") // Assert low < exp <= high (double)
#define AS_BETWEEN_EE_F64(low, exp, high) _ASX_3(double,low,< ,exp,< ,high,"%f","") // Assert low < exp < high (double)
#define AS_EQ_BOOL(exp1, exp2)             _ASX_2(bool,exp1,==,exp2,"%d","") // Assert exp1 == exp2 (bool)
#define AS_NE_BOOL(exp1, exp2)             _ASX_2(bool,exp1,!=,exp2,"%d","") // Assert exp1 != exp2 (bool)
#define AS_LT_BOOL(exp1, exp2)             _ASX_2(bool,exp1,< ,exp2,"%d","") // Assert exp1 < exp2 (bool)
#define AS_LE_BOOL(exp1, exp2)             _ASX_2(bool,exp1,<=,exp2,"%d","") // Assert exp1 <= exp2 (bool)
#define AS_GT_BOOL(exp1, exp2)             _ASX_2(bool,exp1,> ,exp2,"%d","") // Assert exp1 > exp2 (bool)
#define AS_GE_BOOL(exp1, exp2)             _ASX_2(bool,exp1,>=,exp2,"%d","") // Assert exp1 >= exp2 (bool)
#define AS_BETWEEN_IE_BOOL(low, exp, high) _ASX_3(bool,low,<=,exp,< ,high,"%d","") // Assert low <= exp < high (bool)
#define AS_BETWEEN_II_BOOL(low, exp, high) _ASX_3(bool,low,<=,exp,<=,high,"%d","") // Assert low <= exp <= high (bool)
#define AS_BETWEEN_EI_BOOL(low, exp, high) _ASX_3(bool,low,< ,exp,<=,high,"%d","") // Assert low < exp <= high (bool)
#define AS_BETWEEN_EE_BOOL(low, exp, high) _ASX_3(bool,low,< ,exp,< ,high,"%d","") // Assert low < exp < high (bool)
#define AS_EQ_CHAR(exp1, exp2)             _ASX_2(char,exp1,==,exp2,"%c","") // Assert exp1 == exp2 (char)
#define AS_NE_CHAR(exp1, exp2)             _ASX_2(char,exp1,!=,exp2,"%c","") // Assert exp1 != exp2 (char)
#define AS_LT_CHAR(exp1, exp2)             _ASX_2(char,exp1,< ,exp2,"%c","") // Assert exp1 < exp2 (char)
#define AS_LE_CHAR(exp1, exp2)             _ASX_2(char,exp1,<=,exp2,"%c","") // Assert exp1 <= exp2 (char)
#define AS_GT_CHAR(exp1, exp2)             _ASX_2(char,exp1,> ,exp2,"%c","") // Assert exp1 > exp2 (char)
#define AS_GE_CHAR(exp1, exp2)             _ASX_2(char,exp1,>=,exp2,"%c","") // Assert exp1 >= exp2 (char)
#define AS_BETWEEN_IE_CHAR(low, exp, high) _ASX_3(char,low,<=,exp,< ,high,"%c","") // Assert low <= exp < high (char)
#define AS_BETWEEN_II_CHAR(low, exp, high) _ASX_3(char,low,<=,exp,<=,high,"%c","") // Assert low <= exp <= high (char)
#define AS_BETWEEN_EI_CHAR(low, exp, high) _ASX_3(char,low,< ,exp,<=,high,"%c","") // Assert low < exp <= high (char)
#define AS_BETWEEN_EE_CHAR(low, exp, high) _ASX_3(char,low,< ,exp,< ,high,"%c","") // Assert low < exp < high (char)
#define AS_EQ_SHORT(exp1, exp2)             _ASX_2(short,exp1,==,exp2,"%hd","") // Assert exp1 == exp2 (short)
#define AS_NE_SHORT(exp1, exp2)             _ASX_2(short,exp1,!=,exp2,"%hd","") // Assert exp1 != exp2 (short)
#define AS_LT_SHORT(exp1, exp2)             _ASX_2(short,exp1,< ,exp2,"%hd","") // Assert exp1 < exp2 (short)
#define AS_LE_SHORT(exp1, exp2)             _ASX_2(short,exp1,<=,exp2,"%hd","") // Assert exp1 <= exp2 (short)
#define AS_GT_SHORT(exp1, exp2)             _ASX_2(short,exp1,> ,exp2,"%hd","") // Assert exp1 > exp2 (short)
#define AS_GE_SHORT(exp1, exp2)             _ASX_2(short,exp1,>=,exp2,"%hd","") // Assert exp1 >= exp2 (short)
#define AS_BETWEEN_IE_SHORT(low, exp, high) _ASX_3(short,low,<=,exp,< ,high,"%hd","") // Assert low <= exp < high (short)
#define AS_BETWEEN_II_SHORT(low, exp, high) _ASX_3(short,low,<=,exp,<=,high,"%hd","") // Assert low <= exp <= high (short)
#define AS_BETWEEN_EI_SHORT(low, exp, high) _ASX_3(short,low,< ,exp,<=,high,"%hd","") // Assert low < exp <= high (short)
#define AS_BETWEEN_EE_SHORT(low, exp, high) _ASX_3(short,low,< ,exp,< ,high,"%hd","") // Assert low < exp < high (short)
#define AS_EQ_INT(exp1, exp2)             _ASX_2(int,exp1,==,exp2,"%d","") // Assert exp1 == exp2 (int)
#define AS_NE_INT(exp1, exp2)             _ASX_2(int,exp1,!=,exp2,"%d","") // Assert exp1 != exp2 (int)
#define AS_LT_INT(exp1, exp2)             _ASX_2(int,exp1,< ,exp2,"%d","") // Assert exp1 < exp2 (int)
#define AS_LE_INT(exp1, exp2)             _ASX_2(int,exp1,<=,exp2,"%d","") // Assert exp1 <= exp2 (int)
#define AS_GT_INT(exp1, exp2)             _ASX_2(int,exp1,> ,exp2,"%d","") // Assert exp1 > exp2 (int)
#define AS_GE_INT(exp1, exp2)             _ASX_2(int,exp1,>=,exp2,"%d","") // Assert exp1 >= exp2 (int)
#define AS_BETWEEN_IE_INT(low, exp, high) _ASX_3(int,low,<=,exp,< ,high,"%d","") // Assert low <= exp < high (int)
#define AS_BETWEEN_II_INT(low, exp, high) _ASX_3(int,low,<=,exp,<=,high,"%d","") // Assert low <= exp <= high (int)
#define AS_BETWEEN_EI_INT(low, exp, high) _ASX_3(int,low,< ,exp,<=,high,"%d","") // Assert low < exp <= high (int)
#define AS_BETWEEN_EE_INT(low, exp, high) _ASX_3(int,low,< ,exp,< ,high,"%d","") // Assert low < exp < high (int)
#define AS_EQ_LONG(exp1, exp2)             _ASX_2(long,exp1,==,exp2,"%ld","") // Assert exp1 == exp2 (long)
#define AS_NE_LONG(exp1, exp2)             _ASX_2(long,exp1,!=,exp2,"%ld","") // Assert exp1 != exp2 (long)
#define AS_LT_LONG(exp1, exp2)             _ASX_2(long,exp1,< ,exp2,"%ld","") // Assert exp1 < exp2 (long)
#define AS_LE_LONG(exp1, exp2)             _ASX_2(long,exp1,<=,exp2,"%ld","") // Assert exp1 <= exp2 (long)
#define AS_GT_LONG(exp1, exp2)             _ASX_2(long,exp1,> ,exp2,"%ld","") // Assert exp1 > exp2 (long)
#define AS_GE_LONG(exp1, exp2)             _ASX_2(long,exp1,>=,exp2,"%ld","") // Assert exp1 >= exp2 (long)
#define AS_BETWEEN_IE_LONG(low, exp, high) _ASX_3(long,low,<=,exp,< ,high,"%ld","") // Assert low <= exp < high (long)
#define AS_BETWEEN_II_LONG(low, exp, high) _ASX_3(long,low,<=,exp,<=,high,"%ld","") // Assert low <= exp <= high (long)
#define AS_BETWEEN_EI_LONG(low, exp, high) _ASX_3(long,low,< ,exp,<=,high,"%ld","") // Assert low < exp <= high (long)
#define AS_BETWEEN_EE_LONG(low, exp, high) _ASX_3(long,low,< ,exp,< ,high,"%ld","") // Assert low < exp < high (long)
#define AS_EQ_LLONG(exp1, exp2)             _ASX_2(long long,exp1,==,exp2,"%lld","") // Assert exp1 == exp2 (long long)
#define AS_NE_LLONG(exp1, exp2)             _ASX_2(long long,exp1,!=,exp2,"%lld","") // Assert exp1 != exp2 (long long)
#define AS_LT_LLONG(exp1, exp2)             _ASX_2(long long,exp1,< ,exp2,"%lld","") // Assert exp1 < exp2 (long long)
#define AS_LE_LLONG(exp1, exp2)             _ASX_2(long long,exp1,<=,exp2,"%lld","") // Assert exp1 <= exp2 (long long)
#define AS_GT_LLONG(exp1, exp2)             _ASX_2(long long,exp1,> ,exp2,"%lld","") // Assert exp1 > exp2 (long long)
#define AS_GE_LLONG(exp1, exp2)             _ASX_2(long long,exp1,>=,exp2,"%lld","") // Assert exp1 >= exp2 (long long)
#define AS_BETWEEN_IE_LLONG(low, exp, high) _ASX_3(long long,low,<=,exp,< ,high,"%lld","") // Assert low <= exp < high (long long)
#define AS_BETWEEN_II_LLONG(low, exp, high) _ASX_3(long long,low,<=,exp,<=,high,"%lld","") // Assert low <= exp <= high (long long)
#define AS_BETWEEN_EI_LLONG(low, exp, high) _ASX_3(long long,low,< ,exp,<=,high,"%lld","") // Assert low < exp <= high (long long)
#define AS_BETWEEN_EE_LLONG(low, exp, high) _ASX_3(long long,low,< ,exp,< ,high,"%lld","") // Assert low < exp < high (long long)
#define AS_EQ_UCHAR(exp1, exp2)             _ASX_2(unsigned char,exp1,==,exp2,"%c","") // Assert exp1 == exp2 (unsigned char)
#define AS_NE_UCHAR(exp1, exp2)             _ASX_2(unsigned char,exp1,!=,exp2,"%c","") // Assert exp1 != exp2 (unsigned char)
#define AS_LT_UCHAR(exp1, exp2)             _ASX_2(unsigned char,exp1,< ,exp2,"%c","") // Assert exp1 < exp2 (unsigned char)
#define AS_LE_UCHAR(exp1, exp2)             _ASX_2(unsigned char,exp1,<=,exp2,"%c","") // Assert exp1 <= exp2 (unsigned char)
#define AS_GT_UCHAR(exp1, exp2)             _ASX_2(unsigned char,exp1,> ,exp2,"%c","") // Assert exp1 > exp2 (unsigned char)
#define AS_GE_UCHAR(exp1, exp2)             _ASX_2(unsigned char,exp1,>=,exp2,"%c","") // Assert exp1 >= exp2 (unsigned char)
#define AS_BETWEEN_IE_UCHAR(low, exp, high) _ASX_3(unsigned char,low,<=,exp,< ,high,"%c","") // Assert low <= exp < high (unsigned char)
#define AS_BETWEEN_II_UCHAR(low, exp, high) _ASX_3(unsigned char,low,<=,exp,<=,high,"%c","") // Assert low <= exp <= high (unsigned char)
#define AS_BETWEEN_EI_UCHAR(low, exp, high) _ASX_3(unsigned char,low,< ,exp,<=,high,"%c","") // Assert low < exp <= high (unsigned char)
#define AS_BETWEEN_EE_UCHAR(low, exp, high) _ASX_3(unsigned char,low,< ,exp,< ,high,"%c","") // Assert low < exp < high (unsigned char)
#define AS_EQ_USHORT(exp1, exp2)             _ASX_2(unsigned short,exp1,==,exp2,"%hu","") // Assert exp1 == exp2 (unsigned short)
#define AS_NE_USHORT(exp1, exp2)             _ASX_2(unsigned short,exp1,!=,exp2,"%hu","") // Assert exp1 != exp2 (unsigned short)
#define AS_LT_USHORT(exp1, exp2)             _ASX_2(unsigned short,exp1,< ,exp2,"%hu","") // Assert exp1 < exp2 (unsigned short)
#define AS_LE_USHORT(exp1, exp2)             _ASX_2(unsigned short,exp1,<=,exp2,"%hu","") // Assert exp1 <= exp2 (unsigned short)
#define AS_GT_USHORT(exp1, exp2)             _ASX_2(unsigned short,exp1,> ,exp2,"%hu","") // Assert exp1 > exp2 (unsigned short)
#define AS_GE_USHORT(exp1, exp2)             _ASX_2(unsigned short,exp1,>=,exp2,"%hu","") // Assert exp1 >= exp2 (unsigned short)
#define AS_BETWEEN_IE_USHORT(low, exp, high) _ASX_3(unsigned short,low,<=,exp,< ,high,"%hu","") // Assert low <= exp < high (unsigned short)
#define AS_BETWEEN_II_USHORT(low, exp, high) _ASX_3(unsigned short,low,<=,exp,<=,high,"%hu","") // Assert low <= exp <= high (unsigned short)
#define AS_BETWEEN_EI_USHORT(low, exp, high) _ASX_3(unsigned short,low,< ,exp,<=,high,"%hu","") // Assert low < exp <= high (unsigned short)
#define AS_BETWEEN_EE_USHORT(low, exp, high) _ASX_3(unsigned short,low,< ,exp,< ,high,"%hu","") // Assert low < exp < high (unsigned short)
#define AS_EQ_UINT(exp1, exp2)             _ASX_2(unsigned int,exp1,==,exp2,"%u","") // Assert exp1 == exp2 (unsigned int)
#define AS_NE_UINT(exp1, exp2)             _ASX_2(unsigned int,exp1,!=,exp2,"%u","") // Assert exp1 != exp2 (unsigned int)
#define AS_LT_UINT(exp1, exp2)             _ASX_2(unsigned int,exp1,< ,exp2,"%u","") // Assert exp1 < exp2 (unsigned int)
#define AS_LE_UINT(exp1, exp2)             _ASX_2(unsigned int,exp1,<=,exp2,"%u","") // Assert exp1 <= exp2 (unsigned int)
#define AS_GT_UINT(exp1, exp2)             _ASX_2(unsigned int,exp1,> ,exp2,"%u","") // Assert exp1 > exp2 (unsigned int)
#define AS_GE_UINT(exp1, exp2)             _ASX_2(unsigned int,exp1,>=,exp2,"%u","") // Assert exp1 >= exp2 (unsigned int)
#define AS_BETWEEN_IE_UINT(low, exp, high) _ASX_3(unsigned int,low,<=,exp,< ,high,"%u","") // Assert low <= exp < high (unsigned int)
#define AS_BETWEEN_II_UINT(low, exp, high) _ASX_3(unsigned int,low,<=,exp,<=,high,"%u","") // Assert low <= exp <= high (unsigned int)
#define AS_BETWEEN_EI_UINT(low, exp, high) _ASX_3(unsigned int,low,< ,exp,<=,high,"%u","") // Assert low < exp <= high (unsigned int)
#define AS_BETWEEN_EE_UINT(low, exp, high) _ASX_3(unsigned int,low,< ,exp,< ,high,"%u","") // Assert low < exp < high (unsigned int)
#define AS_EQ_ULONG(exp1, exp2)             _ASX_2(unsigned long,exp1,==,exp2,"%lu","") // Assert exp1 == exp2 (unsigned long)
#define AS_NE_ULONG(exp1, exp2)             _ASX_2(unsigned long,exp1,!=,exp2,"%lu","") // Assert exp1 != exp2 (unsigned long)
#define AS_LT_ULONG(exp1, exp2)             _ASX_2(unsigned long,exp1,< ,exp2,"%lu","") // Assert exp1 < exp2 (unsigned long)
#define AS_LE_ULONG(exp1, exp2)             _ASX_2(unsigned long,exp1,<=,exp2,"%lu","") // Assert exp1 <= exp2 (unsigned long)
#define AS_GT_ULONG(exp1, exp2)             _ASX_2(unsigned long,exp1,> ,exp2,"%lu","") // Assert exp1 > exp2 (unsigned long)
#define AS_GE_ULONG(exp1, exp2)             _ASX_2(unsigned long,exp1,>=,exp2,"%lu","") // Assert exp1 >= exp2 (unsigned long)
#define AS_BETWEEN_IE_ULONG(low, exp, high) _ASX_3(unsigned long,low,<=,exp,< ,high,"%lu","") // Assert low <= exp < high (unsigned long)
#define AS_BETWEEN_II_ULONG(low, exp, high) _ASX_3(unsigned long,low,<=,exp,<=,high,"%lu","") // Assert low <= exp <= high (unsigned long)
#define AS_BETWEEN_EI_ULONG(low, exp, high) _ASX_3(unsigned long,low,< ,exp,<=,high,"%lu","") // Assert low < exp <= high (unsigned long)
#define AS_BETWEEN_EE_ULONG(low, exp, high) _ASX_3(unsigned long,low,< ,exp,< ,high,"%lu","") // Assert low < exp < high (unsigned long)
#define AS_EQ_ULLONG(exp1, exp2)             _ASX_2(unsigned long long,exp1,==,exp2,"%llu","") // Assert exp1 == exp2 (unsigned long long)
#define AS_NE_ULLONG(exp1, exp2)             _ASX_2(unsigned long long,exp1,!=,exp2,"%llu","") // Assert exp1 != exp2 (unsigned long long)
#define AS_LT_ULLONG(exp1, exp2)             _ASX_2(unsigned long long,exp1,< ,exp2,"%llu","") // Assert exp1 < exp2 (unsigned long long)
#define AS_LE_ULLONG(exp1, exp2)             _ASX_2(unsigned long long,exp1,<=,exp2,"%llu","") // Assert exp1 <= exp2 (unsigned long long)
#define AS_GT_ULLONG(exp1, exp2)             _ASX_2(unsigned long long,exp1,> ,exp2,"%llu","") // Assert exp1 > exp2 (unsigned long long)
#define AS_GE_ULLONG(exp1, exp2)             _ASX_2(unsigned long long,exp1,>=,exp2,"%llu","") // Assert exp1 >= exp2 (unsigned long long)
#define AS_BETWEEN_IE_ULLONG(low, exp, high) _ASX_3(unsigned long long,low,<=,exp,< ,high,"%llu","") // Assert low <= exp < high (unsigned long long)
#define AS_BETWEEN_II_ULLONG(low, exp, high) _ASX_3(unsigned long long,low,<=,exp,<=,high,"%llu","") // Assert low <= exp <= high (unsigned long long)
#define AS_BETWEEN_EI_ULLONG(low, exp, high) _ASX_3(unsigned long long,low,< ,exp,<=,high,"%llu","") // Assert low < exp <= high (unsigned long long)
#define AS_BETWEEN_EE_ULLONG(low, exp, high) _ASX_3(unsigned long long,low,< ,exp,< ,high,"%llu","") // Assert low < exp < high (unsigned long long)
#define AS_EQ_FLOAT(exp1, exp2)             _ASX_2(float,exp1,==,exp2,"%f","") // Assert exp1 == exp2 (float)
#define AS_NE_FLOAT(exp1, exp2)             _ASX_2(float,exp1,!=,exp2,"%f","") // Assert exp1 != exp2 (float)
#define AS_LT_FLOAT(exp1, exp2)             _ASX_2(float,exp1,< ,exp2,"%f","") // Assert exp1 < exp2 (float)
#define AS_LE_FLOAT(exp1, exp2)             _ASX_2(float,exp1,<=,exp2,"%f","") // Assert exp1 <= exp2 (float)
#define AS_GT_FLOAT(exp1, exp2)             _ASX_2(float,exp1,> ,exp2,"%f","") // Assert exp1 > exp2 (float)
#define AS_GE_FLOAT(exp1, exp2)             _ASX_2(float,exp1,>=,exp2,"%f","") // Assert exp1 >= exp2 (float)
#define AS_BETWEEN_IE_FLOAT(low, exp, high) _ASX_3(float,low,<=,exp,< ,high,"%f","") // Assert low <= exp < high (float)
#define AS_BETWEEN_II_FLOAT(low, exp, high) _ASX_3(float,low,<=,exp,<=,high,"%f","") // Assert low <= exp <= high (float)
#define AS_BETWEEN_EI_FLOAT(low, exp, high) _ASX_3(float,low,< ,exp,<=,high,"%f","") // Assert low < exp <= high (float)
#define AS_BETWEEN_EE_FLOAT(low, exp, high) _ASX_3(float,low,< ,exp,< ,high,"%f","") // Assert low < exp < high (float)
#define AS_EQ_DOUBLE(exp1, exp2)             _ASX_2(double,exp1,==,exp2,"%lf","") // Assert exp1 == exp2 (double)
#define AS_NE_DOUBLE(exp1, exp2)             _ASX_2(double,exp1,!=,exp2,"%lf","") // Assert exp1 != exp2 (double)
#define AS_LT_DOUBLE(exp1, exp2)             _ASX_2(double,exp1,< ,exp2,"%lf","") // Assert exp1 < exp2 (double)
#define AS_LE_DOUBLE(exp1, exp2)             _ASX_2(double,exp1,<=,exp2,"%lf","") // Assert exp1 <= exp2 (double)
#define AS_GT_DOUBLE(exp1, exp2)             _ASX_2(double,exp1,> ,exp2,"%lf","") // Assert exp1 > exp2 (double)
#define AS_GE_DOUBLE(exp1, exp2)             _ASX_2(double,exp1,>=,exp2,"%lf","") // Assert exp1 >= exp2 (double)
#define AS_BETWEEN_IE_DOUBLE(low, exp, high) _ASX_3(double,low,<=,exp,< ,high,"%lf","") // Assert low <= exp < high (double)
#define AS_BETWEEN_II_DOUBLE(low, exp, high) _ASX_3(double,low,<=,exp,<=,high,"%lf","") // Assert low <= exp <= high (double)
#define AS_BETWEEN_EI_DOUBLE(low, exp, high) _ASX_3(double,low,< ,exp,<=,high,"%lf","") // Assert low < exp <= high (double)
#define AS_BETWEEN_EE_DOUBLE(low, exp, high) _ASX_3(double,low,< ,exp,< ,high,"%lf","") // Assert low < exp < high (double)
#define AS_EQ_LDOUBLE(exp1, exp2)             _ASX_2(long double,exp1,==,exp2,"%Lf","") // Assert exp1 == exp2 (long double)
#define AS_NE_LDOUBLE(exp1, exp2)             _ASX_2(long double,exp1,!=,exp2,"%Lf","") // Assert exp1 != exp2 (long double)
#define AS_LT_LDOUBLE(exp1, exp2)             _ASX_2(long double,exp1,< ,exp2,"%Lf","") // Assert exp1 < exp2 (long double)
#define AS_LE_LDOUBLE(exp1, exp2)             _ASX_2(long double,exp1,<=,exp2,"%Lf","") // Assert exp1 <= exp2 (long double)
#define AS_GT_LDOUBLE(exp1, exp2)             _ASX_2(long double,exp1,> ,exp2,"%Lf","") // Assert exp1 > exp2 (long double)
#define AS_GE_LDOUBLE(exp1, exp2)             _ASX_2(long double,exp1,>=,exp2,"%Lf","") // Assert exp1 >= exp2 (long double)
#define AS_BETWEEN_IE_LDOUBLE(low, exp, high) _ASX_3(long double,low,<=,exp,< ,high,"%Lf","") // Assert low <= exp < high (long double)
#define AS_BETWEEN_II_LDOUBLE(low, exp, high) _ASX_3(long double,low,<=,exp,<=,high,"%Lf","") // Assert low <= exp <= high (long double)
#define AS_BETWEEN_EI_LDOUBLE(low, exp, high) _ASX_3(long double,low,< ,exp,<=,high,"%Lf","") // Assert low < exp <= high (long double)
#define AS_BETWEEN_EE_LDOUBLE(low, exp, high) _ASX_3(long double,low,< ,exp,< ,high,"%Lf","") // Assert low < exp < high (long double)
#define AS_EQ_PTR(exp1, exp2)             _ASX_2(void *,exp1,==,exp2,"%p","") // Assert exp1 == exp2 (void *)
#define AS_NE_PTR(exp1, exp2)             _ASX_2(void *,exp1,!=,exp2,"%p","") // Assert exp1 != exp2 (void *)
#define AS_LT_PTR(exp1, exp2)             _ASX_2(void *,exp1,< ,exp2,"%p","") // Assert exp1 < exp2 (void *)
#define AS_LE_PTR(exp1, exp2)             _ASX_2(void *,exp1,<=,exp2,"%p","") // Assert exp1 <= exp2 (void *)
#define AS_GT_PTR(exp1, exp2)             _ASX_2(void *,exp1,> ,exp2,"%p","") // Assert exp1 > exp2 (void *)
#define AS_GE_PTR(exp1, exp2)             _ASX_2(void *,exp1,>=,exp2,"%p","") // Assert exp1 >= exp2 (void *)
#define AS_BETWEEN_IE_PTR(low, exp, high) _ASX_3(void *,low,<=,exp,< ,high,"%p","") // Assert low <= exp < high (void *)
#define AS_BETWEEN_II_PTR(low, exp, high) _ASX_3(void *,low,<=,exp,<=,high,"%p","") // Assert low <= exp <= high (void *)
#define AS_BETWEEN_EI_PTR(low, exp, high) _ASX_3(void *,low,< ,exp,<=,high,"%p","") // Assert low < exp <= high (void *)
#define AS_BETWEEN_EE_PTR(low, exp, high) _ASX_3(void *,low,< ,exp,< ,high,"%p","") // Assert low < exp < high (void *)
#define ASX_EQ_I8(exp1, exp2, fmt, ...)             _ASX_2(int8_t,exp1,==,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int8_t)
#define ASX_NE_I8(exp1, exp2, fmt, ...)             _ASX_2(int8_t,exp1,!=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int8_t)
#define ASX_LT_I8(exp1, exp2, fmt, ...)             _ASX_2(int8_t,exp1,< ,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int8_t)
#define ASX_LE_I8(exp1, exp2, fmt, ...)             _ASX_2(int8_t,exp1,<=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int8_t)
#define ASX_GT_I8(exp1, exp2, fmt, ...)             _ASX_2(int8_t,exp1,> ,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int8_t)
#define ASX_GE_I8(exp1, exp2, fmt, ...)             _ASX_2(int8_t,exp1,>=,exp2,"%hhd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int8_t)
#define ASX_BETWEEN_IE_I8(low, exp, high, fmt, ...) _ASX_3(int8_t,low,<=,exp,< ,high,"%hhd",fmt,##__VA_ARGS__) // Assert low <= exp < high (int8_t)
#define ASX_BETWEEN_II_I8(low, exp, high, fmt, ...) _ASX_3(int8_t,low,<=,exp,<=,high,"%hhd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int8_t)
#define ASX_BETWEEN_EI_I8(low, exp, high, fmt, ...) _ASX_3(int8_t,low,< ,exp,<=,high,"%hhd",fmt,##__VA_ARGS__) // Assert low < exp <= high (int8_t)
#define ASX_BETWEEN_EE_I8(low, exp, high, fmt, ...) _ASX_3(int8_t,low,< ,exp,< ,high,"%hhd",fmt,##__VA_ARGS__) // Assert low < exp < high (int8_t)
#define ASX_EQ_I16(exp1, exp2, fmt, ...)             _ASX_2(int16_t,exp1,==,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int16_t)
#define ASX_NE_I16(exp1, exp2, fmt, ...)             _ASX_2(int16_t,exp1,!=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int16_t)
#define ASX_LT_I16(exp1, exp2, fmt, ...)             _ASX_2(int16_t,exp1,< ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int16_t)
#define ASX_LE_I16(exp1, exp2, fmt, ...)             _ASX_2(int16_t,exp1,<=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int16_t)
#define ASX_GT_I16(exp1, exp2, fmt, ...)             _ASX_2(int16_t,exp1,> ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int16_t)
#define ASX_GE_I16(exp1, exp2, fmt, ...)             _ASX_2(int16_t,exp1,>=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int16_t)
#define ASX_BETWEEN_IE_I16(low, exp, high, fmt, ...) _ASX_3(int16_t,low,<=,exp,< ,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp < high (int16_t)
#define ASX_BETWEEN_II_I16(low, exp, high, fmt, ...) _ASX_3(int16_t,low,<=,exp,<=,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int16_t)
#define ASX_BETWEEN_EI_I16(low, exp, high, fmt, ...) _ASX_3(int16_t,low,< ,exp,<=,high,"%hd",fmt,##__VA_ARGS__) // Assert low < exp <= high (int16_t)
#define ASX_BETWEEN_EE_I16(low, exp, high, fmt, ...) _ASX_3(int16_t,low,< ,exp,< ,high,"%hd",fmt,##__VA_ARGS__) // Assert low < exp < high (int16_t)
#define ASX_EQ_I32(exp1, exp2, fmt, ...)             _ASX_2(int32_t,exp1,==,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int32_t)
#define ASX_NE_I32(exp1, exp2, fmt, ...)             _ASX_2(int32_t,exp1,!=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int32_t)
#define ASX_LT_I32(exp1, exp2, fmt, ...)             _ASX_2(int32_t,exp1,< ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int32_t)
#define ASX_LE_I32(exp1, exp2, fmt, ...)             _ASX_2(int32_t,exp1,<=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int32_t)
#define ASX_GT_I32(exp1, exp2, fmt, ...)             _ASX_2(int32_t,exp1,> ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int32_t)
#define ASX_GE_I32(exp1, exp2, fmt, ...)             _ASX_2(int32_t,exp1,>=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int32_t)
#define ASX_BETWEEN_IE_I32(low, exp, high, fmt, ...) _ASX_3(int32_t,low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp < high (int32_t)
#define ASX_BETWEEN_II_I32(low, exp, high, fmt, ...) _ASX_3(int32_t,low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int32_t)
#define ASX_BETWEEN_EI_I32(low, exp, high, fmt, ...) _ASX_3(int32_t,low,< ,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low < exp <= high (int32_t)
#define ASX_BETWEEN_EE_I32(low, exp, high, fmt, ...) _ASX_3(int32_t,low,< ,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low < exp < high (int32_t)
#define ASX_EQ_I64(exp1, exp2, fmt, ...)             _ASX_2(int64_t,exp1,==,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int64_t)
#define ASX_NE_I64(exp1, exp2, fmt, ...)             _ASX_2(int64_t,exp1,!=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int64_t)
#define ASX_LT_I64(exp1, exp2, fmt, ...)             _ASX_2(int64_t,exp1,< ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int64_t)
#define ASX_LE_I64(exp1, exp2, fmt, ...)             _ASX_2(int64_t,exp1,<=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int64_t)
#define ASX_GT_I64(exp1, exp2, fmt, ...)             _ASX_2(int64_t,exp1,> ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int64_t)
#define ASX_GE_I64(exp1, exp2, fmt, ...)             _ASX_2(int64_t,exp1,>=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int64_t)
#define ASX_BETWEEN_IE_I64(low, exp, high, fmt, ...) _ASX_3(int64_t,low,<=,exp,< ,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp < high (int64_t)
#define ASX_BETWEEN_II_I64(low, exp, high, fmt, ...) _ASX_3(int64_t,low,<=,exp,<=,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int64_t)
#define ASX_BETWEEN_EI_I64(low, exp, high, fmt, ...) _ASX_3(int64_t,low,< ,exp,<=,high,"%lld",fmt,##__VA_ARGS__) // Assert low < exp <= high (int64_t)
#define ASX_BETWEEN_EE_I64(low, exp, high, fmt, ...) _ASX_3(int64_t,low,< ,exp,< ,high,"%lld",fmt,##__VA_ARGS__) // Assert low < exp < high (int64_t)
#define ASX_EQ_U8(exp1, exp2, fmt, ...)             _ASX_2(uint8_t,exp1,==,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint8_t)
#define ASX_NE_U8(exp1, exp2, fmt, ...)             _ASX_2(uint8_t,exp1,!=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint8_t)
#define ASX_LT_U8(exp1, exp2, fmt, ...)             _ASX_2(uint8_t,exp1,< ,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint8_t)
#define ASX_LE_U8(exp1, exp2, fmt, ...)             _ASX_2(uint8_t,exp1,<=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint8_t)
#define ASX_GT_U8(exp1, exp2, fmt, ...)             _ASX_2(uint8_t,exp1,> ,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint8_t)
#define ASX_GE_U8(exp1, exp2, fmt, ...)             _ASX_2(uint8_t,exp1,>=,exp2,"%hhu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint8_t)
#define ASX_BETWEEN_IE_U8(low, exp, high, fmt, ...) _ASX_3(uint8_t,low,<=,exp,< ,high,"%hhu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint8_t)
#define ASX_BETWEEN_II_U8(low, exp, high, fmt, ...) _ASX_3(uint8_t,low,<=,exp,<=,high,"%hhu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint8_t)
#define ASX_BETWEEN_EI_U8(low, exp, high, fmt, ...) _ASX_3(uint8_t,low,< ,exp,<=,high,"%hhu",fmt,##__VA_ARGS__) // Assert low < exp <= high (uint8_t)
#define ASX_BETWEEN_EE_U8(low, exp, high, fmt, ...) _ASX_3(uint8_t,low,< ,exp,< ,high,"%hhu",fmt,##__VA_ARGS__) // Assert low < exp < high (uint8_t)
#define ASX_EQ_U16(exp1, exp2, fmt, ...)             _ASX_2(uint16_t,exp1,==,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint16_t)
#define ASX_NE_U16(exp1, exp2, fmt, ...)             _ASX_2(uint16_t,exp1,!=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint16_t)
#define ASX_LT_U16(exp1, exp2, fmt, ...)             _ASX_2(uint16_t,exp1,< ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint16_t)
#define ASX_LE_U16(exp1, exp2, fmt, ...)             _ASX_2(uint16_t,exp1,<=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint16_t)
#define ASX_GT_U16(exp1, exp2, fmt, ...)             _ASX_2(uint16_t,exp1,> ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint16_t)
#define ASX_GE_U16(exp1, exp2, fmt, ...)             _ASX_2(uint16_t,exp1,>=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint16_t)
#define ASX_BETWEEN_IE_U16(low, exp, high, fmt, ...) _ASX_3(uint16_t,low,<=,exp,< ,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint16_t)
#define ASX_BETWEEN_II_U16(low, exp, high, fmt, ...) _ASX_3(uint16_t,low,<=,exp,<=,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint16_t)
#define ASX_BETWEEN_EI_U16(low, exp, high, fmt, ...) _ASX_3(uint16_t,low,< ,exp,<=,high,"%hu",fmt,##__VA_ARGS__) // Assert low < exp <= high (uint16_t)
#define ASX_BETWEEN_EE_U16(low, exp, high, fmt, ...) _ASX_3(uint16_t,low,< ,exp,< ,high,"%hu",fmt,##__VA_ARGS__) // Assert low < exp < high (uint16_t)
#define ASX_EQ_U32(exp1, exp2, fmt, ...)             _ASX_2(uint32_t,exp1,==,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint32_t)
#define ASX_NE_U32(exp1, exp2, fmt, ...)             _ASX_2(uint32_t,exp1,!=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint32_t)
#define ASX_LT_U32(exp1, exp2, fmt, ...)             _ASX_2(uint32_t,exp1,< ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint32_t)
#define ASX_LE_U32(exp1, exp2, fmt, ...)             _ASX_2(uint32_t,exp1,<=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint32_t)
#define ASX_GT_U32(exp1, exp2, fmt, ...)             _ASX_2(uint32_t,exp1,> ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint32_t)
#define ASX_GE_U32(exp1, exp2, fmt, ...)             _ASX_2(uint32_t,exp1,>=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint32_t)
#define ASX_BETWEEN_IE_U32(low, exp, high, fmt, ...) _ASX_3(uint32_t,low,<=,exp,< ,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint32_t)
#define ASX_BETWEEN_II_U32(low, exp, high, fmt, ...) _ASX_3(uint32_t,low,<=,exp,<=,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint32_t)
#define ASX_BETWEEN_EI_U32(low, exp, high, fmt, ...) _ASX_3(uint32_t,low,< ,exp,<=,high,"%u",fmt,##__VA_ARGS__) // Assert low < exp <= high (uint32_t)
#define ASX_BETWEEN_EE_U32(low, exp, high, fmt, ...) _ASX_3(uint32_t,low,< ,exp,< ,high,"%u",fmt,##__VA_ARGS__) // Assert low < exp < high (uint32_t)
#define ASX_EQ_U64(exp1, exp2, fmt, ...)             _ASX_2(uint64_t,exp1,==,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (uint64_t)
#define ASX_NE_U64(exp1, exp2, fmt, ...)             _ASX_2(uint64_t,exp1,!=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (uint64_t)
#define ASX_LT_U64(exp1, exp2, fmt, ...)             _ASX_2(uint64_t,exp1,< ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (uint64_t)
#define ASX_LE_U64(exp1, exp2, fmt, ...)             _ASX_2(uint64_t,exp1,<=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (uint64_t)
#define ASX_GT_U64(exp1, exp2, fmt, ...)             _ASX_2(uint64_t,exp1,> ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (uint64_t)
#define ASX_GE_U64(exp1, exp2, fmt, ...)             _ASX_2(uint64_t,exp1,>=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (uint64_t)
#define ASX_BETWEEN_IE_U64(low, exp, high, fmt, ...) _ASX_3(uint64_t,low,<=,exp,< ,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp < high (uint64_t)
#define ASX_BETWEEN_II_U64(low, exp, high, fmt, ...) _ASX_3(uint64_t,low,<=,exp,<=,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (uint64_t)
#define ASX_BETWEEN_EI_U64(low, exp, high, fmt, ...) _ASX_3(uint64_t,low,< ,exp,<=,high,"%llu",fmt,##__VA_ARGS__) // Assert low < exp <= high (uint64_t)
#define ASX_BETWEEN_EE_U64(low, exp, high, fmt, ...) _ASX_3(uint64_t,low,< ,exp,< ,high,"%llu",fmt,##__VA_ARGS__) // Assert low < exp < high (uint64_t)
#define ASX_EQ_SIZET(exp1, exp2, fmt, ...)             _ASX_2(size_t,exp1,==,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (size_t)
#define ASX_NE_SIZET(exp1, exp2, fmt, ...)             _ASX_2(size_t,exp1,!=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (size_t)
#define ASX_LT_SIZET(exp1, exp2, fmt, ...)             _ASX_2(size_t,exp1,< ,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (size_t)
#define ASX_LE_SIZET(exp1, exp2, fmt, ...)             _ASX_2(size_t,exp1,<=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (size_t)
#define ASX_GT_SIZET(exp1, exp2, fmt, ...)             _ASX_2(size_t,exp1,> ,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (size_t)
#define ASX_GE_SIZET(exp1, exp2, fmt, ...)             _ASX_2(size_t,exp1,>=,exp2,"%zu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (size_t)
#define ASX_BETWEEN_IE_SIZET(low, exp, high, fmt, ...) _ASX_3(size_t,low,<=,exp,< ,high,"%zu",fmt,##__VA_ARGS__) // Assert low <= exp < high (size_t)
#define ASX_BETWEEN_II_SIZET(low, exp, high, fmt, ...) _ASX_3(size_t,low,<=,exp,<=,high,"%zu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (size_t)
#define ASX_BETWEEN_EI_SIZET(low, exp, high, fmt, ...) _ASX_3(size_t,low,< ,exp,<=,high,"%zu",fmt,##__VA_ARGS__) // Assert low < exp <= high (size_t)
#define ASX_BETWEEN_EE_SIZET(low, exp, high, fmt, ...) _ASX_3(size_t,low,< ,exp,< ,high,"%zu",fmt,##__VA_ARGS__) // Assert low < exp < high (size_t)
#define ASX_EQ_SSIZET(exp1, exp2, fmt, ...)             _ASX_2(ssize_t,exp1,==,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (ssize_t)
#define ASX_NE_SSIZET(exp1, exp2, fmt, ...)             _ASX_2(ssize_t,exp1,!=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (ssize_t)
#define ASX_LT_SSIZET(exp1, exp2, fmt, ...)             _ASX_2(ssize_t,exp1,< ,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (ssize_t)
#define ASX_LE_SSIZET(exp1, exp2, fmt, ...)             _ASX_2(ssize_t,exp1,<=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (ssize_t)
#define ASX_GT_SSIZET(exp1, exp2, fmt, ...)             _ASX_2(ssize_t,exp1,> ,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (ssize_t)
#define ASX_GE_SSIZET(exp1, exp2, fmt, ...)             _ASX_2(ssize_t,exp1,>=,exp2,"%zd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (ssize_t)
#define ASX_BETWEEN_IE_SSIZET(low, exp, high, fmt, ...) _ASX_3(ssize_t,low,<=,exp,< ,high,"%zd",fmt,##__VA_ARGS__) // Assert low <= exp < high (ssize_t)
#define ASX_BETWEEN_II_SSIZET(low, exp, high, fmt, ...) _ASX_3(ssize_t,low,<=,exp,<=,high,"%zd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (ssize_t)
#define ASX_BETWEEN_EI_SSIZET(low, exp, high, fmt, ...) _ASX_3(ssize_t,low,< ,exp,<=,high,"%zd",fmt,##__VA_ARGS__) // Assert low < exp <= high (ssize_t)
#define ASX_BETWEEN_EE_SSIZET(low, exp, high, fmt, ...) _ASX_3(ssize_t,low,< ,exp,< ,high,"%zd",fmt,##__VA_ARGS__) // Assert low < exp < high (ssize_t)
#define ASX_EQ_F32(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,==,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (float)
#define ASX_NE_F32(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,!=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (float)
#define ASX_LT_F32(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,< ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (float)
#define ASX_LE_F32(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,<=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (float)
#define ASX_GT_F32(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,> ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (float)
#define ASX_GE_F32(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,>=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (float)
#define ASX_BETWEEN_IE_F32(low, exp, high, fmt, ...) _ASX_3(float,low,<=,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp < high (float)
#define ASX_BETWEEN_II_F32(low, exp, high, fmt, ...) _ASX_3(float,low,<=,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp <= high (float)
#define ASX_BETWEEN_EI_F32(low, exp, high, fmt, ...) _ASX_3(float,low,< ,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low < exp <= high (float)
#define ASX_BETWEEN_EE_F32(low, exp, high, fmt, ...) _ASX_3(float,low,< ,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low < exp < high (float)
#define ASX_EQ_F64(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,==,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (double)
#define ASX_NE_F64(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,!=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (double)
#define ASX_LT_F64(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,< ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (double)
#define ASX_LE_F64(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,<=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (double)
#define ASX_GT_F64(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,> ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (double)
#define ASX_GE_F64(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,>=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (double)
#define ASX_BETWEEN_IE_F64(low, exp, high, fmt, ...) _ASX_3(double,low,<=,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp < high (double)
#define ASX_BETWEEN_II_F64(low, exp, high, fmt, ...) _ASX_3(double,low,<=,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp <= high (double)
#define ASX_BETWEEN_EI_F64(low, exp, high, fmt, ...) _ASX_3(double,low,< ,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low < exp <= high (double)
#define ASX_BETWEEN_EE_F64(low, exp, high, fmt, ...) _ASX_3(double,low,< ,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low < exp < high (double)
#define ASX_EQ_BOOL(exp1, exp2, fmt, ...)             _ASX_2(bool,exp1,==,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (bool)
#define ASX_NE_BOOL(exp1, exp2, fmt, ...)             _ASX_2(bool,exp1,!=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (bool)
#define ASX_LT_BOOL(exp1, exp2, fmt, ...)             _ASX_2(bool,exp1,< ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (bool)
#define ASX_LE_BOOL(exp1, exp2, fmt, ...)             _ASX_2(bool,exp1,<=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (bool)
#define ASX_GT_BOOL(exp1, exp2, fmt, ...)             _ASX_2(bool,exp1,> ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (bool)
#define ASX_GE_BOOL(exp1, exp2, fmt, ...)             _ASX_2(bool,exp1,>=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (bool)
#define ASX_BETWEEN_IE_BOOL(low, exp, high, fmt, ...) _ASX_3(bool,low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp < high (bool)
#define ASX_BETWEEN_II_BOOL(low, exp, high, fmt, ...) _ASX_3(bool,low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp <= high (bool)
#define ASX_BETWEEN_EI_BOOL(low, exp, high, fmt, ...) _ASX_3(bool,low,< ,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low < exp <= high (bool)
#define ASX_BETWEEN_EE_BOOL(low, exp, high, fmt, ...) _ASX_3(bool,low,< ,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low < exp < high (bool)
#define ASX_EQ_CHAR(exp1, exp2, fmt, ...)             _ASX_2(char,exp1,==,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (char)
#define ASX_NE_CHAR(exp1, exp2, fmt, ...)             _ASX_2(char,exp1,!=,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (char)
#define ASX_LT_CHAR(exp1, exp2, fmt, ...)             _ASX_2(char,exp1,< ,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (char)
#define ASX_LE_CHAR(exp1, exp2, fmt, ...)             _ASX_2(char,exp1,<=,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (char)
#define ASX_GT_CHAR(exp1, exp2, fmt, ...)             _ASX_2(char,exp1,> ,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (char)
#define ASX_GE_CHAR(exp1, exp2, fmt, ...)             _ASX_2(char,exp1,>=,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (char)
#define ASX_BETWEEN_IE_CHAR(low, exp, high, fmt, ...) _ASX_3(char,low,<=,exp,< ,high,"%c",fmt,##__VA_ARGS__) // Assert low <= exp < high (char)
#define ASX_BETWEEN_II_CHAR(low, exp, high, fmt, ...) _ASX_3(char,low,<=,exp,<=,high,"%c",fmt,##__VA_ARGS__) // Assert low <= exp <= high (char)
#define ASX_BETWEEN_EI_CHAR(low, exp, high, fmt, ...) _ASX_3(char,low,< ,exp,<=,high,"%c",fmt,##__VA_ARGS__) // Assert low < exp <= high (char)
#define ASX_BETWEEN_EE_CHAR(low, exp, high, fmt, ...) _ASX_3(char,low,< ,exp,< ,high,"%c",fmt,##__VA_ARGS__) // Assert low < exp < high (char)
#define ASX_EQ_SHORT(exp1, exp2, fmt, ...)             _ASX_2(short,exp1,==,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (short)
#define ASX_NE_SHORT(exp1, exp2, fmt, ...)             _ASX_2(short,exp1,!=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (short)
#define ASX_LT_SHORT(exp1, exp2, fmt, ...)             _ASX_2(short,exp1,< ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (short)
#define ASX_LE_SHORT(exp1, exp2, fmt, ...)             _ASX_2(short,exp1,<=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (short)
#define ASX_GT_SHORT(exp1, exp2, fmt, ...)             _ASX_2(short,exp1,> ,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (short)
#define ASX_GE_SHORT(exp1, exp2, fmt, ...)             _ASX_2(short,exp1,>=,exp2,"%hd",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (short)
#define ASX_BETWEEN_IE_SHORT(low, exp, high, fmt, ...) _ASX_3(short,low,<=,exp,< ,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp < high (short)
#define ASX_BETWEEN_II_SHORT(low, exp, high, fmt, ...) _ASX_3(short,low,<=,exp,<=,high,"%hd",fmt,##__VA_ARGS__) // Assert low <= exp <= high (short)
#define ASX_BETWEEN_EI_SHORT(low, exp, high, fmt, ...) _ASX_3(short,low,< ,exp,<=,high,"%hd",fmt,##__VA_ARGS__) // Assert low < exp <= high (short)
#define ASX_BETWEEN_EE_SHORT(low, exp, high, fmt, ...) _ASX_3(short,low,< ,exp,< ,high,"%hd",fmt,##__VA_ARGS__) // Assert low < exp < high (short)
#define ASX_EQ_INT(exp1, exp2, fmt, ...)             _ASX_2(int,exp1,==,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (int)
#define ASX_NE_INT(exp1, exp2, fmt, ...)             _ASX_2(int,exp1,!=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (int)
#define ASX_LT_INT(exp1, exp2, fmt, ...)             _ASX_2(int,exp1,< ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (int)
#define ASX_LE_INT(exp1, exp2, fmt, ...)             _ASX_2(int,exp1,<=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (int)
#define ASX_GT_INT(exp1, exp2, fmt, ...)             _ASX_2(int,exp1,> ,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (int)
#define ASX_GE_INT(exp1, exp2, fmt, ...)             _ASX_2(int,exp1,>=,exp2,"%d",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (int)
#define ASX_BETWEEN_IE_INT(low, exp, high, fmt, ...) _ASX_3(int,low,<=,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp < high (int)
#define ASX_BETWEEN_II_INT(low, exp, high, fmt, ...) _ASX_3(int,low,<=,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low <= exp <= high (int)
#define ASX_BETWEEN_EI_INT(low, exp, high, fmt, ...) _ASX_3(int,low,< ,exp,<=,high,"%d",fmt,##__VA_ARGS__) // Assert low < exp <= high (int)
#define ASX_BETWEEN_EE_INT(low, exp, high, fmt, ...) _ASX_3(int,low,< ,exp,< ,high,"%d",fmt,##__VA_ARGS__) // Assert low < exp < high (int)
#define ASX_EQ_LONG(exp1, exp2, fmt, ...)             _ASX_2(long,exp1,==,exp2,"%ld",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (long)
#define ASX_NE_LONG(exp1, exp2, fmt, ...)             _ASX_2(long,exp1,!=,exp2,"%ld",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (long)
#define ASX_LT_LONG(exp1, exp2, fmt, ...)             _ASX_2(long,exp1,< ,exp2,"%ld",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (long)
#define ASX_LE_LONG(exp1, exp2, fmt, ...)             _ASX_2(long,exp1,<=,exp2,"%ld",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (long)
#define ASX_GT_LONG(exp1, exp2, fmt, ...)             _ASX_2(long,exp1,> ,exp2,"%ld",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (long)
#define ASX_GE_LONG(exp1, exp2, fmt, ...)             _ASX_2(long,exp1,>=,exp2,"%ld",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (long)
#define ASX_BETWEEN_IE_LONG(low, exp, high, fmt, ...) _ASX_3(long,low,<=,exp,< ,high,"%ld",fmt,##__VA_ARGS__) // Assert low <= exp < high (long)
#define ASX_BETWEEN_II_LONG(low, exp, high, fmt, ...) _ASX_3(long,low,<=,exp,<=,high,"%ld",fmt,##__VA_ARGS__) // Assert low <= exp <= high (long)
#define ASX_BETWEEN_EI_LONG(low, exp, high, fmt, ...) _ASX_3(long,low,< ,exp,<=,high,"%ld",fmt,##__VA_ARGS__) // Assert low < exp <= high (long)
#define ASX_BETWEEN_EE_LONG(low, exp, high, fmt, ...) _ASX_3(long,low,< ,exp,< ,high,"%ld",fmt,##__VA_ARGS__) // Assert low < exp < high (long)
#define ASX_EQ_LLONG(exp1, exp2, fmt, ...)             _ASX_2(long long,exp1,==,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (long long)
#define ASX_NE_LLONG(exp1, exp2, fmt, ...)             _ASX_2(long long,exp1,!=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (long long)
#define ASX_LT_LLONG(exp1, exp2, fmt, ...)             _ASX_2(long long,exp1,< ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (long long)
#define ASX_LE_LLONG(exp1, exp2, fmt, ...)             _ASX_2(long long,exp1,<=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (long long)
#define ASX_GT_LLONG(exp1, exp2, fmt, ...)             _ASX_2(long long,exp1,> ,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (long long)
#define ASX_GE_LLONG(exp1, exp2, fmt, ...)             _ASX_2(long long,exp1,>=,exp2,"%lld",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (long long)
#define ASX_BETWEEN_IE_LLONG(low, exp, high, fmt, ...) _ASX_3(long long,low,<=,exp,< ,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp < high (long long)
#define ASX_BETWEEN_II_LLONG(low, exp, high, fmt, ...) _ASX_3(long long,low,<=,exp,<=,high,"%lld",fmt,##__VA_ARGS__) // Assert low <= exp <= high (long long)
#define ASX_BETWEEN_EI_LLONG(low, exp, high, fmt, ...) _ASX_3(long long,low,< ,exp,<=,high,"%lld",fmt,##__VA_ARGS__) // Assert low < exp <= high (long long)
#define ASX_BETWEEN_EE_LLONG(low, exp, high, fmt, ...) _ASX_3(long long,low,< ,exp,< ,high,"%lld",fmt,##__VA_ARGS__) // Assert low < exp < high (long long)
#define ASX_EQ_UCHAR(exp1, exp2, fmt, ...)             _ASX_2(unsigned char,exp1,==,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (unsigned char)
#define ASX_NE_UCHAR(exp1, exp2, fmt, ...)             _ASX_2(unsigned char,exp1,!=,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (unsigned char)
#define ASX_LT_UCHAR(exp1, exp2, fmt, ...)             _ASX_2(unsigned char,exp1,< ,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (unsigned char)
#define ASX_LE_UCHAR(exp1, exp2, fmt, ...)             _ASX_2(unsigned char,exp1,<=,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (unsigned char)
#define ASX_GT_UCHAR(exp1, exp2, fmt, ...)             _ASX_2(unsigned char,exp1,> ,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (unsigned char)
#define ASX_GE_UCHAR(exp1, exp2, fmt, ...)             _ASX_2(unsigned char,exp1,>=,exp2,"%c",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (unsigned char)
#define ASX_BETWEEN_IE_UCHAR(low, exp, high, fmt, ...) _ASX_3(unsigned char,low,<=,exp,< ,high,"%c",fmt,##__VA_ARGS__) // Assert low <= exp < high (unsigned char)
#define ASX_BETWEEN_II_UCHAR(low, exp, high, fmt, ...) _ASX_3(unsigned char,low,<=,exp,<=,high,"%c",fmt,##__VA_ARGS__) // Assert low <= exp <= high (unsigned char)
#define ASX_BETWEEN_EI_UCHAR(low, exp, high, fmt, ...) _ASX_3(unsigned char,low,< ,exp,<=,high,"%c",fmt,##__VA_ARGS__) // Assert low < exp <= high (unsigned char)
#define ASX_BETWEEN_EE_UCHAR(low, exp, high, fmt, ...) _ASX_3(unsigned char,low,< ,exp,< ,high,"%c",fmt,##__VA_ARGS__) // Assert low < exp < high (unsigned char)
#define ASX_EQ_USHORT(exp1, exp2, fmt, ...)             _ASX_2(unsigned short,exp1,==,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (unsigned short)
#define ASX_NE_USHORT(exp1, exp2, fmt, ...)             _ASX_2(unsigned short,exp1,!=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (unsigned short)
#define ASX_LT_USHORT(exp1, exp2, fmt, ...)             _ASX_2(unsigned short,exp1,< ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (unsigned short)
#define ASX_LE_USHORT(exp1, exp2, fmt, ...)             _ASX_2(unsigned short,exp1,<=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (unsigned short)
#define ASX_GT_USHORT(exp1, exp2, fmt, ...)             _ASX_2(unsigned short,exp1,> ,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (unsigned short)
#define ASX_GE_USHORT(exp1, exp2, fmt, ...)             _ASX_2(unsigned short,exp1,>=,exp2,"%hu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (unsigned short)
#define ASX_BETWEEN_IE_USHORT(low, exp, high, fmt, ...) _ASX_3(unsigned short,low,<=,exp,< ,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp < high (unsigned short)
#define ASX_BETWEEN_II_USHORT(low, exp, high, fmt, ...) _ASX_3(unsigned short,low,<=,exp,<=,high,"%hu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (unsigned short)
#define ASX_BETWEEN_EI_USHORT(low, exp, high, fmt, ...) _ASX_3(unsigned short,low,< ,exp,<=,high,"%hu",fmt,##__VA_ARGS__) // Assert low < exp <= high (unsigned short)
#define ASX_BETWEEN_EE_USHORT(low, exp, high, fmt, ...) _ASX_3(unsigned short,low,< ,exp,< ,high,"%hu",fmt,##__VA_ARGS__) // Assert low < exp < high (unsigned short)
#define ASX_EQ_UINT(exp1, exp2, fmt, ...)             _ASX_2(unsigned int,exp1,==,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (unsigned int)
#define ASX_NE_UINT(exp1, exp2, fmt, ...)             _ASX_2(unsigned int,exp1,!=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (unsigned int)
#define ASX_LT_UINT(exp1, exp2, fmt, ...)             _ASX_2(unsigned int,exp1,< ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (unsigned int)
#define ASX_LE_UINT(exp1, exp2, fmt, ...)             _ASX_2(unsigned int,exp1,<=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (unsigned int)
#define ASX_GT_UINT(exp1, exp2, fmt, ...)             _ASX_2(unsigned int,exp1,> ,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (unsigned int)
#define ASX_GE_UINT(exp1, exp2, fmt, ...)             _ASX_2(unsigned int,exp1,>=,exp2,"%u",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (unsigned int)
#define ASX_BETWEEN_IE_UINT(low, exp, high, fmt, ...) _ASX_3(unsigned int,low,<=,exp,< ,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp < high (unsigned int)
#define ASX_BETWEEN_II_UINT(low, exp, high, fmt, ...) _ASX_3(unsigned int,low,<=,exp,<=,high,"%u",fmt,##__VA_ARGS__) // Assert low <= exp <= high (unsigned int)
#define ASX_BETWEEN_EI_UINT(low, exp, high, fmt, ...) _ASX_3(unsigned int,low,< ,exp,<=,high,"%u",fmt,##__VA_ARGS__) // Assert low < exp <= high (unsigned int)
#define ASX_BETWEEN_EE_UINT(low, exp, high, fmt, ...) _ASX_3(unsigned int,low,< ,exp,< ,high,"%u",fmt,##__VA_ARGS__) // Assert low < exp < high (unsigned int)
#define ASX_EQ_ULONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long,exp1,==,exp2,"%lu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (unsigned long)
#define ASX_NE_ULONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long,exp1,!=,exp2,"%lu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (unsigned long)
#define ASX_LT_ULONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long,exp1,< ,exp2,"%lu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (unsigned long)
#define ASX_LE_ULONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long,exp1,<=,exp2,"%lu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (unsigned long)
#define ASX_GT_ULONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long,exp1,> ,exp2,"%lu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (unsigned long)
#define ASX_GE_ULONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long,exp1,>=,exp2,"%lu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (unsigned long)
#define ASX_BETWEEN_IE_ULONG(low, exp, high, fmt, ...) _ASX_3(unsigned long,low,<=,exp,< ,high,"%lu",fmt,##__VA_ARGS__) // Assert low <= exp < high (unsigned long)
#define ASX_BETWEEN_II_ULONG(low, exp, high, fmt, ...) _ASX_3(unsigned long,low,<=,exp,<=,high,"%lu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (unsigned long)
#define ASX_BETWEEN_EI_ULONG(low, exp, high, fmt, ...) _ASX_3(unsigned long,low,< ,exp,<=,high,"%lu",fmt,##__VA_ARGS__) // Assert low < exp <= high (unsigned long)
#define ASX_BETWEEN_EE_ULONG(low, exp, high, fmt, ...) _ASX_3(unsigned long,low,< ,exp,< ,high,"%lu",fmt,##__VA_ARGS__) // Assert low < exp < high (unsigned long)
#define ASX_EQ_ULLONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long long,exp1,==,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (unsigned long long)
#define ASX_NE_ULLONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long long,exp1,!=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (unsigned long long)
#define ASX_LT_ULLONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long long,exp1,< ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (unsigned long long)
#define ASX_LE_ULLONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long long,exp1,<=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (unsigned long long)
#define ASX_GT_ULLONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long long,exp1,> ,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (unsigned long long)
#define ASX_GE_ULLONG(exp1, exp2, fmt, ...)             _ASX_2(unsigned long long,exp1,>=,exp2,"%llu",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (unsigned long long)
#define ASX_BETWEEN_IE_ULLONG(low, exp, high, fmt, ...) _ASX_3(unsigned long long,low,<=,exp,< ,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp < high (unsigned long long)
#define ASX_BETWEEN_II_ULLONG(low, exp, high, fmt, ...) _ASX_3(unsigned long long,low,<=,exp,<=,high,"%llu",fmt,##__VA_ARGS__) // Assert low <= exp <= high (unsigned long long)
#define ASX_BETWEEN_EI_ULLONG(low, exp, high, fmt, ...) _ASX_3(unsigned long long,low,< ,exp,<=,high,"%llu",fmt,##__VA_ARGS__) // Assert low < exp <= high (unsigned long long)
#define ASX_BETWEEN_EE_ULLONG(low, exp, high, fmt, ...) _ASX_3(unsigned long long,low,< ,exp,< ,high,"%llu",fmt,##__VA_ARGS__) // Assert low < exp < high (unsigned long long)
#define ASX_EQ_FLOAT(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,==,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (float)
#define ASX_NE_FLOAT(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,!=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (float)
#define ASX_LT_FLOAT(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,< ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (float)
#define ASX_LE_FLOAT(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,<=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (float)
#define ASX_GT_FLOAT(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,> ,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (float)
#define ASX_GE_FLOAT(exp1, exp2, fmt, ...)             _ASX_2(float,exp1,>=,exp2,"%f",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (float)
#define ASX_BETWEEN_IE_FLOAT(low, exp, high, fmt, ...) _ASX_3(float,low,<=,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp < high (float)
#define ASX_BETWEEN_II_FLOAT(low, exp, high, fmt, ...) _ASX_3(float,low,<=,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low <= exp <= high (float)
#define ASX_BETWEEN_EI_FLOAT(low, exp, high, fmt, ...) _ASX_3(float,low,< ,exp,<=,high,"%f",fmt,##__VA_ARGS__) // Assert low < exp <= high (float)
#define ASX_BETWEEN_EE_FLOAT(low, exp, high, fmt, ...) _ASX_3(float,low,< ,exp,< ,high,"%f",fmt,##__VA_ARGS__) // Assert low < exp < high (float)
#define ASX_EQ_DOUBLE(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,==,exp2,"%lf",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (double)
#define ASX_NE_DOUBLE(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,!=,exp2,"%lf",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (double)
#define ASX_LT_DOUBLE(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,< ,exp2,"%lf",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (double)
#define ASX_LE_DOUBLE(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,<=,exp2,"%lf",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (double)
#define ASX_GT_DOUBLE(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,> ,exp2,"%lf",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (double)
#define ASX_GE_DOUBLE(exp1, exp2, fmt, ...)             _ASX_2(double,exp1,>=,exp2,"%lf",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (double)
#define ASX_BETWEEN_IE_DOUBLE(low, exp, high, fmt, ...) _ASX_3(double,low,<=,exp,< ,high,"%lf",fmt,##__VA_ARGS__) // Assert low <= exp < high (double)
#define ASX_BETWEEN_II_DOUBLE(low, exp, high, fmt, ...) _ASX_3(double,low,<=,exp,<=,high,"%lf",fmt,##__VA_ARGS__) // Assert low <= exp <= high (double)
#define ASX_BETWEEN_EI_DOUBLE(low, exp, high, fmt, ...) _ASX_3(double,low,< ,exp,<=,high,"%lf",fmt,##__VA_ARGS__) // Assert low < exp <= high (double)
#define ASX_BETWEEN_EE_DOUBLE(low, exp, high, fmt, ...) _ASX_3(double,low,< ,exp,< ,high,"%lf",fmt,##__VA_ARGS__) // Assert low < exp < high (double)
#define ASX_EQ_LDOUBLE(exp1, exp2, fmt, ...)             _ASX_2(long double,exp1,==,exp2,"%Lf",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (long double)
#define ASX_NE_LDOUBLE(exp1, exp2, fmt, ...)             _ASX_2(long double,exp1,!=,exp2,"%Lf",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (long double)
#define ASX_LT_LDOUBLE(exp1, exp2, fmt, ...)             _ASX_2(long double,exp1,< ,exp2,"%Lf",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (long double)
#define ASX_LE_LDOUBLE(exp1, exp2, fmt, ...)             _ASX_2(long double,exp1,<=,exp2,"%Lf",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (long double)
#define ASX_GT_LDOUBLE(exp1, exp2, fmt, ...)             _ASX_2(long double,exp1,> ,exp2,"%Lf",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (long double)
#define ASX_GE_LDOUBLE(exp1, exp2, fmt, ...)             _ASX_2(long double,exp1,>=,exp2,"%Lf",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (long double)
#define ASX_BETWEEN_IE_LDOUBLE(low, exp, high, fmt, ...) _ASX_3(long double,low,<=,exp,< ,high,"%Lf",fmt,##__VA_ARGS__) // Assert low <= exp < high (long double)
#define ASX_BETWEEN_II_LDOUBLE(low, exp, high, fmt, ...) _ASX_3(long double,low,<=,exp,<=,high,"%Lf",fmt,##__VA_ARGS__) // Assert low <= exp <= high (long double)
#define ASX_BETWEEN_EI_LDOUBLE(low, exp, high, fmt, ...) _ASX_3(long double,low,< ,exp,<=,high,"%Lf",fmt,##__VA_ARGS__) // Assert low < exp <= high (long double)
#define ASX_BETWEEN_EE_LDOUBLE(low, exp, high, fmt, ...) _ASX_3(long double,low,< ,exp,< ,high,"%Lf",fmt,##__VA_ARGS__) // Assert low < exp < high (long double)
#define ASX_EQ_PTR(exp1, exp2, fmt, ...)             _ASX_2(void *,exp1,==,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 == exp2 (void *)
#define ASX_NE_PTR(exp1, exp2, fmt, ...)             _ASX_2(void *,exp1,!=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 != exp2 (void *)
#define ASX_LT_PTR(exp1, exp2, fmt, ...)             _ASX_2(void *,exp1,< ,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 < exp2 (void *)
#define ASX_LE_PTR(exp1, exp2, fmt, ...)             _ASX_2(void *,exp1,<=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 <= exp2 (void *)
#define ASX_GT_PTR(exp1, exp2, fmt, ...)             _ASX_2(void *,exp1,> ,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 > exp2 (void *)
#define ASX_GE_PTR(exp1, exp2, fmt, ...)             _ASX_2(void *,exp1,>=,exp2,"%p",fmt,##__VA_ARGS__) // Assert exp1 >= exp2 (void *)
#define ASX_BETWEEN_IE_PTR(low, exp, high, fmt, ...) _ASX_3(void *,low,<=,exp,< ,high,"%p",fmt,##__VA_ARGS__) // Assert low <= exp < high (void *)
#define ASX_BETWEEN_II_PTR(low, exp, high, fmt, ...) _ASX_3(void *,low,<=,exp,<=,high,"%p",fmt,##__VA_ARGS__) // Assert low <= exp <= high (void *)
#define ASX_BETWEEN_EI_PTR(low, exp, high, fmt, ...) _ASX_3(void *,low,< ,exp,<=,high,"%p",fmt,##__VA_ARGS__) // Assert low < exp <= high (void *)
#define ASX_BETWEEN_EE_PTR(low, exp, high, fmt, ...) _ASX_3(void *,low,< ,exp,< ,high,"%p",fmt,##__VA_ARGS__) // Assert low < exp < high (void *)
/*
categories = [("AS", False), ("ASX", True)]
types = [
    # Standard types
    ("int8_t" ,  "I8",     "%hhd"),
    ("int16_t",  "I16",    "%hd"),
    ("int32_t",  "I32",    "%d"),
    ("int64_t",  "I64",    "%lld"),
    ("uint8_t",  "U8",     "%hhu"),
    ("uint16_t", "U16",    "%hu"),
    ("uint32_t", "U32",    "%u"),
    ("uint64_t", "U64",    "%llu"),
    ("size_t",   "SIZET",  "%zu"),
    ("ssize_t",  "SSIZET", "%zd"),
    ("float",    "F32",    "%f"),
    ("double",   "F64",    "%f"),
    # Primitive types
    ("bool",               "BOOL",    "%d"),
    ("char",               "CHAR",    "%c"),
    ("short",              "SHORT",   "%hd"),
    ("int",                "INT",     "%d"),
    ("long",               "LONG",    "%ld"),
    ("long long",          "LLONG",   "%lld"),
    ("unsigned char",      "UCHAR",   "%c"),
    ("unsigned short",     "USHORT",  "%hu"),
    ("unsigned int",       "UINT",    "%u"),
    ("unsigned long",      "ULONG",   "%lu"),
    ("unsigned long long", "ULLONG",  "%llu"),
    ("float",              "FLOAT"  , "%f"),
    ("double",             "DOUBLE" , "%lf"),
    ("long double",        "LDOUBLE", "%Lf"),
    ("void *",             "PTR",     "%p"),
]
ops = [
    ("EQ", 2, "=="),
    ("NE", 2, "!="),
    ("LT", 2, "<"),
    ("LE", 2, "<="),
    ("GT", 2, ">"),
    ("GE", 2, ">="),
    ("BETWEEN_IE", 3, "<=", "<"),
    ("BETWEEN_II", 3, "<=", "<="),
    ("BETWEEN_EI", 3, "<", "<="),
    ("BETWEEN_EE", 3, "<", "<"),
]

for category in categories:
    for typ in types:
        max_prefix = 0
        lines = []
        for op in ops:
            prefix = f"#define {category[0]}_{op[0]}_{typ[1]}"
            if op[1] == 2:
                prefix += f"(exp1, exp2"
            elif op[1] == 3:
                prefix += f"(low, exp, high"
            if category[1] == False:
                prefix += f")"
            else:
                prefix += f", fmt, ...)"
            suffix = ""
            if op[1] == 2:
                suffix += f'_ASX_2({typ[0]},exp1,{op[2]:2s},exp2,"{typ[2]}",'
            elif op[1] == 3:
                suffix += f'_ASX_3({typ[0]},low,{op[2]:2s},exp,{op[3]:2s},high,"{typ[2]}",'
            if category[1] == False:
                suffix += f'"")'
            else:
                suffix += f'fmt,##__VA_ARGS__)'
            if op[1] == 2:
                suffix += f" // Assert exp1 {op[2]} exp2 ({typ[0]})"
            elif op[1] == 3:
                suffix += f" // Assert low {op[2]} exp {op[3]} high ({typ[0]})"
            max_prefix = max(max_prefix, len(prefix))
            lines.append((prefix, suffix))
        for prefix, suffix in lines:
            print(f"{prefix.ljust(max_prefix)} {suffix}")
*/
