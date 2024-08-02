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


MYS_PUBLIC ssize_t mys_parse_readable_size(const char *text);
MYS_PUBLIC void mys_readable_size(char **ptr, size_t bytes, size_t precision);

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
