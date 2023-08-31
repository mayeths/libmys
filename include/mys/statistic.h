#pragma once

#include <stdio.h>
#include <stdlib.h>

MYS_API double mys_arthimetic_mean(double *arr, int n);
MYS_API double mys_harmonic_mean(double *arr, int n);
MYS_API double mys_geometric_mean(double *arr, int n);
MYS_API double mys_standard_deviation(double *arr, int n);

typedef struct mys_aggregate_t {
    /* my value */
    double mine;
    /* average value */
    double avg;
    /* sum of values */
    double sum;
    /* variance */
    double var;
    /* standard deviation */
    double std;
    /* maximum value */
    double max;
    /* minimum value */
    double min;
    /* rank containing maximum value */
    int loc_max;
    /* rank containing minimum value */
    int loc_min;
} mys_aggregate_t;
MYS_API mys_aggregate_t mys_aggregate_analysis(double value);

////// Legacy
#if !defined(MYS_NO_LEGACY) && !defined(MYS_NO_LEGACY_STATISTIC)
MYS_API double arthimetic_mean(double *arr, int n);
MYS_API double harmonic_mean(double *arr, int n);
MYS_API double geometric_mean(double *arr, int n);
MYS_API double standard_deviation(double *arr, int n);
#endif /*MYS_NO_LEGACY*/
