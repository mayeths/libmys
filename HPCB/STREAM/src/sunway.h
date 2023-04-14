#ifndef _SUNWAY_H_
#define _SUNWAY_H_

#if defined(__sw_host__)
#include <crts.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif defined(__sw_slave__)
#include <slave.h>
#endif

#include "common.h"

#define TASK_100_ID 100
typedef struct task100_args_t {
    int narr;
    int *arr;
} task100_args_t;

#define TASK_COPY_ID 201
typedef struct {
    uint64_t n;
    double *c;
    const double *a;
} task_copy_args_t;

#define TASK_SCALE_ID 202
typedef struct {
    uint64_t n;
    double scalar;
    double *c;
    const double *a;
} task_scale_args_t;

#define TASK_ADD_ID 203
typedef struct {
    uint64_t n;
    double *c;
    const double *a;
    const double *b;
} task_add_args_t;

#define TASK_TRIAD_ID 204
typedef struct {
    uint64_t n;
    double scalar;
    double *c;
    const double *a;
    const double *b;
} task_triad_args_t;

typedef struct xcpe_task_t {
    int id;
    void *fn_addr;
    void *args;
} xcpe_task_t;

typedef struct _xcpe_G_t {
    struct {
        void *fn_addr;
        size_t arg_size;
        int id;
    } handlers[64];
    xcpe_task_t task;
    bool running;
} _xcpe_G_t;

#define TASK_EXIT -1

#if defined(__sw_host__) && defined(XCPE_HOST_IMPL)
_xcpe_G_t _xcpe_G = {
    .handlers = {
        { .fn_addr = NULL, .arg_size = 0, .id = 0 /* Uninitalized ID is 0 */ },
    },
};

void xcpe_dispatch(int id, void *args_ptr)
{
    _xcpe_G.task.id = id;
    _xcpe_G.task.args = args_ptr;
    athread_sync_master_array();
    _xcpe_G.running = true;
}
void xcpe_waitjob()
{
    athread_sync_master_array();
    _xcpe_G.running = false;
}
void xcpe_stop_runtime()
{
    if (_xcpe_G.running == true)
        xcpe_waitjob();
    xcpe_dispatch(TASK_EXIT, NULL);
}
int _xcpe_reg(void *fn_addr, size_t arg_size) {
    // printf("function_pc is %p args_size is %llu\n", fn_addr, arg_size);
    int used_max_id = INT32_MIN;
    for (int i = 0; i < 64; i++) {
        if (_xcpe_G.handlers[i].fn_addr == NULL)
            break;
        if (used_max_id < _xcpe_G.handlers[i].id)
            used_max_id = _xcpe_G.handlers[i].id;
    }
    int id = used_max_id <= 0 ? 1 : used_max_id + 1;
    for (int i = 0; i < 64; i++) {
        if (_xcpe_G.handlers[i].fn_addr != NULL)
            continue;
        _xcpe_G.handlers[i].fn_addr = fn_addr;
        _xcpe_G.handlers[i].arg_size = arg_size;
        _xcpe_G.handlers[i].id = id;
        break;
    }
    return id;
}
#define xcpe_register(fn_name, arg_type) _xcpe_reg(&SLAVE_FUN(fn_name), sizeof(arg_type))

#elif defined(__sw_slave__) && defined(XCPE_SLAVE_IMPL)
extern __uncached _xcpe_G_t _xcpe_G;
void xcpe_next(int *id, void **args_ptr)
{
    athread_ssync_master_array();
    xcpe_task_t task;
    athread_dma_bcast_coll(&task, &_xcpe_G.task, sizeof(xcpe_task_t));
    *id = task.id;
    *args_ptr = task.args;
}

#else
/* dummy MPE declarations */
extern int _xcpe_reg();
#define xcpe_register(fn_name, arg_type) _xcpe_reg()
extern void xcpe_dispatch(int id, void *args_ptr);
extern void xcpe_stop_runtime();
/* dummy CPE declarations */
extern void xcpe_runtime();
extern void xcpe_next(int *id, void **args_ptr);
typedef int athread_rply_t;
#endif

#endif /*_SUNWAY_H_*/