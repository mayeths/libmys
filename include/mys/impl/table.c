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
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../os.h"
#include "../memory.h"
#include "../hrtime.h"
#include "../table.h"

#include <string.h>
#include <inttypes.h>

mys_table_t *mys_table_create(mys_MPI_Comm comm, size_t num_attrs, ...) {
    mys_table_t *table = (mys_table_t *)malloc(sizeof(mys_table_t));
    if (!table) {
        perror("Failed to allocate memory for table");
        return NULL;
    }
    table->comm = comm;
    table->num_attrs = num_attrs;
    table->attr_types = (int *)malloc(num_attrs * sizeof(int));
    table->attr_formats = (char **)malloc(num_attrs * sizeof(char *));
    table->attr_names = (char **)malloc(num_attrs * sizeof(char *));
    table->schema = NULL;
    table->comments = NULL;
    table->num_comments = 0;
    table->cell_size = 0;
    table->num_cells = 0;
    table->capacity = 0;
    table->cells = NULL;

    va_list args;
    va_start(args, num_attrs);
    for (size_t i = 0; i < num_attrs; i++) {
        const char *attr_definition = va_arg(args, const char *);
        char attr_type[64], attr_name[64];
        sscanf(attr_definition, "%63s %63s", attr_type, attr_name);

        // Check if a format is present in attr_type after the attribute type
        char *perc = strchr(attr_type, '%');
        if (perc) {
            table->attr_formats[i] = strdup(perc);  // Save format part
            *perc = '\0';  // Terminate the attr_type string before the format
        } else {
            table->attr_formats[i] = NULL;  // No format found
        }

        if (strcmp(attr_type, "int32_t") == 0) {
            table->attr_types[i] = MYS_TABLE_ATTR_INT32_T;
            table->cell_size += sizeof(int32_t);
        } else if (strcmp(attr_type, "int64_t") == 0) {
            table->attr_types[i] = MYS_TABLE_ATTR_INT64_T;
            table->cell_size += sizeof(int64_t);
        } else if (strcmp(attr_type, "uint32_t") == 0) {
            table->attr_types[i] = MYS_TABLE_ATTR_UINT32_T;
            table->cell_size += sizeof(uint32_t);
        } else if (strcmp(attr_type, "uint64_t") == 0) {
            table->attr_types[i] = MYS_TABLE_ATTR_UINT64_T;
            table->cell_size += sizeof(uint64_t);
        } else if (strcmp(attr_type, "float") == 0) {
            table->attr_types[i] = MYS_TABLE_ATTR_FLOAT;
            table->cell_size += sizeof(float);
        } else if (strcmp(attr_type, "double") == 0) {
            table->attr_types[i] = MYS_TABLE_ATTR_DOUBLE;
            table->cell_size += sizeof(double);
        } else {
            FAILED("Invalid attribute: %s", attr_definition);
        }

        table->attr_names[i] = strdup(attr_name);
    }
    va_end(args);
    return table;
}

void mys_table_destroy(mys_table_t **table_ptr) {
    if (!table_ptr || !*table_ptr) return;
    mys_table_t *table = *table_ptr;
    for (size_t i = 0; i < table->num_cells; i++) {
        free(table->cells[i]);
    }
    free(table->cells);
    for (size_t i = 0; i < table->num_attrs; i++) {
        free(table->attr_formats[i]);
        free(table->attr_names[i]);
    }
    free(table->attr_names);
    free(table->attr_formats);
    free(table->attr_types);
    if (table->schema) free(table->schema);
    if (table->num_comments != 0) {
        for (size_t i = 0; i < table->num_comments; i++)
            free(table->comments[i]);
        free(table->comments);
    }
    free(table);
    *table_ptr = NULL;
}

void mys_table_append_cell(mys_table_t *table, ...) {
    if (!table) return;
    if (table->num_cells == table->capacity) {
        table->capacity = table->capacity == 0 ? 16 : table->capacity * 2;
        table->cells = (mys_table_cell_t **)realloc(table->cells, table->capacity * sizeof(mys_table_cell_t *));
    }

    mys_table_cell_t *cell = (mys_table_cell_t *)malloc(sizeof(mys_table_cell_t) + table->cell_size);

    va_list args;
    va_start(args, table);
    size_t offset = 0;
    for (size_t i = 0; i < table->num_attrs; i++) {
        switch (table->attr_types[i]) {
            case MYS_TABLE_ATTR_INT32_T:
                *(int32_t *)(cell->values + offset) = va_arg(args, int);
                offset += sizeof(int32_t);
                break;
            case MYS_TABLE_ATTR_INT64_T:
                *(int64_t *)(cell->values + offset) = va_arg(args, int64_t);
                offset += sizeof(int64_t);
                break;
            case MYS_TABLE_ATTR_UINT32_T:
                *(uint32_t *)(cell->values + offset) = va_arg(args, unsigned int);
                offset += sizeof(uint32_t);
                break;
            case MYS_TABLE_ATTR_UINT64_T:
                *(uint64_t *)(cell->values + offset) = va_arg(args, uint64_t);
                offset += sizeof(uint64_t);
                break;
            case MYS_TABLE_ATTR_FLOAT:
                *(float *)(cell->values + offset) = (float)va_arg(args, double);
                offset += sizeof(float);
                break;
            case MYS_TABLE_ATTR_DOUBLE:
                *(double *)(cell->values + offset) = va_arg(args, double);
                offset += sizeof(double);
                break;
        }
    }
    va_end(args);
    table->cells[table->num_cells++] = cell;
}

void mys_table_set_schema(mys_table_t *table, const char *schema)
{
    if (table->schema != NULL)
        free(table->schema);
    table->schema = strdup(schema);
}

MYS_PUBLIC void mys_table_add_comment(mys_table_t *table, const char *comment)
{
    char *new_comment = strdup(comment);
    char **new_comments = (char **)realloc(table->comments, (table->num_comments + 1) * sizeof(char *));
    new_comments[table->num_comments] = new_comment;
    table->comments = new_comments;
    table->num_comments++;
}

void mys_table_dump(mys_table_t *table, const char *file_name) {
    int myrank, nranks;
    mys_MPI_Comm_rank(table->comm, &myrank);
    mys_MPI_Comm_size(table->comm, &nranks);

    FILE *file = NULL;
    if (myrank == 0) {
        file = fopen(file_name, "w");
        if (!file) {
            perror("Error opening file");
            return;
        }
        // Write header
        fprintf(file, "################################ Mayeths Table #################################\n");
        fprintf(file, "# Version 1\n");
        for (size_t i = 0; i < table->num_comments; i++) {
            fprintf(file, "# %s\n", table->comments[i]);
        }
        fprintf(file, "################################################################################\n");
        // Write meta
        if (table->schema == NULL) {
            fprintf(file, "[schema] none\n");
        } else {
            fprintf(file, "[schema] %s\n", table->schema);
        }
        fprintf(file, "[attributes] ");
        for (size_t i = 0; i < table->num_attrs; i++) {
            const char *attr_type;
            switch (table->attr_types[i]) {
                case MYS_TABLE_ATTR_INT32_T:
                    attr_type = "int32_t";
                    break;
                case MYS_TABLE_ATTR_INT64_T:
                    attr_type = "int64_t";
                    break;
                case MYS_TABLE_ATTR_UINT32_T:
                    attr_type = "uint32_t";
                    break;
                case MYS_TABLE_ATTR_UINT64_T:
                    attr_type = "uint64_t";
                    break;
                case MYS_TABLE_ATTR_FLOAT:
                    attr_type = "float";
                    break;
                case MYS_TABLE_ATTR_DOUBLE:
                    attr_type = "double";
                    break;
                default:
                    attr_type = "invalid_type";
                    break;
            }
            if (table->attr_formats[i] == NULL) {
                fprintf(file, "%s %s", attr_type, table->attr_names[i]);
            } else {
                fprintf(file, "%s%s %s", attr_type, table->attr_formats[i], table->attr_names[i]);
            }
            if (i < table->num_attrs - 1) {
                fprintf(file, ", ");
            }
        }
        fprintf(file, "\n");
        fprintf(file, "################################################################################\n");
    }

    int root = 0;
    uint64_t num_cells = table->num_cells;

    // Gather the number of cells from all ranks
    uint64_t *all_num_cells = (uint64_t *)malloc(nranks * sizeof(uint64_t));
    if (!all_num_cells) {
        perror("Error allocating memory for all_num_cells");
        if (file) fclose(file);
        return;
    }
    mys_MPI_Gather(&num_cells, 1, mys_MPI_UINT64_T, all_num_cells, 1, mys_MPI_UINT64_T, root, table->comm);

    for (int rank = 0; rank < nranks; rank++) {
        int num_requests = 0;
        if (myrank == rank) num_requests += table->num_cells;
        if (myrank == root) num_requests += all_num_cells[rank];
        mys_MPI_Request *reqs = (mys_MPI_Request *)malloc(sizeof(mys_MPI_Request) * num_requests);

        num_requests = 0;
        if (myrank == rank) {
            // Send cells to the root rank
            if (nranks > 1) {
                for (size_t i = 0; i < table->num_cells; i++) {
                    mys_MPI_Isend(table->cells[i]->values, table->cell_size, mys_MPI_BYTE, root, 1000, table->comm, &reqs[num_requests++]);
                }
            }
        }

        if (myrank == root) {
            // Receive cells from each rank
            size_t recv_cells = all_num_cells[rank];
            uint8_t *tmp_cells = (uint8_t *)malloc(recv_cells * table->cell_size);

            if (nranks > 1) {
                for (size_t i = 0; i < recv_cells; i++) {
                    mys_table_cell_t *tmp_cell = (mys_table_cell_t *)(tmp_cells + i * table->cell_size);
                    mys_MPI_Irecv(tmp_cell->values, table->cell_size, mys_MPI_BYTE, rank, 1000, table->comm, &reqs[num_requests++]);
                }
                mys_MPI_Waitall(num_requests, reqs, mys_MPI_STATUSES_IGNORE);
            } else {
                for (size_t i = 0; i < recv_cells; i++) {
                    mys_table_cell_t *tmp_cell = (mys_table_cell_t *)(tmp_cells + i * table->cell_size);
                    memcpy(tmp_cell->values, table->cells[i]->values, table->cell_size);
                }
            }

            // Write received cells to the file
            for (size_t i = 0; i < recv_cells; i++) {
                size_t offset = 0;
                mys_table_cell_t *tmp_cell = (mys_table_cell_t *)(tmp_cells + i * table->cell_size);
                for (size_t j = 0; j < table->num_attrs; j++) {

#define GOGOGO(type, default_fmt) do {                        \
    const char *fmt = (table->attr_formats[j] != NULL)        \
        ? table->attr_formats[j]                              \
        : default_fmt;                                        \
    fprintf(file, fmt, *(type *)(tmp_cell->values + offset)); \
    offset += sizeof(type);                                   \
} while (0)

                    if (table->attr_types[j] == MYS_TABLE_ATTR_INT32_T) {
                        GOGOGO(int32_t, "%" PRIi32);
                    } else if (table->attr_types[j] == MYS_TABLE_ATTR_INT64_T) {
                        GOGOGO(int64_t, "%" PRIi64);
                    } else if (table->attr_types[j] == MYS_TABLE_ATTR_UINT32_T) {
                        GOGOGO(uint32_t, "%" PRIu32);
                    } else if (table->attr_types[j] == MYS_TABLE_ATTR_UINT64_T) {
                        GOGOGO(uint64_t, "%" PRIu64);
                    } else if (table->attr_types[j] == MYS_TABLE_ATTR_FLOAT) {
                        GOGOGO(float, "%.9e");
                    } else if (table->attr_types[j] == MYS_TABLE_ATTR_DOUBLE) {
                        GOGOGO(double, "%.17e");
                    } else {
                        fprintf(stderr, "Unknown attribute type encountered at index %zu\n", j);
                    }

                    if (j < table->num_attrs - 1) {
                        fprintf(file, ",");
                    }

#undef GOGOGO
                }
                fprintf(file, ";");
            }
            free(tmp_cells);
            fprintf(file, "\n");
        } else {
            mys_MPI_Waitall(num_requests, reqs, mys_MPI_STATUSES_IGNORE);
        }
        free(reqs);
    }

    if (myrank == root) {
        fclose(file);
    }

    free(all_num_cells);
}
