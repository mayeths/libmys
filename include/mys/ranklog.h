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

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "_config.h"
#include "macro.h"

/**
 * Print log message separately to a file named 'rank:6d.log' within the folder, like to folder/000001.log
```c
RANKLOG_OPEN("LOG.test");
RANKLOG("LOG.test", "setup time %f", setup_time);
RANKLOG("LOG.test", "solve time %f", solve_time);
RANKLOG_CLOSE("LOG.test");
```
 */
#define RANKLOG_OPEN(folder) mys_ranklog_open_old(__FILE__, __LINE__, folder) // Collective call
#define RANKLOG_CLOSE(folder) mys_ranklog_close_old(__FILE__, __LINE__, folder) // Collective call
#define RANKLOG(folder, fmt, ...) mys_ranklog_old(__FILE__, __LINE__, folder, fmt, ##__VA_ARGS__)

MYS_PUBLIC void mys_ranklog_open_old(const char *callfile, int callline, const char *folder);
MYS_PUBLIC void mys_ranklog_close_old(const char *callfile, int callline, const char *folder);
MYS_ATTR_PRINTF(4, 5) MYS_PUBLIC void mys_ranklog_old(const char *callfile, int callline, const char *folder, const char *fmt, ...);
