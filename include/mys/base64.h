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
// See https://opensource.apple.com/source/QuickTimeStreamingServer/QuickTimeStreamingServer-452/CommonUtilitiesLib/base64.h
#pragma once
#include <string.h>
#include <stdint.h>
#include "_config.h"
#include "macro.h"

/**
 * @example
    char stext[] = "abcd\0!";
    char *btext = NULL;
    char *dtext = NULL;

    size_t elen = mys_base64_encode_len(sizeof(stext));
    btext = (char *)calloc(elen, sizeof(char));
    mys_base64_encode(btext, elen, stext, sizeof(stext));

    size_t dlen = mys_base64_decode_len(btext);
    dtext = (char *)calloc(dlen, sizeof(char));
    mys_base64_decode(dtext, dlen, btext, strnlen(btext, 65536));

    printf("slen=%2zu |%s|\n", sizeof(stext), stext);
    printf("elen=%2zu |%s|\n", elen, btext);
    printf("dlen=%2zu |%s| (%c)\n", dlen, dtext, dtext[5]);

    free(btext);
    free(dtext);
 */

/**
 * @brief Calculate the required size to encode a plain text.
 * 
 * @param plain_size The size of plain text.
 * @return The size of buffer that can successfully hold the encode output.
 * @note For text messages, passing sizeof(text) which count the tailing '\0', the
 *       return value will be slightly larger than it need to encode the text.
 *       Passing strlen(text) will return the precisely value, but both cases are safe.
 *       Be aware that raw stream can contains arbitrary ascii character, including '\0'.
 *       So do not use strlen(text) on binary messages.
 */
MYS_PUBLIC size_t mys_base64_encode_len(size_t stream_size);
/**
 * @brief Traverse the base64-encoded text to calculate the required size to decode it.
 * 
 * @param str The encoded text.
 * @return The size of buffer that can successfully hold the decode output.
 */
MYS_PUBLIC size_t mys_base64_decode_len(const char *str);
/**
 * @brief Encode the source string with base64
 * 
 * @param dst the encode output buffer
 * @param dst_size the size of output buffer
 * @param src the source to be encoded
 * @param src_size the size of source string
 * @return The size of content written to dst, not including the tailing '\0', same as strlen(dst)
 */
MYS_PUBLIC size_t mys_base64_encode(char *dst, size_t dst_size, const void *src, size_t src_size);
/**
 * @brief Decode the base64-encoded string
 * 
 * @param dst the decode output buffer
 * @param dst_size the size of output buffer
 * @param src the source to be decoded
 * @param src_size the size of source string
 * @return The size of content written to dst, not including the tailing '\0', same as strlen(dst)
 */
MYS_PUBLIC size_t mys_base64_decode(void *dst, size_t dst_size, const char *src, size_t src_size);

