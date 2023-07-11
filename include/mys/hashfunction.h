// See https://github.com/B-Con/crypto-algorithms/blob/master/sha256.h
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "_config.h"
#include "macro.h"
#include "base64.h"

/**
 * @example Simplest way to get SHA256 result on data.
    char text[] = "hallo";
    char outhex[MYS_SHA256_HEX_SIZE];
    char outbase64[MYS_SHA256_BASE64_SIZE];
    mys_sha256_hex(text, sizeof(text) - 1, outhex); // no NULL '\0'
    mys_sha256_base64(text, sizeof(text) - 1, outbase64);

    printf("hex:    %s\n", outhex);
    printf("base64: %s\n", outbase64);

 * @example Update SHA256 context multiple times.
    char text[] = "hallo";
    char outhex[MYS_SHA256_HEX_SIZE];
    char outbase64[MYS_SHA256_BASE64_SIZE];
    mys_sha256_ctx_t ctx;
    mys_sha256_init(&ctx);
    mys_sha256_update(&ctx, text, sizeof(text) - 1); // no NULL '\0'
    mys_sha256_update(&ctx, text, sizeof(text) - 1);
    mys_sha256_update(&ctx, text, sizeof(text) - 1);

    mys_sha256_dump_base64(&ctx, outbase64);
    mys_sha256_dump_hex(&ctx, outhex);
    printf("hex:    %s\n", outhex);
    printf("base64: %s\n", outbase64);
 */


/**
 * @brief The size in bytes that can hold a SHA256 binary output
 */
#define MYS_SHA256_BIN_SIZE 32
/**
 * @brief The size in bytes that can hold a SHA256 base64 output
 */
#define MYS_SHA256_BASE64_SIZE (45 + 1)
/**
 * @brief The size in bytes that can hold a SHA256 hexadecimal output
 */
#define MYS_SHA256_HEX_SIZE (64 + 1)

/**
 * @brief Run SHA256 algorithm on text and output the binary result.
 * 
 * @param text The text to be hashed.
 * @param size Size of text in bytes.
 * @param outhex Output buffer. Must be larger than or equal to char[MYS_SHA256_BIN_SIZE].
 * 
 * @note mys_sha256_bin(), mys_sha256_hex() and mys_sha256_base64() are three versions that
 *       output sha256 context in binary, hex-text and base64-text format, with len(output) ==
 *       MYS_SHA256_BIN_SIZE(32), MYS_SHA256_BASE64_SIZE(46) and MYS_SHA256_HEX_SIZE(65) bytes.
 */
MYS_API void mys_sha256_bin(const void *text, size_t size, uint8_t output[MYS_SHA256_BIN_SIZE]);
/**
 * @brief Run SHA256 algorithm on text and output the base64-text result.
 * 
 * @param text The text to be hashed.
 * @param size Size of text in bytes.
 * @param outhex Output buffer. Must be larger than or equal to char[MYS_SHA256_BASE64_SIZE].
 * 
 * @note mys_sha256_bin(), mys_sha256_hex() and mys_sha256_base64() are three versions that
 *       output sha256 context in binary, hex-text and base64-text format, with len(output) ==
 *       MYS_SHA256_BIN_SIZE(32), MYS_SHA256_BASE64_SIZE(46) and MYS_SHA256_HEX_SIZE(65) bytes.
 */
MYS_API void mys_sha256_base64(const void *text, size_t size, char output[MYS_SHA256_BASE64_SIZE]);
/**
 * @brief Run SHA256 algorithm on text and output the hexadecimal-text result.
 * 
 * @param text The text to be hashed.
 * @param size Size of text in bytes.
 * @param outhex Output buffer. Must be larger than or equal to char[MYS_SHA256_HEX_SIZE].
 * 
 * @note mys_sha256_bin(), mys_sha256_hex() and mys_sha256_base64() are three versions that
 *       output sha256 context in binary, hex-text and base64-text format, with len(output) ==
 *       MYS_SHA256_BIN_SIZE(32), MYS_SHA256_BASE64_SIZE(46) and MYS_SHA256_HEX_SIZE(65) bytes.
 */
MYS_API void mys_sha256_hex(const void *text, size_t size, char output[MYS_SHA256_HEX_SIZE]);


typedef struct mys_sha256_ctx_t {
    unsigned char data[64];
    unsigned int datalen;
    unsigned long long bitlen;
    unsigned int state[8];
} mys_sha256_ctx_t;
/**
 * @brief Initialize a SHA256 algorithm context.
 * 
 * @param ctx The pointer to context struct.
 */
MYS_API void mys_sha256_init(mys_sha256_ctx_t *ctx);
/**
 * @brief Use data to update the internal states of SHA256 context.
 * 
 * @param ctx The pointer to context struct.
 * @param data The data used to update context.
 * @param size Size of data in bytes.
 */
MYS_API void mys_sha256_update(mys_sha256_ctx_t *ctx, const void *data, size_t size);
MYS_API void mys_sha256_update_i8(mys_sha256_ctx_t *ctx, const int8_t data);
MYS_API void mys_sha256_update_i16(mys_sha256_ctx_t *ctx, const int16_t data);
MYS_API void mys_sha256_update_i32(mys_sha256_ctx_t *ctx, const int32_t data);
MYS_API void mys_sha256_update_i64(mys_sha256_ctx_t *ctx, const int64_t data);
MYS_API void mys_sha256_update_u8(mys_sha256_ctx_t *ctx, const uint8_t data);
MYS_API void mys_sha256_update_u16(mys_sha256_ctx_t *ctx, const uint16_t data);
MYS_API void mys_sha256_update_u32(mys_sha256_ctx_t *ctx, const uint32_t data);
MYS_API void mys_sha256_update_u64(mys_sha256_ctx_t *ctx, const uint64_t data);
MYS_API void mys_sha256_update_f32(mys_sha256_ctx_t *ctx, const float data);
MYS_API void mys_sha256_update_f64(mys_sha256_ctx_t *ctx, const double data);
MYS_API void mys_sha256_update_int(mys_sha256_ctx_t *ctx, const int data);
MYS_API void mys_sha256_update_float(mys_sha256_ctx_t *ctx, const float data);
MYS_API void mys_sha256_update_double(mys_sha256_ctx_t *ctx, const double data);
MYS_API void mys_sha256_update_char(mys_sha256_ctx_t *ctx, const char data);
MYS_API void mys_sha256_update_ptr(mys_sha256_ctx_t *ctx, const void *data);
MYS_API void mys_sha256_update_arr(mys_sha256_ctx_t *ctx, const void *data, size_t size);

/**
 * @brief Dump the SHA256 result to hexadecimal.
 * 
 * @param ctx The pointer to context struct.
 * @param outhex Output buffer. Must be larger than or equal to char[MYS_SHA256_HEX_SIZE].
 * 
 * @note The mys_sha256_dump_base64() and mys_sha256_dump_bin() are the alternative versions
 *       to output in different format, with buffer length must larger than or equal to
 *       MYS_SHA256_BASE64_SIZE and MYS_SHA256_BIN_SIZE.
 */
MYS_API void mys_sha256_dump_hex(mys_sha256_ctx_t *ctx, void *outhex);
MYS_API void mys_sha256_dump_base64(mys_sha256_ctx_t *ctx, void *outbase64);
MYS_API void mys_sha256_dump_bin(mys_sha256_ctx_t *ctx, void *outbin);
