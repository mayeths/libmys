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

#include "_config.h"

MYS_PUBLIC void mys_numa_capability_is_not_available(const char *function_name, const char *file, int line)
    MYS_DEPRECATED("You are calling APIs that rely on (1) #define macro MYS_ENABLE_NUMA before #include mys.h, (2) add -lnuma to linker.");
MYS_PUBLIC void mys_debug_timeout_capability_is_not_available(const char *function_name, const char *file, int line)
    MYS_DEPRECATED("You are calling APIs that rely on (1) #define macro MYS_ENABLE_DEBUG_TIMEOUT before #include mys.h, (2) add -lrt to linker.");
