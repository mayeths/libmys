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

#include "_config.h"
#include "macro.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct mys_backtrace_t {
    void *pc;              // Program counter, represents the address of the instruction in the code
    void *function;        // Pointer to the function in which the backtrace was captured
    char symbol[128];      // Symbolic name of the function, usually a demangled function name
    char source[128];      // The source file and line where the function is located
} mys_backtrace_t;

MYS_PUBLIC int mys_backtrace(mys_backtrace_t *buffer, int max_depth);
