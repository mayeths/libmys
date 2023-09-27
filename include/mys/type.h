#pragma once

#include <stdint.h>
#include <stdlib.h>

// https://aticleworld.com/little-and-big-endian-importance
union _mys_endian_t { uint32_t u32; uint8_t u8[4]; };
static const union _mys_endian_t _mys_endian_check = { .u32 = 1 };
#define _MYS_LITTLE_ENDIAN() (_mys_endian_check.u8[0] == 1)
#define _MYS_BIG_ENDIAN() (_mys_endian_check.u8[0] == 0)
