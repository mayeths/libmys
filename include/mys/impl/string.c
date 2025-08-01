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
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../string.h"
#include "../memory.h"

MYS_PUBLIC ssize_t mys_parse_readable_size(const char *text)
{
#define Bbase (double)(1.0)
#define Kbase ((double)1024.0 * (Bbase))
#define Mbase ((double)1024.0 * (Kbase))
#define Gbase ((double)1024.0 * (Mbase))
#define Tbase ((double)1024.0 * (Gbase))
#define Pbase ((double)1024.0 * (Tbase))
#define Ebase ((double)1024.0 * (Pbase))
#define Zbase ((double)1024.0 * (Ebase))
    struct unit_t {
        const char *suffix;
        double base;
    };
    static struct unit_t units[] = {
        { .suffix = "Bytes",  .base = Bbase },
        { .suffix = "Byte",   .base = Bbase },
        { .suffix = "B",      .base = Bbase },
        { .suffix = "KBytes", .base = Kbase },
        { .suffix = "KB",     .base = Kbase },
        { .suffix = "K",      .base = Kbase },
        { .suffix = "MBytes", .base = Mbase },
        { .suffix = "MB",     .base = Mbase },
        { .suffix = "M",      .base = Mbase },
        { .suffix = "GBytes", .base = Gbase },
        { .suffix = "GB",     .base = Gbase },
        { .suffix = "G",      .base = Gbase },
        { .suffix = "TBytes", .base = Tbase },
        { .suffix = "TB",     .base = Tbase },
        { .suffix = "T",      .base = Tbase },
        { .suffix = "PBytes", .base = Pbase },
        { .suffix = "PB",     .base = Pbase },
        { .suffix = "P",      .base = Pbase },
        { .suffix = "EBytes", .base = Ebase },
        { .suffix = "EB",     .base = Ebase },
        { .suffix = "E",      .base = Ebase },
        { .suffix = "ZBytes", .base = Zbase },
        { .suffix = "ZB",     .base = Zbase },
        { .suffix = "Z",      .base = Zbase },
    };

    char *endptr = NULL;
    errno = 0;
    double dnum = strtod(text, &endptr);
    int error = errno;
    errno = 0;

    if (endptr == text)
        return -1; /* contains with non-number */
    if (error == ERANGE)
        return -1; /* number out of range for double */
    if (dnum != dnum)
        return -1; /* not a number */

    while (*endptr == ' ')
        endptr++;
    if (*endptr == '\0')
        return (ssize_t)dnum; /* no suffix */

    for (size_t i = 0; i < sizeof(units) / sizeof(struct unit_t); i++) {
        struct unit_t *unit = &units[i];
        int matched = strncmp(endptr, unit->suffix, 32) == 0;
        if (matched)
            return (ssize_t)(dnum * unit->base);
    }

    return -1;
#undef Bbase
#undef Kbase
#undef Mbase
#undef Gbase
#undef Tbase
#undef Pbase
#undef Ebase
#undef Zbase
}

MYS_PUBLIC void mys_to_readable_size(size_t bytes, size_t precision, char *buffer, size_t buflen)
{
    int i = 0;
    const char* units[] = {"Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double size = bytes;
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    snprintf(buffer, buflen, "%.*f %s", (int)precision, size, units[i]);
}

MYS_PUBLIC mys_string_t *mys_string_create()
{
    mys_string_t *str = (mys_string_t *)mys_malloc2(MYS_ARENA_STR, sizeof(mys_string_t));
    if (!str)
        return NULL;
    str->capacity = 16;
    str->size = 0;
    str->text = (char *)mys_malloc2(MYS_ARENA_STR, str->capacity);
    if (!str->text) {
        mys_free2(MYS_ARENA_STR, str, sizeof(mys_string_t));
        return NULL;
    }
    str->text[0] = '\0';

    return str;
}

MYS_PUBLIC mys_string_t *mys_string_create2(const char *initial_fmt, ...)
{
    mys_string_t *str = mys_string_create();
    if (str == NULL)
        return NULL; // Allocation failed
    va_list vargs;
    va_start(vargs, initial_fmt);
    int written = mys_string_fmt_v(str, initial_fmt, vargs);
    va_end(vargs);
    if (written < 0) {
        mys_string_destroy(&str);
        return NULL; // Formatting failed
    }
    return str;
}

MYS_PUBLIC void mys_string_destroy(mys_string_t **str)
{
    if (str != NULL) {
        if ((*str)->text != NULL)
            mys_free2(MYS_ARENA_STR, (*str)->text, (*str)->capacity);
        mys_free2(MYS_ARENA_STR, *str, sizeof(mys_string_t));
    }
    *str = NULL;
}

MYS_STATIC ssize_t _mys_string_reallocate_if_needed(mys_string_t *str, size_t required)
{
    if (str->capacity < str->size + required) {
        size_t new_capacity = str->capacity;
        while (new_capacity < str->size + required) {
            new_capacity *= 2; // Double the capacity
        }
        char *new_text = (char *)mys_realloc2(MYS_ARENA_STR, str->text, new_capacity, str->capacity);
        if (new_text == NULL)
            return -1;
        str->text = new_text;
        str->capacity = new_capacity;
    }
    return (ssize_t)str->capacity;
}

MYS_PUBLIC int mys_string_fmt(mys_string_t *str, const char *format, ...)
{
    int written;
    va_list vargs;
    va_start(vargs, format);
    written = mys_string_fmt_v(str, format, vargs);
    va_end(vargs);
    return written;
}

MYS_PUBLIC int mys_string_fmt_v(mys_string_t *str, const char *format, va_list vargs)
{
    int written = -1;
    int needed = 0;
    va_list vargs_copy;

    if (str == NULL || format == NULL)
        goto finish;

    va_copy(vargs_copy, vargs);
    needed = vsnprintf(NULL, 0, format, vargs_copy);
    va_end(vargs_copy);
    if (needed < 0)
        goto finish;
    if (_mys_string_reallocate_if_needed(str, needed + 1/*'\0'*/) == -1)
        goto finish;

    written = vsnprintf(str->text + str->size, str->capacity - str->size, format, vargs);
    str->size += written;
finish:
    return written;
}


MYS_PUBLIC int mys_string_append(mys_string_t *str, const char *other)
{
    return mys_string_append_n(str, other, strlen(other));
}

MYS_PUBLIC int mys_string_append2(mys_string_t *str, mys_string_t *other)
{
    if (str == NULL || other == NULL || other->text == NULL)
        return -1; // Handle null input
    return mys_string_append_n(str, other->text, other->size);
}

MYS_PUBLIC int mys_string_append_n(mys_string_t *str, const char *other, size_t len)
{
    if (_mys_string_reallocate_if_needed(str, len + 1/*'\0'*/) == -1)
        return -1;

    memcpy(str->text + str->size, other, len);
    str->size += len;
    str->text[str->size] = '\0';
    return len + 1;
}

MYS_PUBLIC void mys_string_clear(mys_string_t *str)
{
    mys_string_resize(str, 0);
}

MYS_PUBLIC void mys_string_resize(mys_string_t *str, size_t len)
{
    if (len < str->size) {
        // Truncate the string by adjusting its size
        str->size = len;
        str->text[len] = '\0'; // Null-terminate the string
    } else if (len > str->size) {
        // Extend the string if needed
        if (_mys_string_reallocate_if_needed(str, len + 1/*'\0'*/) == -1) {
            return; // Allocation failed, leave the string unchanged
        }
        // Fill the extended area with null characters
        memset(str->text + str->size, '\0', len - str->size);
        str->size = len;
        str->text[len] = '\0'; // Null-terminate the string
    }
}

MYS_PUBLIC mys_string_t *mys_string_dup(const char *str)
{
    return mys_string_dup_n(str, strlen(str));
}

MYS_PUBLIC mys_string_t *mys_string_dup_n(const char *str, size_t len)
{
    if (str == NULL)
        return NULL; // Handle null input

    mys_string_t *new_str = mys_string_create();
    if (new_str == NULL)
        return NULL; // Allocation failed

    if (mys_string_append_n(new_str, str, len) == -1) {
        mys_string_destroy(&new_str);
        return NULL;
    }
    return new_str;
}

MYS_PUBLIC int mys_str_to_int(const char *str, int default_val)
{
    if (sizeof(int) == sizeof(int32_t))
        return (int)mys_str_to_i32(str, default_val);
    else
        return (int)mys_str_to_i64(str, default_val);
}

MYS_PUBLIC long mys_str_to_long(const char *str, long default_val)
{
    if (sizeof(long) == sizeof(int32_t))
        return (long)mys_str_to_i32(str, default_val);
    else
        return (long)mys_str_to_i64(str, default_val);
}

MYS_PUBLIC size_t mys_str_to_sizet(const char *str, size_t default_val)
{
    if (sizeof(size_t) == sizeof(uint32_t))
        return (size_t)mys_str_to_u32(str, default_val);
    else
        return (size_t)mys_str_to_u64(str, default_val);
}

MYS_PUBLIC double mys_str_to_double(const char *str, double default_val)
{
    return (double)mys_str_to_f64(str, default_val);
}

MYS_PUBLIC float mys_str_to_float(const char *str, float default_val)
{
    return (float)mys_str_to_f32(str, default_val);
}


MYS_PUBLIC uint64_t mys_str_to_u64(const char *str, uint64_t default_val)
{
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    uint64_t num = strtoull(str, &stop, 0);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if ((num == ULLONG_MAX) && error == ERANGE)
        return default_val; /* number out of range for ULONG */
    return num;
}

MYS_PUBLIC uint32_t mys_str_to_u32(const char *str, uint32_t default_val)
{
    uint64_t num = mys_str_to_u64(str, (uint64_t)default_val);
    if (num > UINT_MAX)
        return default_val; /* number out of range for UINT */
    return (uint32_t)num;
}


MYS_PUBLIC int64_t mys_str_to_i64(const char *str, int64_t default_val)
{
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    int64_t num = strtoll(str, &stop, 0);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if ((num == LLONG_MAX || num == LLONG_MIN) && error == ERANGE)
        return default_val; /* number out of range for LONG */
    return num;
}

MYS_PUBLIC int32_t mys_str_to_i32(const char *str, int32_t default_val)
{
    int64_t num = mys_str_to_i64(str, (int64_t)default_val);
    if ((num < INT_MIN) || (num > INT_MAX))
        return default_val; /* number out of range for INT */
    return (int32_t)num;
}

MYS_PUBLIC double mys_str_to_f64(const char *str, double default_val)
{
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    double num = strtod(str, &stop);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if (error == ERANGE)
        return default_val; /* number out of range for DOUBLE */
    if (num != num)
        return default_val; /* Not A Number */
    return num;
}

/*
 * We don't assume to use IEEE754 arithmetic where
 * default_val float->double->float is unchanged.
 * Use strtof instead of mys_str_to_f64
 */
MYS_PUBLIC float mys_str_to_f32(const char *str, float default_val)
{
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    float num = strtof(str, &stop);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if (error == ERANGE)
        return default_val; /* number out of range for FLOAT */
    if (num != num)
        return default_val; /* Not A Number */
    return num;
}
