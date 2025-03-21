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
#include "../base64.h"

static const char _mys_base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static const unsigned char _mys_base64_map[256] =
{
    /* ASCII table */
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

MYS_PUBLIC size_t mys_base64_encode_len(size_t plain_size)
{
    return ((plain_size + 2) / 3 * 4) + 1;
}

MYS_PUBLIC size_t mys_base64_decode_len(const char *encoded_src)
{
    size_t nbytesdecoded;
    const unsigned char *s;
    size_t nprbytes;

    s = (const unsigned char *)encoded_src;
    while (_mys_base64_map[*(s++)] <= 63)
        ;

    nprbytes      = (s - (const unsigned char *)encoded_src) - 1;
    nbytesdecoded = ((nprbytes + 3) / 4) * 3;

    return nbytesdecoded + 1;
}

#define _PCHK() do {                      \
    /* Save 1 char for NULL end */        \
    size_t pos = dst_size - 1;            \
    if ((char *)d >= (char *)dst + pos) { \
        goto finish;                      \
    }                                     \
} while (0)

MYS_PUBLIC size_t mys_base64_encode(char *dst, size_t dst_size, const void *src, size_t src_size)
{
    const char *tab = _mys_base64_table;
    char *d;
    char *s;
    size_t i;

    if (dst_size == 0 || src_size == 0)
        return 0;

    i = 0;
    s = (char *)src;
    d = dst;
    if (src_size > 2) {
        for (i = 0; i < src_size - 2; i += 3) {
            _PCHK(); *d++ = tab[(s[i] >> 2) & 0x3F];
            _PCHK(); *d++ = tab[((s[i] & 0x3) << 4) | ((int)(s[i + 1] & 0xF0) >> 4)];
            _PCHK(); *d++ = tab[((s[i + 1] & 0xF) << 2) | ((int)(s[i + 2] & 0xC0) >> 6)];
            _PCHK(); *d++ = tab[s[i + 2] & 0x3F];
        }
    }
    _PCHK();

    if (i < src_size) {
        *d++ = tab[(s[i] >> 2) & 0x3F];
        if (i == (src_size - 1)) {
            _PCHK(); *d++ = tab[((s[i] & 0x3) << 4)];
            _PCHK(); *d++ = '=';
        } else {
            _PCHK(); *d++ = tab[((s[i] & 0x3) << 4) | ((int)(s[i + 1] & 0xF0) >> 4)];
            _PCHK(); *d++ = tab[((s[i + 1] & 0xF) << 2)];
        }
        _PCHK(); *d++ = '=';
    }

finish:
    if (d >= dst + dst_size)
        d = dst + dst_size - 1;
    *d = '\0';
    return d - dst;
}

MYS_PUBLIC size_t mys_base64_decode(void *dst, size_t dst_size, const char *src, size_t src_size)
{
    const unsigned char *map = _mys_base64_map;
    typedef unsigned char u8_t;
    const u8_t *s;
    u8_t *d;
    size_t nprbytes;

    if (dst_size == 0 || src_size == 0)
        return 0;

    d = (u8_t *)dst;
    s = (const u8_t *)src;
    while (map[*(s++)] <= 63)
        ;

    nprbytes = (s - (const u8_t *)src) - 1;
    s  = (const u8_t *)src;

    while (nprbytes > 4) {
        _PCHK(); *(d++) = (u8_t)(map[*s] << 2 | map[s[1]] >> 4);
        _PCHK(); *(d++) = (u8_t)(map[s[1]] << 4 | map[s[2]] >> 2);
        _PCHK(); *(d++) = (u8_t)(map[s[2]] << 6 | map[s[3]]);
        s += 4;
        nprbytes -= 4;
    }
    _PCHK();

    /* Note: (nprbytes == 1) would be an error, so just ingore that case */
    if (nprbytes > 1) {
        _PCHK(); *(d++) = (u8_t)(map[*s] << 2 | map[s[1]] >> 4);
    }
    if (nprbytes > 2) {
        _PCHK(); *(d++) = (u8_t)(map[s[1]] << 4 | map[s[2]] >> 2);
    }
    if (nprbytes > 3) {
        _PCHK(); *(d++) = (u8_t)(map[s[2]] << 6 | map[s[3]]);
    }
    _PCHK();

finish:
    if (d >= (u8_t *)dst + dst_size)
        d = (u8_t *)dst + dst_size - 1;
    *d = '\0';
    return d - (u8_t *)dst;
}

#undef _PCHK
