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

// typedef struct mys_trace_event0_t {
//     double time;
// } mys_trace_event0_t;

typedef struct mys_trace_event1_t {
    double time;
    union {
        uint64_t u1;
        int64_t i1;
        double d1;
        void *p1;
        struct {          int li1;          int hi1; }; // Do not change order of l and h
        struct { unsigned int lu1; unsigned int hu1; }; // Do not change order of l and h
        struct {        float lf1;        float hf1; }; // Do not change order of l and h
    };
} mys_trace_event1_t;

enum { MYS_TRACE_EVENT0, MYS_TRACE_EVENT1, MYS_TRACE_EVENT2, MYS_TRACE_EVENT3, MYS_TRACE_EVENT4 };

typedef struct mys_trace_iter_t {
    union {
        // mys_trace_event0_t *e0;
        mys_trace_event1_t *e1;
    };
    int type;
} mys_trace_iter_t;


typedef struct mys_trace_t mys_trace_t;

MYS_PUBLIC mys_trace_t *mys_trace_create();
MYS_PUBLIC void mys_trace_destroy(mys_trace_t **trace);

MYS_PUBLIC mys_trace_iter_t *mys_trace_start_iter(mys_trace_t *trace);
MYS_PUBLIC mys_trace_iter_t *mys_trace_next_iter(mys_trace_t *trace, mys_trace_iter_t *iter);
MYS_PUBLIC void mys_trace_interrupt_iter(mys_trace_t *trace, mys_trace_iter_t *iter);
// MYS_PUBLIC void mys_trace(mys_trace_t *trace);
MYS_PUBLIC void mys_trace_1p(mys_trace_t *trace, void *data1);
MYS_PUBLIC void mys_trace_1u(mys_trace_t *trace, uint64_t data1);
MYS_PUBLIC void mys_trace_1i(mys_trace_t *trace, int64_t data1);
MYS_PUBLIC void mys_trace_1d(mys_trace_t *trace, double data1);
