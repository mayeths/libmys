#pragma once

#include "assert.h"

#define CHKCUDA(fncall) do {                  \
    cudaError_t val = (cudaError_t)(fncall);  \
    ASSERT(val == cudaSuccess,                \
        "Expect (%s) return %d but %d. (%s)", \
        #fncall, (int)cudaSuccess, (int)val,  \
        cudaGetErrorString(val));             \
} while (0)
