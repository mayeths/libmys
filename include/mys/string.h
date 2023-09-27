#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "_config.h"

struct mys_string_struct {
    char *text;
    size_t size; // the length not including '\0'. same as `strlen(text)`
    size_t capacity; // total space of text
};

typedef struct mys_string_struct *mys_string_t;

/**
 * @brief Create a string manager
 * 
 * @return `mys_string_t` a string manager where you can access string in `mys_string_t::text`
 * 
 * @note
 * `mys_string_t str = mys_string_create(); printf("%s\\n", str->text);`
 */
MYS_API mys_string_t mys_string_create();
/**
 * @brief Destroy a string manager
 * 
 * @param str string manager to destroy
 */
MYS_API void mys_string_destroy(mys_string_t str);
/**
 * @brief Append formatted string to the end of `str->text`
 * 
 * @param str string manager
 * @param format formatter
 * @param ... variable length arguments
 * 
 * @note this routine will automatically extend the size of string if necessary.
 */
MYS_API void mys_string_fmt(mys_string_t str, const char *format, ...);
