#include "../statistic.h"

MYS_API double mys_arthimetic_mean(double *arr, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return (1 / (double)n) * sum;
}

MYS_API double mys_harmonic_mean(double *arr, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += 1 / arr[i];
    }
    return ((double)n) / sum;
}

MYS_API double mys_geometric_mean(double *arr, int n)
{
    double product = 1;
    for (int i = 0; i < n; i++) {
        product *= arr[i];
    }
    return _mys_math_pow(product, 1 / (double)n);
}

MYS_API double mys_standard_deviation(double *arr, int n)
{
    double xbar = mys_arthimetic_mean(arr, n);
    double denom = 0;
    double nom = n - 1;
    for (int i = 0; i < n; i++) {
        double diff = arr[i] - xbar;
        denom += diff * diff;
    }
    return _mys_math_sqrt(denom / nom);
}

MYS_API void mys_aggregate_analysis_array(size_t n, double *values, mys_aggregate_t *results)
{
    mys_mpi_init();
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    struct di_t { double d; int i; };
    struct di_t *dibuf = (struct di_t *)malloc(sizeof(struct di_t) * n);
    double *dbuf = (double *)((void *)dibuf);

    {// mine, sum, avg
        _mys_MPI_Allreduce(values, dbuf, n, _mys_MPI_DOUBLE, _mys_MPI_SUM, mys_mpi_comm());
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
        _mys_MPI_Allreduce(_mys_MPI_IN_PLACE, dbuf, n, _mys_MPI_DOUBLE, _mys_MPI_SUM, mys_mpi_comm());
        for (size_t i = 0; i < n; i++) {
            results[i].var = dbuf[i] / (double)nranks;
            results[i].std = _mys_math_sqrt(results[i].var);
        }
    }
    {// max and min
        for (size_t i = 0; i < n; i++) {
            dibuf[i].d = values[i];
            dibuf[i].i = myrank;
        }
        _mys_MPI_Allreduce(_mys_MPI_IN_PLACE, dibuf, n, _mys_MPI_DOUBLE_INT, _mys_MPI_MAXLOC, mys_mpi_comm());
        for (size_t i = 0; i < n; i++) {
            results[i].max = dibuf[i].d;
            results[i].loc_max = dibuf[i].i;
        }

        for (size_t i = 0; i < n; i++) {
            dibuf[i].d = values[i];
            dibuf[i].i = myrank;
        }
        _mys_MPI_Allreduce(_mys_MPI_IN_PLACE, dibuf, n, _mys_MPI_DOUBLE_INT, _mys_MPI_MINLOC, mys_mpi_comm());
        for (size_t i = 0; i < n; i++) {
            results[i].min = dibuf[i].d;
            results[i].loc_min = dibuf[i].i;
        }
    }
    free(dibuf);
}

MYS_API mys_aggregate_t mys_aggregate_analysis(double value)
{
    mys_aggregate_t result;
    mys_aggregate_analysis_array(1, &value, &result);
    return result;
}


static mys_boxplot_t _mys_boxplot_impl(double *values, size_t n, bool save_fliers)
{
    mys_boxplot_t bxp;
    bxp.whislo = bxp.q1 = bxp.med = bxp.q3 = bxp.whishi = values[0];
    bxp.iqr = 0;
    bxp.n_fliers = 0;
    bxp.nb_fliers = 0;
    bxp.nt_fliers = 0;
    bxp.fliers = NULL;

    if (n == 1)
        return bxp;

    double *arr = (double *)malloc(sizeof(double) * n);
    memcpy(arr, values, sizeof(double) * n);
    qsort(arr, n, sizeof(double), mys_sortfn_f64);

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
    // To zero indexing
    ig_q1 -= 1;
    ig_q2 -= 1;
    ig_q3 -= 1;
    size_t arg_q1 = (size_t)(ig_q1);
    size_t arg_q2 = (size_t)(ig_q2);
    size_t arg_q3 = (size_t)(ig_q3);
    bxp.q1  = arr[arg_q1] + (arr[arg_q1 + 1] - arr[arg_q1]) * (ig_q1 - arg_q1);
    bxp.med = arr[arg_q2] + (arr[arg_q2 + 1] - arr[arg_q2]) * (ig_q2 - arg_q2);
    bxp.q3  = arr[arg_q3] + (arr[arg_q3 + 1] - arr[arg_q3]) * (ig_q3 - arg_q3);

    bxp.iqr = bxp.q3 - bxp.q1;
    double loval = bxp.q1 - 1.5 * bxp.iqr;
    double hival = bxp.q3 + 1.5 * bxp.iqr;

    bxp.whislo = bxp.q1;
    bxp.whishi = bxp.q3;
    bxp.nb_fliers = 0;
    bxp.nt_fliers = 0;
    for (size_t i = 0; i <= arg_q1; i++) {
        if (arr[i] < loval)
            bxp.nb_fliers += 1;
        if (arr[i] >= loval && arr[i] < bxp.whislo)
            bxp.whislo = arr[i];
    }
    for (size_t i = arg_q3; i < n; i++) {
        if (arr[i] <= hival && arr[i] > bxp.whishi)
            bxp.whishi = arr[i];
        if (arr[i] > hival)
            bxp.nt_fliers += 1;
    }
    bxp.n_fliers = bxp.nt_fliers + bxp.nb_fliers;

    if (save_fliers) {
        bxp.fliers = (double *)malloc(sizeof(double) * (bxp.nb_fliers + bxp.nt_fliers));
        size_t c = 0;
        for (size_t i = 0; i < bxp.nb_fliers; i++)
            bxp.fliers[c++] = arr[i];
        for (size_t i = n - bxp.nt_fliers; i < n; i++)
            bxp.fliers[c++] = arr[i];
    }

    free(arr);
    return bxp;
}

MYS_API mys_boxplot_t mys_boxplot(double *values, size_t n)
{
    return _mys_boxplot_impl(values, n, true);
}

MYS_API mys_boxplot_t mys_boxplot_noflier(double *values, size_t n)
{
    return _mys_boxplot_impl(values, n, false);
}

// MYS_API char *mys_boxplot_serialize(const mys_boxplot_t *bxp, bool pretty_print)
// {
//     if (pretty_print) {
//         printf("{\n");
//         printf("  'whislo': %.7f,\n", bxp->whislo);
//         printf("  'q1': %.7f,\n", bxp->q1);
//         printf("  'med': %.7f,\n", bxp->med);
//         printf("  'q3': %.7f,\n", bxp->q3);
//         printf("  'whishi': %.7f,\n", bxp->whishi);
//         printf("  'fliers': [");
//         for (size_t i = 0; i < bxp->n_fliers; i++) {
//             printf("%.7f, ", bxp->fliers[i]);
//         }
//         printf("]\n");
//         printf("}\n");
//     } else {
//         printf("{");
//         printf("'whislo': %.6e, ", bxp->whislo);
//         printf("'q1': %.6e, ", bxp->q1);
//         printf("'med': %.6e, ", bxp->med);
//         printf("'q3': %.6e, ", bxp->q3);
//         printf("'whishi': %.6e, ", bxp->whishi);
//         printf("'fliers': [");
//         for (size_t i = 0; i < bxp->n_fliers; i++) {
//             printf("%.6e, ", bxp->fliers[i]);
//         }
//         printf("]");
//         printf("}\n");
//     }
// }
