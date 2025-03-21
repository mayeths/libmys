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
#pragma once

#include "_config.h"
#include "mpistubs.h"
#include "algorithm.h"
#include "math.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

MYS_PUBLIC double mys_arthimetic_mean(double *arr, int n);
MYS_PUBLIC double mys_harmonic_mean(double *arr, int n);
MYS_PUBLIC double mys_geometric_mean(double *arr, int n);
MYS_PUBLIC double mys_standard_deviation(double *arr, int n);

typedef struct mys_aggregate_t {
    double self; /* self value */
    double avg;  /* average value */
    double sum;  /* sum of values */
    double var;  /* variance */
    double std;  /* standard deviation */
    double max;  /* maximum value */
    double min;  /* minimum value */
    int loc_max  /* rank containing maximum value */;
    int loc_min; /* rank containing minimum value */
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
MYS_PUBLIC mys_aggregate_t mys_aggregate_analysis(double value);
/**
 * @brief Do aggregate analysis among all MPI ranks
 * 
 * @param n size of values and results array
 * @param values the data to be analysis
 * @param results resulting array to containing all infomations about values of all MPI ranks
 */
MYS_PUBLIC void mys_aggregate_analysis_array(size_t n, double *values, mys_aggregate_t *results);



typedef struct mys_boxplot_t {
    double whishi;    /* Top whisker position */
    double q3;        /* Third quartile (75th percentile) */
    double med;       /* Median (50th percentile) */
    double q1;        /* First quartile (25th percentile) */
    double whislo;    /* Bottom whisker position */
    double iqr;       /* Interquartile range (q3-q1) */
    size_t n_fliers;  /* Number of fliers */
    size_t nt_fliers; /* Number of top fliers */
    size_t nb_fliers; /* Number of bottom fliers */
    double *fliers;   /* Fliers (outliers) array. NULL if no fliers. */
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
/*
import numpy as np
import matplotlib.pyplot as plt

baseline_128 = #{"whislo": ... }
baseline_256 = #{"whislo": ... }
baseline_512 = #{"whislo": ... }
baseline_1024 = #{"whislo": ...}

opt_128 = #{"whislo": ... }
opt_256 = #{"whislo": ... }
opt_512 = #{"whislo": ... }
opt_1024 = #{"whislo": ...}

fig1, ax1 = plt.subplots(1, 1, figsize=(4.5, 2), dpi=400, layout='constrained')

flierprops = dict(markeredgecolor='#c5283d', markersize=2) # 离群值的设置（离群的点）
capprops = dict(color='#c5283d')
whiskerprops = dict(color='#c5283d') # 连接箱体和胡须的上下两条竖线
boxprops = dict(color='#c5283d')
medianprops = dict(color='#c5283d') # 中位数的设置（最中间的那个横线）
ax1.bxp([baseline_128, baseline_256, baseline_512, baseline_1024], positions=[1, 3, 5, 7], boxprops=boxprops, capprops=capprops, whiskerprops=whiskerprops, flierprops=flierprops, medianprops=medianprops)

flierprops = dict(markeredgecolor='#255f85', markersize=2)
capprops = dict(color='#255f85')
whiskerprops = dict(color='#255f85')
boxprops = dict(color='#255f85')
medianprops = dict(color='#255f85')
ax1.bxp([opt_128, opt_256, opt_512, opt_1024], positions=[1.5, 3.5, 5.5, 7.5], boxprops=boxprops, capprops=capprops, whiskerprops=whiskerprops, flierprops=flierprops, medianprops=medianprops)

plt.show()
*/

/**
 * @brief Return statistics string used to draw a series of box and whisker plots using bxp.
 * 
 * See mys/statistic.h for example of how to use returned result in python matplotlib.
 * 
 * `ax.bxp([res])`
 * 
 * See matplotlib.cbook.boxplot_stats and matplotlib.bxp for details.
 * 
 * @param values Data that will be represented in the boxplots
 * @param n Size of the data
 * @return char* JSON formatted string representing the boxplot statistics
 * 
 * @note The caller must free the returned string.
 */
MYS_PUBLIC char *mys_boxplot(double *values, size_t n);

/**
 * @brief Return statistics used to draw a series of box and whisker plots using bxp.
 * 
 * `ax.bxp([res])`
 * 
 * See mys/statistic.h for example of how to use returned result in python matplotlib.
 * 
 * See matplotlib.cbook.boxplot_stats and matplotlib.bxp for details.
 * 
 * @param values Data that will be represented in the boxplots
 * @param n Size of the data
 * @return mys_boxplot_t* Pointer to structure containing the calculated boxplot statistics
 * 
 * @note The caller must free the returned structure using `mys_boxplot_destroy`.
 * 
 * @note Autorange: when the data are distributed such that the 25th and 75th percentiles are equal,
 * whis is set to (0, 100) such that the whisker ends are at the minimum and maximum of the data.
 */
MYS_PUBLIC mys_boxplot_t *mys_boxplot_create(double *values, size_t n);

/**
 * @brief Free the memory allocated for the boxplot statistics.
 * 
 * @param bxp Pointer to the boxplot statistics structure to be freed
 */
MYS_PUBLIC void mys_boxplot_destroy(mys_boxplot_t **bxp);

/**
 * @brief Serialize the boxplot statistics to a JSON string.
 * 
 * See matplotlib.cbook.boxplot_stats and matplotlib.bxp for details.
 * 
 * @param bxp Pointer to the boxplot statistics structure to be serialized
 * @return char* JSON formatted string representing the boxplot statistics
 * 
 * @note The caller must free the returned string.
 */
MYS_PUBLIC char *mys_boxplot_serialize(const mys_boxplot_t *bxp);

/**
 * @brief Serialize the boxplot statistics to a pretty-printed JSON string.
 * 
 * See matplotlib.cbook.boxplot_stats and matplotlib.bxp for details.
 * 
 * @param bxp Pointer to the boxplot statistics structure to be serialized
 * @return char* Pretty-printed JSON formatted string representing the boxplot statistics
 * 
 * @note The caller must free the returned string.
 */
MYS_PUBLIC char *mys_boxplot_serialize_pretty(const mys_boxplot_t *bxp);

// MYS_PUBLIC char *mys_boxplot_serialize(const mys_boxplot_t *bxp, bool pretty_print);

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MYS_IMPL
#include <mys.h>

int main() {
    double arr[] = { 1, 3, 7, 300, 301, 302, 303, 304, 305, 306, 307, 308, 1000, 2000 };
    // obj = {"whislo": 3.200e-07, "q1": 3.300e-07, "med": 3.300e-07, "q3": 3.300e-07, "whishi": 4.200e-07, "fliers": []}
    // ax.bxp([obj])
    size_t n = sizeof(arr) / sizeof(double);
    char *json = mys_boxplot(arr, n);
    if (json) {
        printf("%s\n", json);
        free(json);
    }
    // import numpy as np
    // import matplotlib
    // import matplotlib.pyplot as plt
    // arr=[1, 3, 7, 300, 301, 302, 303, 304, 305, 306, 307, 308, 1000, 2000]
    // bxp = matplotlib.cbook.boxplot_stats(arr) # return bxp data in a list
    // print(bxp)
    // fig, ax = plt.subplots(1, 1, figsize=(4.5, 1.5), dpi=400, layout='constrained')
    // ax.bxp(bxp) # argument is list. bxp is already list here, we don't have to enclose it with []
    // plt.show()
    return 0;
}
*/
