#include "_private.h"
#include "../string.h"

MYS_API ssize_t mys_parse_readable_size(const char *text)
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

MYS_API void mys_readable_size(char **ptr, size_t bytes, size_t precision)
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

// https://stackoverflow.com/a/466242/11702338
static size_t _mys_round_ceil_2_power(size_t num)
{
    --num;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
#if SIZE_MAX == UINT64_MAX
    num |= num >> 32;
#endif
    return ++num;
}

MYS_API mys_string_t mys_string_create()
{
    mys_string_t str = (mys_string_t)malloc(sizeof(struct mys_string_struct));
    str->text = NULL;
    str->size = 0;
    str->capacity = 0;
    return str;
}

MYS_API void mys_string_destroy(mys_string_t str)
{
    if (str != NULL) {
        if (str->text != NULL)
            free(str->text);
        free(str);
    }
}

MYS_API void mys_string_fmt(mys_string_t str, const char *format, ...)
{
    va_list vargs, vargs_copy;
    va_start(vargs, format);

    va_copy(vargs_copy, vargs);
    int needed = vsnprintf(NULL, 0, format, vargs_copy) + 1; // Extra space for '\0'
    va_end(vargs_copy);
    if (needed <= 0)
        goto finish;

    if (str->capacity - str->size < (size_t)needed) {
        size_t new_capacity = _mys_round_ceil_2_power(str->capacity + (size_t)needed);
        char *new_text = (char *)realloc(str->text, new_capacity);
        if (new_text == NULL)
            goto finish;
        str->text = new_text;
        str->capacity = new_capacity;
    }
    str->size += vsnprintf(str->text + str->size, str->capacity - str->size, format, vargs);
finish:
    va_end(vargs);
}
