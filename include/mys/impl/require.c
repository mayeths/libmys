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
#include "../_config.h"
#include "../macro.h"

MYS_PUBLIC void mys_numa_capability_is_not_available(const char *function_name, const char *file, int line)
{
    printf(
        "%s:%d Error: to use %s, "
        "please #define macro MYS_ENABLE_NUMA before #include mys.h, "
        "and add -lnuma to linker.\n",
        file, line, function_name
    );
    exit(1);
}

MYS_PUBLIC void mys_debug_timeout_capability_is_not_available(const char *function_name, const char *file, int line)
{
    printf(
        "%s:%d Error: to use %s, "
        "please #define macro MYS_ENABLE_DEBUG_TIMEOUT before #include mys.h, "
        "and add -lrt to linker.\n",
        file, line, function_name
    );
    exit(1);
}
