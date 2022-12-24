#pragma once

#include "MBase.hpp"
#include "../vec/VSeq.hpp"
#include "mys.hpp"
#include "mys/raii.hpp"

class MSeq : public MBase<VSeq, int, double>
{
public:
    using BASE = MBase<VSeq, int, double>;
    using VType = BASE::VType;
    using IType = BASE::IType;
    using DType = BASE::DType;

    int nrows = -1;
    std::vector<int> I;
    std::vector<int> J;
    std::vector<double> V;
    guard_t guard;

    MSeq() { }

    MSeq(const MSeq &src) {
        this->nrows = src.nrows;
        this->I.resize(src.I.size());
        this->J.resize(src.I.size());
        this->V.resize(src.I.size());
        std::copy(src.I.begin(), src.I.end(), this->I.begin());
        std::copy(src.J.begin(), src.J.end(), this->J.begin());
        std::copy(src.V.begin(), src.V.end(), this->V.begin());
        this->guard = src.guard;
    }

    MSeq(const int nrows, const int *Ap, const int *Aj, const double *Av) {
        int nnz = Ap[nrows];
        this->nrows = nrows;
        this->I.resize(nrows + 1);
        this->J.resize(nnz);
        this->V.resize(nnz);
        std::copy(&Ap[0], &Ap[nrows + 1], this->I.begin());
        std::copy(&Aj[0], &Aj[nnz - 1], this->J.begin());
        std::copy(&Av[0], &Av[nnz - 1], this->V.begin());
        this->guard.set();
    }

    ~MSeq() {
        this->nrows = -1;
        this->I.clear();
        this->J.clear();
        this->V.clear();
        this->guard.reset();
    }

    virtual void Apply(const VSeq &x, VSeq &y, bool xzero = false) const {
        this->guard.ensure();
        x.guard.ensure();
        y.guard.ensure();
        ASSERT_EQ(this->nrows, x.nrows);
        ASSERT_EQ(x.nrows, y.nrows);

        #pragma omp parallel for
        for (int i = 0; i < this->nrows; i++) {
            const int rowstart = this->I[i];
            const int rowstop = this->I[i + 1];
            y.values[i] = 0;
            for (int jj = rowstart; jj < rowstop; jj++) {
                const int j = this->J[jj];
                const double v = this->V[jj];
                y.values[i] += v * x.values[j];
            }
        }
    }
    virtual VSeq GetDiagonals() const {
        std::vector<double> values;
        int count = 0;
        int dc = 0;
        for (int i = 0; i < this->nrows; i++) {
            const int rowstart = this->I[i];
            const int rowstop = this->I[i + 1];
            for (int jj = rowstart; jj < rowstop; jj++) {
                const int j = this->J[jj];
                const double v = this->V[jj];
                if (j == i) {
                    values.push_back(v);
                    count += 1;
                }
            }
        }
        ASSERT_EQ(count, this->nrows);
        VSeq diag(values);
        return diag;
    }
    virtual const char *GetName() const {
        return "MSeq";
    }

};
