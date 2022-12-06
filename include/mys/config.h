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
#elif defined(__NVCC__)
#define COMPILER_NAME "NVCC"
#define COMPILER_NVCC
#elif defined(__SWCC__)
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
#elif defined(_WIN32)
#define OS_NAME "Windows"
#define OS_WINDOWS
#elif defined(__APPLE__)
#define OS_NAME "MacOS"
#define OS_MACOS
#elif defined(__ANDROID__)
#define OS_NAME "Android"
#define OS_ANDROID
#define OS_LINUX
#else
#define OS_NAME "Unknown"
#define OS_UNKNOWN
#endif

#if defined(OS_LINUX) || defined(OS_MACOS) || defined(OS_ANDROID)
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
#elif defined(__SWCC__)
#define ARCH_NAME "Sunway"
#define ARCH_SUNWAY
#else
#define ARCH_NAME "Unknown"
#define ARCH_UNKNOWN
#endif

/* See cJSON */
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG) || defined(COMPILER_ICC) || defined(COMPILER_NVCC) || defined(COMPILER_SWCC)
#define MYS_API __attribute__((unused)) /*__attribute__((visibility("default")))*/
#elif defined(COMPILER_MSVC)
#define MYS_API /*__declspec(dllexport)*/
#else
#define MYS_API /* Emit nothing */
#endif

#define REP2(inst) do { inst; inst; } while (0)
#define REP4(inst) do { REP2(inst); REP2(inst); } while (0)
#define REP8(inst) do { REP4(inst); REP4(inst); } while (0)
#define REP16(inst) do { REP8(inst); REP8(inst); } while (0)
#define REP32(inst) do { REP16(inst); REP16(inst); } while (0)
#define REP64(inst) do { REP32(inst); REP32(inst); } while (0)
#define REP128(inst) do { REP64(inst); REP64(inst); } while (0)
#define REP256(inst) do { REP128(inst); REP128(inst); } while (0)
#define REP512(inst) do { REP256(inst); REP256(inst); } while (0)
#define REP1024(inst) do { REP512(inst); REP512(inst); } while (0)

#define REP5(inst) do { inst; inst; inst; inst; inst; } while (0)
#define REP10(inst) do { REP5(inst); REP5(inst); } while (0)
#define REP50(inst) do { REP10(inst); REP10(inst); REP10(inst); REP10(inst); REP10(inst); } while (0)
#define REP100(inst) do { REP50(inst); REP50(inst); } while (0)
#define REP500(inst) do { REP100(inst); REP100(inst); REP100(inst); REP100(inst); REP100(inst); } while (0)
#define REP1000(inst) do { REP500(inst); REP500(inst); } while (0)
