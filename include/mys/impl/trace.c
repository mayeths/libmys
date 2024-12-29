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
#include "../errno.h"
#include "../mpistubs.h"
#include "../assert.h"
#include "../trace.h"
#include "../memory.h"
#include "../thread.h"
#include "../hrtime.h"

typedef struct mys_trace_e1block_t {
    struct mys_trace_e1block_t *prev;
    struct mys_trace_e1block_t *next;
    struct mys_trace_event1_t *events;
    uint32_t capacity;
    uint32_t size;
} mys_trace_e1block_t;

typedef struct mys_trace_t {
    struct mys_trace_e1block_t *e1block_head;
    struct mys_trace_e1block_t *e1block_tail;
    uint64_t e1_total_capacity;
    uint64_t e1_total_size;
    bool being_iter;
    struct mys_trace_e1block_t *e1_iter_last_block;
    uint32_t e1_iter_last_index;
} mys_trace_t;

typedef struct _mys_trace_G_t {
    bool inited;
    mys_mutex_t lock;
} _mys_trace_G_t;

static _mys_trace_G_t _mys_trace_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
};

MYS_ATTR_OPTIMIZE_O3
MYS_INTERNAL void mys_G_trace_init()
{
    if (_mys_trace_G.inited == true)
        return;
    mys_mutex_lock(&_mys_trace_G.lock);
    {
        _mys_trace_G.inited = true;
    }
    mys_mutex_unlock(&_mys_trace_G.lock);
}

MYS_ATTR_OPTIMIZE_O3
MYS_INTERNAL void mys_G_trace_fini()
{
    if (_mys_trace_G.inited == false)
        return;
    mys_mutex_lock(&_mys_trace_G.lock);
    {
        _mys_trace_G.inited = false;
    }
    mys_mutex_unlock(&_mys_trace_G.lock);
}

MYS_STATIC mys_trace_e1block_t *_mys_expand_trace_block(mys_trace_t *trace)
{
    AS_NE_PTR(trace, NULL);

    mys_trace_e1block_t *block = trace->e1block_tail;
    uint32_t capacity =
        (block == NULL || block->capacity == 0) ? 1024 :
        (block->capacity < UINT32_MAX / 2) ? (block->capacity * 2) :
        block->capacity;

    block = (mys_trace_e1block_t *)mys_malloc2(MYS_ARENA_TRACE, sizeof(mys_trace_e1block_t));
    // DLOG(0, "Constructing block=%p cap=%u", block, capacity);
    // DLOG(0, "    Allocating block=%p %zu", block, sizeof(mys_trace_e1block_t));
    if (block == NULL)
        return NULL;

    block->capacity = capacity;
    block->size = 0;
    block->events = (mys_trace_event1_t *)mys_malloc2(MYS_ARENA_TRACE, sizeof(mys_trace_event1_t) * capacity);
    if (block->events == NULL) {
        mys_free2(MYS_ARENA_TRACE, block, sizeof(mys_trace_e1block_t));
        return NULL;
    }
    // DLOG(0, "    Allocating block->events=%p %zu", block->events, sizeof(mys_trace_event1_t) * capacity);

    mys_trace_e1block_t *e1block_tail = trace->e1block_tail;
    // update self link
    block->prev = trace->e1block_tail;
    block->next = NULL;
    // update tail link
    if (e1block_tail != NULL) e1block_tail->next = block;
    // update trace
    trace->e1_total_capacity += capacity;
    trace->e1block_tail = block;
    if (trace->e1block_head == NULL) trace->e1block_head = block;

    return block;
}

MYS_STATIC void _mys_destroy_trace_block(mys_trace_t *trace, mys_trace_e1block_t **block)
{
    AS_NE_PTR(trace, NULL);

    if (block == NULL || *block == NULL)
        return;

    mys_trace_e1block_t *prev = (*block)->prev;
    mys_trace_e1block_t *next = (*block)->next;
    // update prev and next link
    if (prev != NULL) prev->next = next;
    if (next != NULL) next->prev = prev;
    // update trace
    trace->e1_total_capacity -= (*block)->capacity;
    trace->e1_total_size -= (*block)->size;
    if (trace->e1block_head == *block) trace->e1block_head = next;
    if (trace->e1block_tail == *block) trace->e1block_tail = prev;

    // DLOG(0, "    Freeing block->event=%p %zu", (*block)->events, sizeof(mys_trace_event1_t) * (*block)->capacity);
    mys_free2(MYS_ARENA_TRACE, (*block)->events, sizeof(mys_trace_event1_t) * (*block)->capacity);
    // DLOG(0, "    Freeing block=%p %zu", (*block), sizeof(mys_trace_e1block_t));
    mys_free2(MYS_ARENA_TRACE, (*block), sizeof(mys_trace_e1block_t));
    *block = NULL;
}

MYS_PUBLIC mys_trace_t *mys_trace_create()
{
    mys_G_trace_init();
    mys_trace_t *trace = (mys_trace_t *)mys_malloc2(MYS_ARENA_TRACE, sizeof(mys_trace_t));
    MYS_RETIF(trace == NULL, MYS_ENOMEM, NULL);

    trace->e1block_head = NULL;
    trace->e1block_tail = NULL;
    trace->e1_total_capacity = 0;
    trace->e1_total_size = 0;
    trace->being_iter = false;
    // trace->e0_next_iter = 0;
    trace->e1_iter_last_index = 0;
    trace->e1_iter_last_block = NULL;
    // trace->e2_next_iter = 0;
    // trace->e3_next_iter = 0;
    // trace->e4_next_iter = 0;
    _mys_expand_trace_block(trace);

    return trace;
}

MYS_PUBLIC void mys_trace_destroy(mys_trace_t **trace)
{
    mys_G_trace_init();
    MYS_RETIF(trace == NULL || *trace == NULL, MYS_EINVAL);

    mys_trace_e1block_t *block = (*trace)->e1block_head;
    while (block != NULL) {
        mys_trace_e1block_t *next = block->next;
        // DLOG(0, "Destroying block=%p cap=%u", block, block->capacity);
        _mys_destroy_trace_block(*trace, &block);
        block = next;
    }
    mys_free2(MYS_ARENA_TRACE, (*trace), sizeof(mys_trace_t));
}


MYS_PUBLIC mys_trace_iter_t *mys_trace_start_iter(mys_trace_t *trace)
{
    AS_EQ_BOOL(trace->being_iter, false);

    mys_trace_e1block_t *block = trace->e1block_head;
    if (block == NULL || block->size == 0) {
        return NULL;
    }

    mys_trace_iter_t *iter = (mys_trace_iter_t *)mys_malloc2(MYS_ARENA_TRACE, sizeof(mys_trace_iter_t));
    if (iter == NULL) {
        return NULL;
    }

    iter->type = MYS_TRACE_EVENT1;
    iter->e1 = &block->events[0];

    trace->being_iter = true;
    trace->e1_iter_last_block = block;
    trace->e1_iter_last_index = 0;
    return iter;
}

MYS_PUBLIC mys_trace_iter_t *mys_trace_next_iter(mys_trace_t *trace, mys_trace_iter_t *iter)
{
    MYS_RETIF(trace == NULL || iter == NULL, MYS_EINVAL, NULL);
    MYS_RETIF(trace->being_iter == false, MYS_EINVAL, NULL);

    mys_trace_e1block_t *block = trace->e1_iter_last_block;
    uint32_t index = trace->e1_iter_last_index + 1;
    if (block == NULL) {
        mys_trace_interrupt_iter(trace, iter);
        return NULL;
    }
    if (index >= block->size) {
        if (block->next == NULL) {
            mys_trace_interrupt_iter(trace, iter);
            return NULL;
        } else {
            block = block->next;
            index = 0;
            if (block == NULL) {
                mys_trace_interrupt_iter(trace, iter);
                return NULL;
            }
            if (index >= block->size) {
                mys_trace_interrupt_iter(trace, iter);
                return NULL;
            }
        }
    }

    iter->type = MYS_TRACE_EVENT1;
    iter->e1 = &block->events[index];

    trace->e1_iter_last_block = block;
    trace->e1_iter_last_index = index;

    return iter;
}

MYS_PUBLIC void mys_trace_interrupt_iter(mys_trace_t *trace, mys_trace_iter_t *iter)
{
    if (trace == NULL || iter == NULL)
        return;
    trace->being_iter = false;
    trace->e1_iter_last_block = NULL;
    trace->e1_iter_last_index = 0;
    mys_free2(MYS_ARENA_TRACE, iter, sizeof(mys_trace_iter_t));
}

MYS_ATTR_OPTIMIZE_O3
MYS_ATTR_ALWAYS_INLINE
MYS_STATIC void _mys_trace_append1(mys_trace_t *trace, uint64_t data1)
{
    // mys_G_trace_init(); // cost 30ns~75 ns even in O3 optimization for opening it
    // ASX_EQ_BOOL(trace->being_iter, false, "Trace %p is being used for iteration", trace); // 20~50ns overhead

    // This function is sensitive to latency. For O3 optimization, we get 215ns per op.
    // The following helps to reduce latency:
    // 1. Reduce the body size of "if (...)" statement by moving its logic to other function. Get 180ns per op.
    // 2. Move "if (...)" to after all other code, got 100ns per op.

    mys_trace_e1block_t *block = trace->e1block_tail;
    block->events[block->size].u1 = data1;
    block->events[block->size].time = mys_hrtime();
    block->size += 1;
    trace->e1_total_size += 1;
    if (block->size == block->capacity) {
        _mys_expand_trace_block(trace);
    }
}

union _mys_trace_num_t {
    uint64_t u64;
    int64_t i64;
    double d64;
    void *ptr;
    struct {          int l;          int h; } i32; // Do not change order of l and h
    struct { unsigned int l; unsigned int h; } u32; // Do not change order of l and h
    struct {        float l;        float h; } f32; // Do not change order of l and h
};

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_trace_1p(mys_trace_t *trace, void *data1)
{
    union _mys_trace_num_t num;
    num.ptr = data1;
    _mys_trace_append1(trace, num.u64);
}

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_trace_1u(mys_trace_t *trace, uint64_t data1)
{
    union _mys_trace_num_t num;
    num.u64 = data1;
    _mys_trace_append1(trace, num.u64);
}

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_trace_1i(mys_trace_t *trace, int64_t data1)
{
    union _mys_trace_num_t num;
    num.i64 = data1;
    _mys_trace_append1(trace, num.u64);
}

MYS_ATTR_OPTIMIZE_O3
MYS_PUBLIC void mys_trace_1d(mys_trace_t *trace, double data1)
{
    union _mys_trace_num_t num;
    num.d64 = data1;
    _mys_trace_append1(trace, num.u64);
}

