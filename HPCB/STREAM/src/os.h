#ifndef __OS_H__
#define __OS_H__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>

static inline int64_t str_to_i64(const char *str, int64_t default_val)
{
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    int64_t num = strtoll(str, &stop, 10);
    int error = errno;
    errno = 0;

    if (stop == str)
        return default_val; /* contains with non-number */
    if ((num == LLONG_MAX || num == LLONG_MIN) && error == ERANGE)
        return default_val; /* number out of range for LONG */
    return num;
}

static inline int32_t str_to_i32(const char *str, int32_t default_val)
{
    int64_t num = str_to_i64(str, (int64_t)default_val);
    if ((num < INT_MIN) || (num > INT_MAX))
        return default_val; /* number out of range for INT */
    return (int32_t)num;
}

static inline double str_to_f64(const char *str, double default_val)
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
 * Use strtof instead of envf64
 */
static inline float str_to_f32(const char *str, float default_val)
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

static inline void to_readable_size(char **ptr, size_t bytes, size_t precision)
{
    int i = 0;
    const char* units[] = {"Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double size = bytes;
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    int len = snprintf(NULL, 0, "%.*f %s", (int)precision, size, units[i]) + 1; /*%.*f*/
    *ptr = (char *)malloc(sizeof(char) * len);
    snprintf(*ptr, len, "%.*f %s", (int)precision, size, units[i]);
}

static ssize_t from_readable_size(const char *text) {
    typedef struct {
        const char *suffix;
        size_t base;
    } unit_t;
    static const size_t Bbase = 1ULL;
    static const size_t Kbase = 1024ULL * Bbase;
    static const size_t Mbase = 1024ULL * Kbase;
    static const size_t Gbase = 1024ULL * Mbase;
    static const size_t Tbase = 1024ULL * Gbase;
    static const size_t Pbase = 1024ULL * Tbase;
    static const size_t Ebase = 1024ULL * Pbase;
    static const size_t Zbase = 1024ULL * Ebase;
    unit_t units[] = {
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

    ssize_t num = (ssize_t)dnum;

    while (*endptr == ' ')
        endptr++;
    if (*endptr == '\0')
        return (ssize_t)dnum; /* no suffix */

    for (int i = 0; i < sizeof(units) / sizeof(unit_t); i++) {
        unit_t unit = units[i];
        int matched = strncmp(endptr, unit.suffix, 32) == 0;
        if (matched)
            return num * unit.base;
    }

    return -1;
}

static inline size_t read_all_content(void **buffer, const char *filename)
{
    *buffer = NULL;
    FILE *fd = fopen(filename, "rb");
    if (fd == NULL)
        return 0;
    fseek(fd, 0, SEEK_END);
    size_t len = (size_t)ftell(fd);
    fseek(fd, 0, SEEK_SET);
    *buffer = (void *)calloc(sizeof(char), len + 1);
    fread(*buffer, len, 1, fd);
    fclose(fd);
    ((char *)*buffer)[len] = 0;
    return len;
}

/**
 * @brief Get the size of data cache
 * 
 * @param cpu CPU id to query from
 * @param level The level of data cache. Use -1 for LLC
 * @return The size of cache. -1 if error occur
 * 
 * @note The result is query from "/sys/devices/system/cpu0/cache/" on LINUX,
 *       or by running "sysctl -a" on MacOS (TODO)
 */
static inline int get_datacache_size(int level)
{
    int last_size = -1;
#ifdef __linux__
    int index = 0;
    do {
        char dir[256];
        char type_file[512];
        char size_file[512];
        char level_file[512];
        snprintf(dir, sizeof(dir), "/sys/devices/system/cpu/cpu0/cache/index%d", index);
        snprintf(type_file, sizeof(type_file), "%s/type", dir);
        snprintf(size_file, sizeof(size_file), "%s/size", dir);
        snprintf(level_file, sizeof(level_file), "%s/level", dir);

        char *type_buffer = NULL;
        char *size_buffer = NULL;
        char *level_buffer = NULL;
        size_t type_slen = read_all_content((void **)&type_buffer, type_file);
        read_all_content((void **)&size_buffer, size_file);
        read_all_content((void **)&level_buffer, level_file);
        if (type_buffer == NULL || size_buffer == NULL || level_buffer == NULL)
            break;

        int is_unified = strncmp(type_buffer, "Unified", type_slen) == 0;
        int is_data = strncmp(type_buffer, "Data", type_slen) == 0;
        if (!is_unified && !is_data)
            continue;

        int curr_size = atoi(size_buffer);
        int curr_level = atoi(level_buffer);
        if (curr_level == level)
            return curr_size;
        last_size = curr_size;
    } while (1);
#elif defined(__APPLE__)
#endif
    int asked_LLC = level == -1;
    return asked_LLC ? last_size : -1;
}

#endif /*__OS_H__*/