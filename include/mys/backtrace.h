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
#include "macro.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 * @brief Get the backtrace of the current thread.
 * 
 * @param addrs the buffer to store the backtrace addresses
 * @param max_depth the maximum depth of the backtrace
 * @return the number of addresses stored in the buffer
 */
MYS_PUBLIC int mys_backtrace(void **addrs, int max_depth);
/**
 * @brief Get the source file and line number of a given address.
 * 
 * @param addr the address to look up
 * @param source the buffer to store the source file and line number
 * @param max_size the maximum size of the source buffer
 */
MYS_PUBLIC void mys_backtrace_source(void *addr, char *source, size_t max_size);
/**
 * @brief Get the symbol name of a given address.
 * 
 * @param addr the address to look up
 * @param symbol the buffer to store the symbol name
 * @param max_size the maximum size of the symbol buffer
 */
MYS_PUBLIC void mys_backtrace_symbol(void *addr, char *symbol, size_t max_size);
