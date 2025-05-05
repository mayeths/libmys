/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

#include "_config.h"

typedef struct mys_string_t {
    char *text;
    size_t size; // the length not including '\0'. same as `strlen(text)`
    size_t capacity; // total space of text
} mys_string_t;

/**
 * @brief Create a string manager
 * 
 * @return `mys_string_t` a string manager where you can access string in `mys_string_t::text`
 * 
 * @note
 * `mys_string_t *str = mys_string_create(); printf("%s\\n", str->text);`
 */
MYS_PUBLIC mys_string_t *mys_string_create();
/**
 * @brief Destroy a string manager
 * 
 * @param str string manager to destroy
 */
MYS_PUBLIC void mys_string_destroy(mys_string_t **str);
/**
 * @brief Append formatted string to the end of `str.text`
 * 
 * @param str string manager
 * @param format formatter
 * @param ... variable length arguments
 * 
 * @note this routine will automatically extend the size of string if necessary.
 */
MYS_PUBLIC int mys_string_fmt(mys_string_t *str, const char *format, ...);
/**
 * @brief Append other string to the end of `str.text`
 * 
 * @param str string manager
 * @param other other string
 * 
 * @note this routine will automatically extend the size of string if necessary.
 */
MYS_PUBLIC int mys_string_append(mys_string_t *str, const char *other);
MYS_PUBLIC int mys_string_append_n(mys_string_t *str, const char *other, size_t len);
MYS_PUBLIC int mys_string_append2(mys_string_t *str, mys_string_t *other);

MYS_PUBLIC void mys_string_resize(mys_string_t *str, size_t len);
MYS_PUBLIC void mys_string_clear(mys_string_t *str);

MYS_PUBLIC mys_string_t *mys_string_dup(const char *str2);
MYS_PUBLIC mys_string_t *mys_string_dup_n(const char *str2, size_t len);

MYS_PUBLIC ssize_t mys_parse_readable_size(const char *text);
/*
Example usage:
```c
char nbytes_str[16];
mys_to_readable_size(num_bytes, 2, nbytes_str, sizeof(nbytes_str));
```
*/
MYS_PUBLIC void mys_to_readable_size(size_t bytes, size_t precision, char *buffer, size_t buflen);

MYS_PUBLIC int mys_str_to_int(const char *str, int default_val);
MYS_PUBLIC long mys_str_to_long(const char *str, long default_val);
MYS_PUBLIC size_t mys_str_to_sizet(const char *str, size_t default_val);
MYS_PUBLIC double mys_str_to_double(const char *str, double default_val);
MYS_PUBLIC float mys_str_to_float(const char *str, float default_val);

MYS_PUBLIC uint64_t mys_str_to_u64(const char *str, uint64_t default_val);
MYS_PUBLIC uint32_t mys_str_to_u32(const char *str, uint32_t default_val);
MYS_PUBLIC int64_t mys_str_to_i64(const char *str, int64_t default_val);
MYS_PUBLIC int32_t mys_str_to_i32(const char *str, int32_t default_val);
MYS_PUBLIC double mys_str_to_f64(const char *str, double default_val);
MYS_PUBLIC float mys_str_to_f32(const char *str, float default_val);
