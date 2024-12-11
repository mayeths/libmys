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
#include "_private.h"
#include "../statistic.h"
#include "../memory.h"

MYS_PUBLIC double mys_arthimetic_mean(double *arr, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return (1 / (double)n) * sum;
}

MYS_PUBLIC double mys_harmonic_mean(double *arr, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += 1 / arr[i];
    }
    return ((double)n) / sum;
}

MYS_PUBLIC double mys_geometric_mean(double *arr, int n)
{
    double product = 1;
    for (int i = 0; i < n; i++) {
        product *= arr[i];
    }
    return mys_math_pow(product, 1 / (double)n);
}

MYS_PUBLIC double mys_standard_deviation(double *arr, int n)
{
    double xbar = mys_arthimetic_mean(arr, n);
    double denom = 0;
    double nom = n - 1;
    for (int i = 0; i < n; i++) {
        double diff = arr[i] - xbar;
        denom += diff * diff;
    }
    return mys_math_sqrt(denom / nom);
}

MYS_PUBLIC void mys_aggregate_analysis_array(size_t n, double *values, mys_aggregate_t *results)
{
    mys_mpi_init();
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    struct di_t { double d; int i; };
    struct di_t *dibuf = (struct di_t *)mys_malloc2(mys_arena_stat, sizeof(struct di_t) * n);
    double *dbuf = (double *)((void *)dibuf);

    {// mine, sum, avg
        mys_MPI_Allreduce(values, dbuf, n, mys_MPI_DOUBLE, mys_MPI_SUM, mys_mpi_comm());
        for (size_t i = 0; i < n; i++) {
            results[i].self = values[i];
            results[i].sum = dbuf[i];
            results[i].avg = dbuf[i] / (double)nranks;
        }
    }
    {// var and std
        for (size_t i = 0; i < n; i++) {
            dbuf[i] = (values[i] - results[i].avg) * (values[i] - results[i].avg);
        }
        mys_MPI_Allreduce(mys_MPI_IN_PLACE, dbuf, n, mys_MPI_DOUBLE, mys_MPI_SUM, mys_mpi_comm());
        for (size_t i = 0; i < n; i++) {
            results[i].var = dbuf[i] / (double)nranks;
            results[i].std = mys_math_sqrt(results[i].var);
        }
    }
    {// max and min
        for (size_t i = 0; i < n; i++) {
            dibuf[i].d = values[i];
            dibuf[i].i = myrank;
        }
        mys_MPI_Allreduce(mys_MPI_IN_PLACE, dibuf, n, mys_MPI_DOUBLE_INT, mys_MPI_MAXLOC, mys_mpi_comm());
        for (size_t i = 0; i < n; i++) {
            results[i].max = dibuf[i].d;
            results[i].loc_max = dibuf[i].i;
        }

        for (size_t i = 0; i < n; i++) {
            dibuf[i].d = values[i];
            dibuf[i].i = myrank;
        }
        mys_MPI_Allreduce(mys_MPI_IN_PLACE, dibuf, n, mys_MPI_DOUBLE_INT, mys_MPI_MINLOC, mys_mpi_comm());
        for (size_t i = 0; i < n; i++) {
            results[i].min = dibuf[i].d;
            results[i].loc_min = dibuf[i].i;
        }
    }
    mys_free2(mys_arena_stat, dibuf, sizeof(struct di_t) * n);
}

MYS_PUBLIC mys_aggregate_t mys_aggregate_analysis(double value)
{
    mys_aggregate_t result;
    mys_aggregate_analysis_array(1, &value, &result);
    return result;
}


MYS_PUBLIC char *mys_boxplot(double *values, size_t n)
{
    mys_boxplot_t *bxp = mys_boxplot_create(values, n);
    if (!bxp)
        return NULL;

    char *json = mys_boxplot_serialize(bxp);
    mys_boxplot_destroy(&bxp);
    return json;
}

MYS_PUBLIC mys_boxplot_t *mys_boxplot_create(double *values, size_t n) {
    if (n == 0) {
        return NULL;
    }

    mys_boxplot_t *bxp = (mys_boxplot_t *)mys_malloc2(mys_arena_stat, sizeof(mys_boxplot_t));
    if (!bxp) {
        return NULL; // Memory allocation failed
    }

    bxp->whislo = bxp->q1 = bxp->med = bxp->q3 = bxp->whishi = values[0];
    bxp->iqr = 0;
    bxp->n_fliers = 0;
    bxp->nb_fliers = 0;
    bxp->nt_fliers = 0;
    bxp->fliers = NULL;

    if (n == 1) {
        return bxp;
    }

    double *arr = (double *)mys_malloc2(mys_arena_stat, sizeof(double) * n);
    if (!arr) {
        mys_free2(mys_arena_stat, bxp, sizeof(mys_boxplot_t));
        return NULL; // Memory allocation failed
    }

    memcpy(arr, values, sizeof(double) * n);
    mys_sort_f64(arr, n);

    double alpha = 1.0;
    double beta = 1.0;

    // See how numpy.percentile calculate index that used
    // by matplotlib.cbook.boxplot_stats internally
    // https://numpy.org/doc/stable/reference/generated/numpy.percentile.html
    // i + g = (q / 100) * (n - alpha - beta + 1) + alpha
    //
    // 0-index: 0   1   2   3   4
    //              ^       ^
    // First quartile       |
    //                      |
    //         Third quartile
    double ig_q1 = ((25. / 100.) * (double)(n - alpha - beta + 1) + (double)alpha);
    double ig_q2 = ((50. / 100.) * (double)(n - alpha - beta + 1) + (double)alpha);
    double ig_q3 = ((75. / 100.) * (double)(n - alpha - beta + 1) + (double)alpha);
    // To 0-indexing
    ig_q1 -= 1;
    ig_q2 -= 1;
    ig_q3 -= 1;
    size_t arg_q1 = (size_t)(ig_q1);
    size_t arg_q2 = (size_t)(ig_q2);
    size_t arg_q3 = (size_t)(ig_q3);
    bxp->q1  = arr[arg_q1] + (arr[arg_q1 + 1] - arr[arg_q1]) * (ig_q1 - arg_q1);
    bxp->med = arr[arg_q2] + (arr[arg_q2 + 1] - arr[arg_q2]) * (ig_q2 - arg_q2);
    bxp->q3  = arr[arg_q3] + (arr[arg_q3 + 1] - arr[arg_q3]) * (ig_q3 - arg_q3);

    bxp->iqr = bxp->q3 - bxp->q1;
    double loval = bxp->q1 - 1.5 * bxp->iqr;
    double hival = bxp->q3 + 1.5 * bxp->iqr;

    double abs_q1 = mys_math_fabs(bxp->q1);
    double abs_q3 = mys_math_fabs(bxp->q3);
    double bound = 1e-9 * ((abs_q1 > abs_q3) ? abs_q1 : abs_q3);
    if (mys_math_fabs(bxp->q1 - bxp->q3) < bound) {
        bxp->whislo = arr[0];
        bxp->whishi = arr[n - 1];
    } else {
        bxp->whislo = bxp->q1;
        bxp->whishi = bxp->q3;
        for (size_t i = 0; i <= arg_q1; i++) {
            if (arr[i] < loval)
                bxp->nb_fliers += 1;
            if (arr[i] >= loval && arr[i] < bxp->whislo)
                bxp->whislo = arr[i];
        }
        for (size_t i = arg_q3; i < n; i++) {
            if (arr[i] <= hival && arr[i] > bxp->whishi)
                bxp->whishi = arr[i];
            if (arr[i] > hival)
                bxp->nt_fliers += 1;
        }
    }

    bxp->n_fliers = bxp->nt_fliers + bxp->nb_fliers;

    bxp->fliers = (double *)mys_malloc2(mys_arena_stat, sizeof(double) * (bxp->n_fliers));
    if (bxp->fliers) {
        size_t c = 0;
        for (size_t i = 0; i < bxp->nb_fliers; i++)
            bxp->fliers[c++] = arr[i];
        for (size_t i = n - bxp->nt_fliers; i < n; i++)
            bxp->fliers[c++] = arr[i];
    }

    mys_free2(mys_arena_stat, arr, sizeof(double) * n);
    return bxp;
}

MYS_PUBLIC void mys_boxplot_destroy(mys_boxplot_t **bxp) {
    if (bxp != NULL && (*bxp) != NULL) {
        if ((*bxp)->fliers != NULL) {
            mys_free2(mys_arena_stat, (*bxp)->fliers, sizeof(double) * ((*bxp)->n_fliers));
        }
        mys_free2(mys_arena_stat, *bxp, sizeof(mys_boxplot_t));
    }
    (*bxp) = NULL;
}

static void _mys_append_buf(char **buffer, size_t *size, size_t *used, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    while (1) {
        // Calculate remaining buffer size
        size_t remaining = *size - *used;

        // Attempt to write to the buffer
        int written = vsnprintf(*buffer + *used, remaining, format, args);

        // Check if the buffer was large enough
        if (written < 0) {
            // Encoding error
            va_end(args);
            return;
        } else if ((size_t)written < remaining) {
            // Successfully written
            *used += written;
            break;
        } else {
            // Buffer too small, reallocate
            *size += (written + 1); // Double the buffer size plus extra space for null terminator
            *buffer = (char *)realloc(*buffer, *size);
            if (!*buffer) {
                va_end(args);
                return; // Memory allocation failed
            }
        }
    }

    va_end(args);
}

static char *_mys_boxplot_serialize_impl(const mys_boxplot_t *bxp, bool pretty_print)
{
    size_t size = 256;
    size_t used = 0;
    char *buffer = (char *)malloc(size);
    if (!buffer)
        return NULL;

    if (pretty_print) {
        _mys_append_buf(&buffer, &size, &used, "{\n");
        _mys_append_buf(&buffer, &size, &used, "  \"whislo\": %.3e,\n", bxp->whislo);
        _mys_append_buf(&buffer, &size, &used, "  \"q1\": %.3e,\n", bxp->q1);
        _mys_append_buf(&buffer, &size, &used, "  \"med\": %.3e,\n", bxp->med);
        _mys_append_buf(&buffer, &size, &used, "  \"q3\": %.3e,\n", bxp->q3);
        _mys_append_buf(&buffer, &size, &used, "  \"whishi\": %.3e,\n", bxp->whishi);
        _mys_append_buf(&buffer, &size, &used, "  \"fliers\": [");
        if (bxp->fliers != NULL) {
            for (size_t i = 0; i < bxp->n_fliers; i++) {
                _mys_append_buf(&buffer, &size, &used, "%.3e%s", bxp->fliers[i], (i < bxp->n_fliers - 1) ? ", " : "");
            }
        }
        _mys_append_buf(&buffer, &size, &used, "]\n");
        _mys_append_buf(&buffer, &size, &used, "}");
    } else {
        _mys_append_buf(&buffer, &size, &used, "{");
        _mys_append_buf(&buffer, &size, &used, "\"whislo\": %.3e, ", bxp->whislo);
        _mys_append_buf(&buffer, &size, &used, "\"q1\": %.3e, ", bxp->q1);
        _mys_append_buf(&buffer, &size, &used, "\"med\": %.3e, ", bxp->med);
        _mys_append_buf(&buffer, &size, &used, "\"q3\": %.3e, ", bxp->q3);
        _mys_append_buf(&buffer, &size, &used, "\"whishi\": %.3e, ", bxp->whishi);
        _mys_append_buf(&buffer, &size, &used, "\"fliers\": [");
        if (bxp->fliers != NULL) {
            for (size_t i = 0; i < bxp->n_fliers; i++) {
                _mys_append_buf(&buffer, &size, &used, "%.3e%s", bxp->fliers[i], (i < bxp->n_fliers - 1) ? ", " : "");
            }
        }
        _mys_append_buf(&buffer, &size, &used, "]}");
    }

    // Null-terminate the buffer
    buffer[used] = '\0';
    return buffer;
}

MYS_PUBLIC char *mys_boxplot_serialize(const mys_boxplot_t *bxp)
{
    return _mys_boxplot_serialize_impl(bxp, false);
}

MYS_PUBLIC char *mys_boxplot_serialize_pretty(const mys_boxplot_t *bxp)
{
    return _mys_boxplot_serialize_impl(bxp, true);
}
