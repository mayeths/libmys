#pragma once

#include "config.h"

#ifndef __cplusplus

/* https://stackoverflow.com/a/18298965 */
#ifndef thread_local
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG) || defined(COMPILER_ICC)
#define thread_local __thread
#elif defined(COMPILER_MSVC)
#define thread_local __declspec(thread)
#elif __STDC_VERSION__ >= 201112 && !defined(__STDC_NO_THREADS__)
#define thread_local _Thread_local
#else
#error "Cannot define thread_local"
#endif
#endif

#endif
