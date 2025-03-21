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
#pragma once

#include <cuda.h>

#include "assert.h"

#define CHKCUDA(fncall) do {                  \
    cudaError_t val = (cudaError_t)(fncall);  \
    ASSERT(val == cudaSuccess,                \
        "Expect (%s) return %d but %d. (%s)", \
        #fncall, (int)cudaSuccess, (int)val,  \
        cudaGetErrorString(val));             \
} while (0)

template<typename T>
static cudaError_t cudaCalloc(T **ptr, size_t nmemb, size_t size) { 
    cudaError_t err = cudaSuccess;
    err = cudaMalloc((void **)ptr, nmemb * size);
    if (err != cudaSuccess) return err;
    err = cudaMemset((void *)*ptr, 0, nmemb * size);
    return err;
}

template<typename T>
void cudaDuplicate(T **dst, T *src, size_t nmemb, size_t size, cudaMemcpyKind kind)
{
    const size_t nbytes = nmemb * size;
    if (kind == cudaMemcpyHostToHost || kind == cudaMemcpyDeviceToHost) {
        CHKPTR(*dst = (T *)malloc(nbytes));
    } else if (kind == cudaMemcpyHostToDevice || kind == cudaMemcpyDeviceToDevice) {
        CHKCUDA(cudaMalloc((void **)dst, nbytes));
    }
    CHKCUDA(cudaMemcpy((void *)*dst, src, nbytes, kind));
}
