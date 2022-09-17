#pragma once
#ifndef MYS_MACRO_H_
#define MYS_MACRO_H_

/* Compiler Detection */
#if defined(__GNUC__)
#define COMPILER_NAME "GCC"
#define COMPILER_GCC
#elif defined(__clang__)
#define COMPILER_NAME "Clang"
#define COMPILER_CLANG
#define COMPILER_GCC
#elif defined(_MSC_VER)
#define COMPILER_NAME "MSVC"
#define COMPILER_MSVC
#elif defined(__NVCC__)
#define COMPILER_NAME "NVCC"
#define COMPILER_NVCC
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
#else
#define OS_NAME "Unknown"
#define OS_UNKNOWN
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
#elif defined(__ia64__) || defined(_M_IA64)
#define ARCH_NAME "IA64"
#define ARCH_IA64
#else
#define ARCH_NAME "Unknown"
#define ARCH_UNKNOWN
#endif

#endif /*MYS_MACRO_H_*/