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
#include "mpistubs.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// TODO: Add csv.c and csv.h, which is 1D array, table.c and table.h is for 2D array.


enum {
    MYS_TABLE_ATTR_INT32_T,
    MYS_TABLE_ATTR_INT64_T,
    MYS_TABLE_ATTR_UINT32_T,
    MYS_TABLE_ATTR_UINT64_T,
    MYS_TABLE_ATTR_DOUBLE,
    MYS_TABLE_ATTR_FLOAT,
};

typedef struct {
    uint8_t values[0];  // Flexible array member
} mys_table_cell_t;

typedef struct {
    mys_MPI_Comm comm;
    size_t num_attrs;
    int *attr_types;
    char **attr_formats;
    char **attr_names;
    size_t cell_size;
    size_t num_cells;
    size_t capacity;
    mys_table_cell_t **cells;
} mys_table_t;

// int dest = 100;
// uint64_t send_bytes = 1024;
// double tstart = 1e-3;
// double tend = 2e-3;
// mys_table_t *table = mys_table_create(MPI_COMM_WORLD, 4, "int32_t dest", "uint64_t send_byte", "double%.6e tstart", "double%.6e tend");
// for (size_t i = 0; i < num_dests; i++) {
//     mys_table_append_cell(table, dests[i], send_bytes[i], tstarts[i], tends[i]);
// }
// mys_table_dump(table, "test.mystable");

mys_table_t *mys_table_create(mys_MPI_Comm comm, size_t num_attrs, ...);
void mys_table_destroy(mys_table_t **table);
void mys_table_append_cell(mys_table_t *table, ...);
void mys_table_dump(mys_table_t *table, const char *file_name);
// void mys_table_dump_excel(mys_table_t *table, const char *file_name);
