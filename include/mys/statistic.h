#pragma once

#include "_config.h"
#include "mpi.h"
#include "algorithm.h"

#include <stdio.h>
#include <stdlib.h>

MYS_API double mys_arthimetic_mean(double *arr, int n);
MYS_API double mys_harmonic_mean(double *arr, int n);
MYS_API double mys_geometric_mean(double *arr, int n);
MYS_API double mys_standard_deviation(double *arr, int n);

typedef struct mys_aggregate_t {
    /* self value */
    double self;
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

/**
 * @brief Do aggregate analysis among all MPI ranks
 * 
 * @param value The value to be analysis
 * @return `mys_aggregate_t` containing all infomations about value of all MPI ranks
 * 
 * @note Comment usage
 * @note
 * `mys_aggregate_t agg = mys_aggregate_analysis(time);`
 * @note
 * `DLOG(0, "time | self %6.3f | avg %6.3f | min %6.3f (%4d) | max %6.3f (%4d)", agg.self, agg.avg, agg.min, agg.loc_min, agg.max, agg.loc_max);`
 */
MYS_API mys_aggregate_t mys_aggregate_analysis(double value);
/**
 * @brief Do aggregate analysis among all MPI ranks
 * 
 * @param n size of values and results array
 * @param values the data to be analysis
 * @param results resulting array to containing all infomations about values of all MPI ranks
 */
MYS_API void mys_aggregate_analysis_array(size_t n, double *values, mys_aggregate_t *results);



typedef struct mys_boxplot_t {
    double whishi;    // Top whisker position
    double q3;        // Third quartile (75th percentile)
    double med;       // Median         (50th percentile)
    double q1;        // First quartile (25th percentile)
    double whislo;    // Bottom whisker position
    double iqr;       // Interquartile range (q3-q1)
    size_t n_fliers;  // Number of fliers
    size_t nt_fliers; // Number of top fliers
    size_t nb_fliers; // Number of bottom fliers
    double *fliers;
} mys_boxplot_t;

/*
       o                 fliers (outliers) 离群值，超过上下四分位数1.5倍IQR的值，样式flierprops
       o                 fliers (outliers) 离群值，超过上下四分位数1.5倍IQR的值，样式flierprops
     -----  (Q3+1.5IQR)  Top whisker 上胡须，样式capprops
       |                 样式whiskerprops
     -----  (Q3) ^       Third quartile (75th percentile) 第三个四分位数，Box 上边界，样式boxprops
    |     |      | IQR   样式boxprops
    |     |      |       样式boxprops
     -----       |       Median (50th percentile) 中位数，Box 正中间，样式medianprops
    |     |      |       样式boxprops
    |     |      |       样式boxprops
     -----  (Q1) v       First quartile (25th percentile) 第一个四分位数，Box 下边界，样式boxprops
       |                 样式whiskerprops
     -----  (Q1-1.5IQR)  Bottom whisker 下胡须，样式capprops
       o                 fliers (outliers) 离群值，超过上下四分位数1.5倍IQR的值，样式flierprops
 超过多少IQR倍数是不定的，看你自己的选择，1.5或者2或者5都行
*/

/**
 * @brief Return statistics used to draw a series of box and whisker plots using bxp. See matplotlib.cbook.boxplot_stats and matplotlib.bxp
 * 
 * @param values Data that will be represented in the boxplots
 * @param n Size of data
 * 
 * @note You should free returned fliers array yourself.
 */
MYS_API mys_boxplot_t mys_boxplot(double *values, size_t n);
/**
 * @brief Return statistics used to draw a series of box and whisker plots using bxp. See matplotlib.cbook.boxplot_stats and matplotlib.bxp
 * 
 * @param values Data that will be represented in the boxplots
 * @param n Size of data
 * 
 * @note This interface wouldn't malloc fliers array.
 */
MYS_API mys_boxplot_t mys_boxplot_noflier(double *values, size_t n);

// MYS_API char *mys_boxplot_serialize(const mys_boxplot_t *bxp, bool pretty_print);

/*
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.h"

int main() {
    double aaa[] = {
        1, 3, 7,
        300, 301, 302, 303, 304, 305, 306, 307, 308,
        1000, 2000
    };
    size_t n = sizeof(aaa) / sizeof(double);
    mys_boxplot_t bxp = mys_boxplot(aaa, n);
    printf("{\n");
    printf("  'whislo': %.7f,\n", bxp.whislo);
    printf("  'q1': %.7f,\n", bxp.q1);
    printf("  'med': %.7f,\n", bxp.med);
    printf("  'q3': %.7f,\n", bxp.q3);
    printf("  'whishi': %.7f,\n", bxp.whishi);
    printf("  'fliers': [");
    for (size_t i = 0; i < bxp.n_fliers; i++) {
        printf("%.7f, ", bxp.fliers[i]);
    }
    printf("]\n");
    printf("}\n");
    free(bxp.fliers);
    // import numpy as np
    // import matplotlib.pyplot as plt
    // fig, ax = plt.subplots(1, 1, figsize=(4.5, 1.5), dpi=400, layout='constrained')
    // ax.bxp(HYPRE_boxes, showfliers=False)
	return 0;
}
*/
