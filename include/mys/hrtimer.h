#pragma once

#include <stdint.h>
#include "macro.h"

#if defined(ARCH_X64)
#error Not implemented

#elif defined(ARCH_AARCH64)
#define HRNAME "AArch64 asm high-resolution timer"
#define HRTICK_T uint64_t
#define HRTICK(v) __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(v));
#define HRFREQ(v) __asm__ __volatile__("mrs %0, CNTFRQ_EL0" : "=r"(v));

#else
#error Not implemented. Implement with time.h in the future
#endif /*if defined(__x86_64__)*/

static inline const char *hrname() { return HRNAME; }
static inline double hrfreq() { HRTICK_T f; HRFREQ(f); return (double)f; }
static inline double hrtime() { HRTICK_T t; HRTICK(t); return (double)t / hrfreq(); }
static inline double hrdiff(double t0) { return hrtime() - t0; }
