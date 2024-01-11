#include "../hash.h"

#define _ROTL(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define _ROTR(a,b) (((a) >> (b)) | ((a) << (32-(b))))
#define _CH(x,y,z)  (((x) & (y)) ^ (~(x) & (z)))
#define _MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define _EP0(x)     (_ROTR(x,2) ^ _ROTR(x,13) ^ _ROTR(x,22))
#define _EP1(x)     (_ROTR(x,6) ^ _ROTR(x,11) ^ _ROTR(x,25))
#define _SIG0(x)    (_ROTR(x,7) ^ _ROTR(x,18) ^ ((x) >> 3))
#define _SIG1(x)    (_ROTR(x,17) ^ _ROTR(x,19) ^ ((x) >> 10))

static const unsigned int _mys_sha256_k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

static void _mys_sha256_trans(mys_sha256_ctx_t *ctx, const unsigned char data[])
{
    unsigned int a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = _SIG1(m[i - 2]) + m[i - 7] + _SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + _EP1(e) + _CH(e,f,g) + _mys_sha256_k[i] + m[i];
        t2 = _EP0(a) + _MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

MYS_API void mys_sha256_init(mys_sha256_ctx_t *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

MYS_API void mys_sha256_update(mys_sha256_ctx_t *ctx, const void *data, size_t len)
{
    unsigned int i;
    const unsigned char *raw = (const unsigned char *)data;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = raw[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            _mys_sha256_trans(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

MYS_API void mys_sha256_update_i8(mys_sha256_ctx_t *ctx, const int8_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int8_t));
}
MYS_API void mys_sha256_update_i16(mys_sha256_ctx_t *ctx, const int16_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int16_t));
}
MYS_API void mys_sha256_update_i32(mys_sha256_ctx_t *ctx, const int32_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int32_t));
}
MYS_API void mys_sha256_update_i64(mys_sha256_ctx_t *ctx, const int64_t data)
{
    mys_sha256_update(ctx, &data, sizeof(int64_t));
}
MYS_API void mys_sha256_update_u8(mys_sha256_ctx_t *ctx, const uint8_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint8_t));
}
MYS_API void mys_sha256_update_u16(mys_sha256_ctx_t *ctx, const uint16_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint16_t));
}
MYS_API void mys_sha256_update_u32(mys_sha256_ctx_t *ctx, const uint32_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint32_t));
}
MYS_API void mys_sha256_update_u64(mys_sha256_ctx_t *ctx, const uint64_t data)
{
    mys_sha256_update(ctx, &data, sizeof(uint64_t));
}
MYS_API void mys_sha256_update_f32(mys_sha256_ctx_t *ctx, const float data)
{
    mys_sha256_update(ctx, &data, sizeof(float));
}
MYS_API void mys_sha256_update_f64(mys_sha256_ctx_t *ctx, const double data)
{
    mys_sha256_update(ctx, &data, sizeof(double));
}
MYS_API void mys_sha256_update_int(mys_sha256_ctx_t *ctx, const int data)
{
    mys_sha256_update(ctx, &data, sizeof(int));
}
MYS_API void mys_sha256_update_float(mys_sha256_ctx_t *ctx, const float data)
{
    mys_sha256_update(ctx, &data, sizeof(float));
}
MYS_API void mys_sha256_update_double(mys_sha256_ctx_t *ctx, const double data)
{
    mys_sha256_update(ctx, &data, sizeof(double));
}
MYS_API void mys_sha256_update_char(mys_sha256_ctx_t *ctx, const char data)
{
    mys_sha256_update(ctx, &data, sizeof(char));
}
MYS_API void mys_sha256_update_ptr(mys_sha256_ctx_t *ctx, const void *data)
{
    mys_sha256_update(ctx, &data, sizeof(void *));
}
MYS_API void mys_sha256_update_arr(mys_sha256_ctx_t *ctx, const void *data, size_t size)
{
    mys_sha256_update(ctx, data, size);
}

MYS_API void mys_sha256_dump_bin(mys_sha256_ctx_t *ctx, void *outbin)
{
    mys_sha256_ctx_t ictx;
    memcpy(&ictx, ctx, sizeof(mys_sha256_ctx_t));

    unsigned int i;
    unsigned char *hash = (unsigned char *)outbin;

    i = ictx.datalen;

    // Pad whatever data is left in the buffer.
    if (ictx.datalen < 56) {
        ictx.data[i++] = 0x80;
        while (i < 56)
            ictx.data[i++] = 0x00;
    }
    else {
        ictx.data[i++] = 0x80;
        while (i < 64)
            ictx.data[i++] = 0x00;
        _mys_sha256_trans(&ictx, ictx.data);
        memset(ictx.data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ictx.bitlen += ictx.datalen * 8;
    ictx.data[63] = ictx.bitlen;
    ictx.data[62] = ictx.bitlen >> 8;
    ictx.data[61] = ictx.bitlen >> 16;
    ictx.data[60] = ictx.bitlen >> 24;
    ictx.data[59] = ictx.bitlen >> 32;
    ictx.data[58] = ictx.bitlen >> 40;
    ictx.data[57] = ictx.bitlen >> 48;
    ictx.data[56] = ictx.bitlen >> 56;
    _mys_sha256_trans(&ictx, ictx.data);

    // Since this implementation uses little endian byte ordering and SHA uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ictx.state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ictx.state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ictx.state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ictx.state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ictx.state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ictx.state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ictx.state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ictx.state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

static const char _mys_hex_table[] = "0123456789abcdef";

MYS_API void mys_sha256_dump_hex(mys_sha256_ctx_t *ctx, void *outhex)
{
    uint8_t outbin[MYS_SHA256_BIN_SIZE];
    mys_sha256_dump_bin(ctx, outbin);
    char *out = (char *)outhex;

    for (int i = 0; i < MYS_SHA256_BIN_SIZE; i++) {
        const char byte = outbin[i];
        uint32_t hi = (byte & 0xF0) >> 4;
        uint32_t lo = (byte & 0x0F) >> 0;
        *(out++) = _mys_hex_table[hi];
        *(out++) = _mys_hex_table[lo];
    }
    *out = '\0';
}

MYS_API void mys_sha256_dump_base64(mys_sha256_ctx_t *ctx, void *outbase64)
{
    uint8_t outbin[MYS_SHA256_BIN_SIZE];
    mys_sha256_dump_bin(ctx, outbin);
    mys_base64_encode((char *)outbase64, MYS_SHA256_BASE64_SIZE, outbin, MYS_SHA256_BIN_SIZE);
}

MYS_API void mys_sha256_bin(const void *text, size_t size, uint8_t output[MYS_SHA256_BIN_SIZE])
{
    mys_sha256_ctx_t ctx;
    mys_sha256_init(&ctx);
    mys_sha256_update(&ctx, text, size);
    mys_sha256_dump_bin(&ctx, (void *)output);
}

MYS_API void mys_sha256_base64(const void *text, size_t size, char output[MYS_SHA256_BASE64_SIZE])
{
    mys_sha256_ctx_t ctx;
    mys_sha256_init(&ctx);
    mys_sha256_update(&ctx, text, size);
    mys_sha256_dump_base64(&ctx, (void *)output);
}

MYS_API void mys_sha256_hex(const void *text, size_t size, char output[MYS_SHA256_HEX_SIZE])
{
    mys_sha256_ctx_t ctx;
    mys_sha256_init(&ctx);
    mys_sha256_update(&ctx, text, size);
    mys_sha256_dump_hex(&ctx, (void *)output);
}
