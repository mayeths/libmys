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
    double xbar = arthimetic_mean(arr, n);
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


#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_STATISTIC)
MYS_API double arthimetic_mean(double *arr, int n)
{
    return mys_arthimetic_mean(arr, n);
}

MYS_API double harmonic_mean(double *arr, int n)
{
    return mys_harmonic_mean(arr, n);
}

MYS_API double geometric_mean(double *arr, int n)
{
    return mys_geometric_mean(arr, n);
}

MYS_API double standard_deviation(double *arr, int n)
{
    return mys_standard_deviation(arr, n);
}
#endif
