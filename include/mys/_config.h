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
#pragma once

/* https://github.com/abseil/abseil.github.io/blob/master/docs/cpp/platforms/macros.md */

/* Compiler Detection */
#if defined(__GNUC__)
#define COMPILER_NAME "GCC"
#define COMPILER_GCC
#elif defined(__clang__)
#define COMPILER_NAME "Clang"
#define COMPILER_CLANG
#define COMPILER_GCC
#elif defined(__INTEL_COMPILER)
#define COMPILER_NAME "ICC"
#define COMPILER_ICC
#elif defined(_MSC_VER)
#define COMPILER_NAME "MSVC"
#define COMPILER_MSVC
#elif defined(__CYGWIN__)
#define COMPILER_NAME "CYGWIN"
#define COMPILER_CYGWIN
#elif defined(__NVCC__)
#define COMPILER_NAME "NVCC"
#define COMPILER_NVCC
#elif defined(__sw_64__)
#define COMPILER_NAME "SWCC"
#define COMPILER_SWCC
#else
#define COMPILER_NAME "Unknown"
#define COMPILER_UNKNOWN
#endif

/* OS Detection */
#if defined(__linux__)
#define OS_NAME "Linux"
#define OS_LINUX
#elif defined(__ANDROID__)
#define OS_NAME "Android"
#define OS_ANDROID
#define OS_LINUX
#elif defined(__FreeBSD__)
#define OS_NAME "FreeBSD"
#define OS_FREEBSD
#define OS_BSD
#elif defined(__OpenBSD__)
#define OS_NAME "OpenBSD"
#define OS_OPENBSD
#define OS_BSD
#elif defined(__APPLE__)
#define OS_NAME "MacOS"
#define OS_MACOS
#define OS_BSD
#elif defined(_WIN32)
#define OS_NAME "Windows"
#define OS_WINDOWS
#else
#define OS_NAME "Unknown"
#define OS_UNKNOWN
#endif

#if defined(OS_LINUX) || defined(OS_BSD)
#define POSIX_COMPLIANCE
#endif

/* Arch Detection */
#if defined(__x86_64__) || defined(_M_X64)
#define ARCH_NAME "x64"
#define ARCH_X64
#elif defined(__i386__) || defined(_M_IX86)
#define ARCH_NAME "x86"
#define ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_NAME "AArch64"
#define ARCH_AARCH64
#elif defined(__arm__) || defined(_M_ARM)
#define ARCH_NAME "Arm32"
#define ARCH_ARM32
#elif defined(__sw_64__)
#define ARCH_NAME "Sunway"
#define ARCH_SUNWAY
#else
#define ARCH_NAME "Unknown"
#define ARCH_UNKNOWN
#endif

#if defined(COMPILER_GCC) && !defined(COMPILER_CLANG)
#define MYS_ATTR_EXPORT __attribute__((visibility("default")))
#define MYS_ATTR_IMPORT __attribute__((visibility("default")))
#define MYS_ATTR_LOCAL  __attribute__((visibility("hidden")))
#define MYS_ATTR_UNUSED __attribute__((unused))
#define MYS_ATTR_NO_INSTRUMENT __attribute__((no_instrument_function))
#define MYS_ATTR_NOINLINE __attribute__((noinline))
#define MYS_ATTR_ALWAYS_INLINE __attribute__((always_inline)) inline
#define MYS_LIKELY(x) __builtin_expect(!!(x), 1)
#define MYS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#ifdef OS_MACOS
#define MYS_ATTR_OPTIMIZE_O3 /* Not available */
#else
#define MYS_ATTR_OPTIMIZE_O3 __attribute__((optimize("O3")))
#endif
#elif defined(COMPILER_CLANG)
#define MYS_ATTR_EXPORT __attribute__((visibility("default")))
#define MYS_ATTR_IMPORT __attribute__((visibility("default")))
#define MYS_ATTR_LOCAL  __attribute__((visibility("hidden")))
#define MYS_ATTR_UNUSED __attribute__((unused))
#define MYS_ATTR_NO_INSTRUMENT __attribute__((no_instrument_function))
#define MYS_ATTR_NOINLINE __attribute__((noinline))
#define MYS_ATTR_ALWAYS_INLINE __attribute__((always_inline)) inline
#define MYS_LIKELY(x) __builtin_expect(!!(x), 1)
#define MYS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define MYS_ATTR_OPTIMIZE_O3 /* No equivalent attribute */
#elif defined(COMPILER_ICC)
#define MYS_ATTR_EXPORT __attribute__((visibility("default")))
#define MYS_ATTR_IMPORT __attribute__((visibility("default")))
#define MYS_ATTR_LOCAL  __attribute__((visibility("hidden")))
#define MYS_ATTR_UNUSED __attribute__((unused))
#define MYS_ATTR_NO_INSTRUMENT /* No equivalent attribute */
#define MYS_ATTR_NOINLINE __attribute__((noinline))
#define MYS_ATTR_ALWAYS_INLINE __attribute__((always_inline)) inline
#define MYS_LIKELY(x) __builtin_expect(!!(x), 1)
#define MYS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define MYS_ATTR_OPTIMIZE_O3 __attribute__((optimize("O3")))
#elif defined(COMPILER_NVCC)
#define MYS_ATTR_EXPORT __attribute__((visibility("default")))
#define MYS_ATTR_IMPORT __attribute__((visibility("default")))
#define MYS_ATTR_LOCAL  __attribute__((visibility("hidden")))
#define MYS_ATTR_UNUSED __attribute__((unused))
#define MYS_ATTR_NO_INSTRUMENT /* No equivalent attribute */
#define MYS_ATTR_NOINLINE /* No equivalent attribute */
#define MYS_ATTR_ALWAYS_INLINE __forceinline__
#define MYS_LIKELY(x) (x) /* No equivalent attribute */
#define MYS_UNLIKELY(x) (x) /* No equivalent attribute */
#define MYS_ATTR_OPTIMIZE_O3 /* No equivalent attribute */
#elif defined(COMPILER_SWCC)
#define MYS_ATTR_EXPORT __attribute__((visibility("default")))
#define MYS_ATTR_IMPORT __attribute__((visibility("default")))
#define MYS_ATTR_LOCAL  __attribute__((visibility("hidden")))
#define MYS_ATTR_UNUSED __attribute__((unused))
#define MYS_ATTR_NO_INSTRUMENT /* No equivalent attribute */
#define MYS_ATTR_NOINLINE /* No equivalent attribute */
#define MYS_ATTR_ALWAYS_INLINE /* No equivalent attribute */
#define MYS_LIKELY(x) (x) /* No equivalent attribute */
#define MYS_UNLIKELY(x) (x) /* No equivalent attribute */
#define MYS_ATTR_OPTIMIZE_O3 /* No equivalent attribute */
#elif defined(COMPILER_MSVC)
#define MYS_ATTR_EXPORT __declspec(dllexport)
#define MYS_ATTR_IMPORT __declspec(dllimport)
#define MYS_ATTR_LOCAL /* No equivalent attribute */
#define MYS_ATTR_UNUSED /* No equivalent attribute */
#define MYS_ATTR_NO_INSTRUMENT /* No equivalent attribute */
#define MYS_ATTR_NOINLINE __declspec(noinline)
#define MYS_ATTR_ALWAYS_INLINE __forceinline
#define MYS_LIKELY(x) (x) /* No equivalent attribute */
#define MYS_UNLIKELY(x) (x) /* No equivalent attribute */
#define MYS_ATTR_OPTIMIZE_O3 /* No equivalent attribute */
#else /* Fallback for unknown compilers */
#define MYS_ATTR_EXPORT
#define MYS_ATTR_IMPORT
#define MYS_ATTR_LOCAL
#define MYS_ATTR_UNUSED
#define MYS_ATTR_NO_INSTRUMENT
#define MYS_ATTR_NOINLINE
#define MYS_ATTR_ALWAYS_INLINE inline
#define MYS_LIKELY(x) (x) /* No equivalent attribute */
#define MYS_UNLIKELY(x) (x) /* No equivalent attribute */
#define MYS_ATTR_OPTIMIZE_O3 /* No equivalent attribute */
#endif

#if defined(MYS_IMPL)         // Make libmys with public visibility.
#define MYS_PUBLIC   MYS_ATTR_UNUSED MYS_ATTR_EXPORT extern
#define MYS_INTERNAL MYS_ATTR_UNUSED MYS_ATTR_LOCAL extern
#define MYS_STATIC   MYS_ATTR_UNUSED static
#elif defined(MYS_IMPL_LOCAL) // Make libmys with private visibility.
#define MYS_PUBLIC   MYS_ATTR_UNUSED MYS_ATTR_LOCAL extern
#define MYS_INTERNAL MYS_ATTR_UNUSED MYS_ATTR_LOCAL extern
#define MYS_STATIC   MYS_ATTR_UNUSED static
#else
#define MYS_PUBLIC   MYS_ATTR_UNUSED MYS_ATTR_IMPORT extern
#define MYS_INTERNAL MYS_ATTR_UNUSED MYS_ATTR_LOCAL extern
#define MYS_STATIC   MYS_ATTR_UNUSED static
#endif


/**
 * @brief Use MYS_IMPL and MYS_IMPL_LOCAL to control the visibility of libmys.
 * 
 * @note
 *       Define MYS_IMPL to build a copy libmys and export its symbols to other dynamic
 *              shared objects (DSOs) and the executable (EXE).
 *       Define MYS_IMPL_LOCAL to build a copy of libmys and hide its symbols, so this DSO
 *              or EXE won't use the libmys export by other DSO or EXE, who export symbols
 *              with defining MYS_IMPL.
 *       No definition for both MYS_IMPL and MYS_IMPL_LOCAL will use the libmys export by
 *              other DSO or EXE that export symbols by defining MYS_IMPL.
 * 
 * @example The following code snippets will build two libmys, one for the main executable
 *          and liba, and another for libb. You can see changing logging level in main only
 *          affect the output of main and liba, but leaving libb alone.
 * 
 * <test-api-main.c>
    #include <mys.h>
    void a_hello();
    void b_hello();
    int main() {
        MPI_Init(NULL, NULL);
        DLOG(0, "Hello from main (level %d)!", mys_log_get_level());
        a_hello();
        b_hello();
        mys_log_set_level(MYS_LOG_DEBUG);
        DLOG(0, "Hello from main (level %d)!", mys_log_get_level());
        a_hello();
        b_hello();
        MPI_Finalize();
        return 0;
    }

 * <test-api-liba.c>
    #define MYS_IMPL
    #include <mys.h>
    void a_hello()
    {
        DLOG(0, "Hello from a (level %d)!", mys_log_get_level());
    }

 * <test-api-libb.c>
    #define MYS_IMPL_LOCAL
    #include <mys.h>
    void b_hello()
    {
        DLOG(0, "Hello from b (level %d)!", mys_log_get_level());
    }

 * <Compile & Run sequence>
    mpicc -Wall -Wextra -Werror -I${MYS_DIR}/include -c test-api-main.c
    mpicc -Wall -Wextra -Werror -I${MYS_DIR}/include -c test-api-liba.c
    mpicc -Wall -Wextra -Werror -I${MYS_DIR}/include -c test-api-libb.c
    mpicc -Wall -Wextra -Werror -dynamiclib -o liba.dylib test-api-liba.o
    mpicc -Wall -Wextra -Werror -dynamiclib -o libb.dylib test-api-libb.o
    mpicc -Wall -Wextra -Werror test-api-main.o -L. -la -lb
    mpirun -n 1 ./a.out

 * <Expected output>
    [D::000 test-api-main.c:010] Hello from main (level 0)!
    [D::000 test-api-liba.c:008] Hello from a (level 0)!
    [D::000 test-api-libb.c:008] Hello from b (level 0)!
    [D::000 test-api-main.c:014] Hello from main (level 1)!
    [D::000 test-api-liba.c:008] Hello from a (level 1)!
    [D::000 test-api-libb.c:008] Hello from b (level 0)!
 */
