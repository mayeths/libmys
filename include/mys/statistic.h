#pragma once

#include "headers.h"
#include "os.h"

double arthimetic_mean(double *xarr, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += xarr[i];
    }
    return (1 / (double)n) * sum;
}

double harmonic_mean(double *xarr, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += 1 / xarr[i];
    }
    return ((double)n) / sum;
}

double geometric_mean(double *xarr, int n) {
    double product = 1;
    for (int i = 0; i < n; i++) {
        product *= xarr[i];
    }
    return pow(product, 1 / (double)n);
}

double standard_deviation(double *xarr, int n) {
    double xbar = arthimetic_mean(xarr, n);
    double denom = 0;
    double nom = n - 1;
    for (int i = 0; i < n; i++) {
        double diff = xarr[i] - xbar;
        denom += diff * diff;
    }
    return sqrt(denom / nom);
}

typedef struct CI_t {
  double low;
  double high;
} CI_t;

CI_t CI(double *xarr, int n, double confidence) {
    double xbar = arthimetic_mean(xarr, n);
    double s = standard_deviation(xarr, n);
    char tmp[4096] = {0};
    const char *code =
        "from scipy.stats import t\n"
        "from numpy import std, mean\n"
        "import sys\n"
        "def one_side_tdist(confidence, dof):\n"
        "    return -t.ppf(confidence, dof)\n"
        "def two_side_tdist(confidence, dof):\n"
        "    return one_side_tdist(confidence / 2, dof)\n"
        "def main():\n"
        "    if len(sys.argv) == 3:\n"
        "        confidence = float(sys.argv[1])\n"
        "        dof = float(sys.argv[2])\n"
        "        res = two_side_tdist(confidence, dof)\n"
        "        print(f\"{res}\")\n"
        "if __name__ == \"__main__\":\n"
        "    main()\n"
        ;

    sprintf(tmp, "python3 -c '%s' %f %d", code, confidence, n - 1);
    char *out = execshell(tmp);
    double t = atof(out);
    free(out);
    CI_t result;
    result.low = xbar - t * s / sqrt((double)n);
    result.high = xbar + t * s / sqrt((double)n);
    return result;
}