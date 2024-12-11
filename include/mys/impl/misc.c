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
#include "../misc.h"
#include <stdio.h>
#include <stdbool.h>

typedef struct _mys_misc_G_t {
    bool inited;
    mys_mutex_t lock;
} _mys_misc_G_t;

MYS_STATIC _mys_misc_G_t _mys_misc_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
};

MYS_PUBLIC void mys_mpi_ensure_init()
{
    if (_mys_misc_G.inited == true)
        return;
    mys_mutex_lock(&_mys_misc_G.lock);
    {
        int mpi_inited;
        mys_MPI_Initialized(&mpi_inited);
        if (!mpi_inited) {
            mys_MPI_Init_thread(NULL, NULL, mys_MPI_THREAD_SINGLE, &mpi_inited);
            fprintf(stdout, ">>>>> ===================================== <<<<<\n");
            fprintf(stdout, ">>>>> Nevel let libmys init MPI you dumbass <<<<<\n");
            fprintf(stdout, ">>>>> ===================================== <<<<<\n");
            fflush(stdout);
        }
    }
    _mys_misc_G.inited = true;
    mys_mutex_unlock(&_mys_misc_G.lock);
}
