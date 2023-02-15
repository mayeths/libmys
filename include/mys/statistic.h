#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

MYS_API double mys_arthimetic_mean(double *arr, int n);
MYS_API double mys_harmonic_mean(double *arr, int n);
MYS_API double mys_geometric_mean(double *arr, int n);
MYS_API double mys_standard_deviation(double *arr, int n);


////// Legacy

MYS_API static double arthimetic_mean(double *arr, int n) { return mys_arthimetic_mean(arr, n); }
MYS_API static double harmonic_mean(double *arr, int n) { return mys_harmonic_mean(arr, n); }
MYS_API static double geometric_mean(double *arr, int n) { return mys_geometric_mean(arr, n); }
MYS_API static double standard_deviation(double *arr, int n) { return mys_standard_deviation(arr, n); }
