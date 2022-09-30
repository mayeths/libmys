#pragma once

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <float.h>

/**
 * @brief setenv in stdlib.h
 * 
 * @param envname
 * @param envval 
 * @param overwrite The environment variable shall be updated if the overwrite is non-zero.
 * @return int - 0 when environment variable is presented after the function called. Otherwise -1 and errno is set.
 */
int setenv(const char *envname, const char *envval, int overwrite);
int unsetenv(const char *envname);

/* Safe string to numeric https://stackoverflow.com/a/18544436 */

static inline const char *envstr(const char *name, const char *default_val)
{
    const char *val = getenv(name);
    if (val == NULL)
        return default_val;
    return val;
}

static inline int64_t envi64(const char *name, int64_t default_val)
{
    const char *str = getenv(name);
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    int64_t num = strtoll(str, &stop, 10);
    int error = errno;
    errno = 0;

    if (stop == str || stop != NULL)
        return default_val; /* contains with non-number */
    if ((num == LLONG_MAX || num == LLONG_MIN) && error == ERANGE)
        return default_val; /* number out of range for LONG */
    return num;
}

static inline int32_t envi32(const char *name, int32_t default_val)
{
    int64_t num = envi64(name, (int64_t)default_val);
    if ((num < INT_MIN) || (num > INT_MAX))
        return default_val; /* number out of range for INT */
    return (int32_t)num;
}

static inline double envf64(const char *name, double default_val)
{
    const char *str = getenv(name);
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    double num = strtod(str, &stop);
    int error = errno;
    errno = 0;

    if (stop == str || stop != NULL)
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
static inline float envf32(const char *name, float default_val)
{
    const char *str = getenv(name);
    if (str == NULL)
        return default_val;

    char *stop = NULL;
    errno = 0;
    float num = strtof(str, &stop);
    int error = errno;
    errno = 0;

    if (stop == str || stop != NULL)
        return default_val; /* contains with non-number */
    if (error == ERANGE)
        return default_val; /* number out of range for FLOAT */
    if (num != num)
        return default_val; /* Not A Number */
    return num;
}
