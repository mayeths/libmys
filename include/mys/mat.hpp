#pragma once

#include "mys/debug.h"
#include "mys/headers.hpp"

enum class MatrixType: int {
    CSR = 0,
    CSC = 1,
    COO = 2,
    S2D5  = 205,
    S2D9  = 209,
    S3D7  = 307,
    S3D19 = 319,
    S3D27 = 327,
};
enum class MatrixIndexing: int {
    ZeroBased = 0, /* C-style indexing */
    OneBased = 1,  /* Fortran-style indexing */
};
enum class NonzeroOrder: int {
    Undefined,
    StrictAscending,    /* Aj[i] < Aj[j] for i < j on each row */
    DiagFirstAscending, /* Aj[Ap[i]] == i */
};
enum class DataCleanStrategy: int {
    ToSquare,     /* Eliminate col < 0 || col >= nrows */
    OnlyNegative, /* Eliminate col < 0 */
};
enum class VectorNorm: int {
    Norm2 = 2,
    Norm2NoSquareRoot,
};

template<typename index_t = int, typename data_t = double>
static int reindex(
    index_t nrow, index_t *Ap, index_t *Aj,
    MatrixIndexing oldindexing, MatrixIndexing newindexing)
{
    index_t old_base = static_cast<index_t>(oldindexing);
    index_t new_base = static_cast<index_t>(newindexing);
    index_t move = new_base - old_base;

    for (index_t ii = 0; ii < nrow; ii++) {
        index_t rowstart = Ap[ii] - old_base;
        index_t rowstop = Ap[ii + 1] - old_base;
        for (index_t jj = rowstart; jj < rowstop; jj++) {
            Aj[jj] = Aj[jj] + move;
        }
        Ap[ii] += move;
    }
    Ap[nrow] += move;

    return 0;
}

template<typename index_t = int, typename data_t = double>
static int reorder(
    index_t nrow, index_t *Ap, index_t *Aj, data_t *Av,
    MatrixIndexing indexing, NonzeroOrder neworder)
{
    index_t base = static_cast<index_t>(indexing);

    for (index_t ii = 0; ii < nrow; ii++) {
        index_t i = ii + base;
        index_t rowstart = Ap[ii] - base;
        index_t rowstop = Ap[ii + 1] - base;
        index_t rownnz = rowstop - rowstart;

        typedef std::pair<index_t, data_t> tuple_t;

        ASSERT(rownnz > 0, "Expect rownnz %d > 0 base %d i %d rowstart %d rowstop %d", rownnz, base, i, rowstart, rowstop);
        std::vector<tuple_t> row(rownnz);
        for (index_t m = 0; m < rownnz; m++) {
            index_t jj = rowstart + m;
            index_t j = Aj[jj];
            data_t v = Av[jj];
            row[m] = std::make_pair(j, v);
        }

        auto it_start = row.begin();
        auto it_stop = row.end();
        if (neworder == NonzeroOrder::StrictAscending) {
            it_start = row.begin();
        } else if (neworder == NonzeroOrder::DiagFirstAscending) {
            for (index_t m = 0; m < rownnz; m++) {
                if (row[m].first == i) {
                    std::swap(row[0], row[m]);
                    break;
                }
            }
            it_start = row.begin() + 1;
        }
        std::sort(it_start, it_stop, [](tuple_t const &t1, tuple_t const &t2) {
            return std::get<0>(t1) < std::get<0>(t2);
        });

        for (index_t m = 0; m < rownnz; m++) {
            index_t jj = rowstart + m;
            Aj[jj] = row[m].first;
            Av[jj] = row[m].second;
        }
    }

    return 0;
}

template<typename index_t = int>
static index_t findncols(const index_t nrows, const index_t * const Ap, const index_t * const Aj)
{
    index_t maxcol = -1;
    for (index_t jj = 0; jj < Ap[nrows]; jj++)
        maxcol = std::max(maxcol, Aj[jj]);
    return maxcol + 1;
}

template<typename index_t = int, typename data_t = double>
static void dataclean(const index_t nrows, index_t *Ap, index_t * Aj, data_t *Av, DataCleanStrategy strategy = DataCleanStrategy::ToSquare)
{
    index_t nnz = Ap[nrows];
    std::vector<index_t> newAp(nrows + 1, 0);
    std::vector<index_t> newAj(nnz, 0);
    std::vector<data_t> newAv(nnz, 0);
    std::copy(Ap, Ap + nrows + 1, newAp.begin());
    std::copy(Aj, Aj + nnz, newAj.begin());
    std::copy(Av, Av + nnz, newAv.begin());
    index_t count = 0;
    for (index_t i = 0; i < nrows; i++) {
        const index_t rowstart = Ap[i];
        const index_t rowend = Ap[i + 1];
        index_t rownnz = 0;
        for (index_t jj = rowstart; jj < rowend; jj++) {
            const index_t j = Aj[jj];
            const data_t v = Av[jj];
            if (strategy == DataCleanStrategy::ToSquare && j < 0 && j >= nrows)
                continue;
            else if (strategy == DataCleanStrategy::OnlyNegative && j < 0)
                continue;
            newAj[count] = j;
            newAv[count] = v;
            count += 1;
        }
        newAp[i + 1] = count;
    }
    newAj.resize(count);
    newAv.resize(count);
    std::copy(newAp.begin(), newAp.end(), Ap);
    std::copy(newAj.begin(), newAj.end(), Aj);
    std::copy(newAv.begin(), newAv.end(), Av);
}

template<typename index_t = int, typename data_t = double>
static void permute(const index_t n, data_t *farr, data_t *tarr, const index_t * const indexset)
{
    if (tarr != farr) {
        for (int i = 0; i < n; i++) tarr[i] = farr[indexset[i]];
    } else {
        std::vector<index_t> tmp(n);
        for (int i = 0; i < n; i++) tmp[i] = farr[indexset[i]];
        for (int i = 0; i < n; i++) farr[i] = tmp[i];
    }
}

template<typename index_t = int, typename data_t = double>
static void matconvert(
    const index_t fn, const index_t *fIa, const index_t *fJa, const data_t *fVa,
    index_t *nrows_, index_t *ncols_, index_t *Istart_, index_t *Jstart_,
    index_t **tIa_, index_t **tJa_, data_t **tVa_,
    const MatrixType ftype, const MatrixType ttype)
{
    if (ftype == MatrixType::COO && ttype == MatrixType::CSR) {
        index_t Istart = std::numeric_limits<index_t>::max();
        index_t Jstart = std::numeric_limits<index_t>::max();
        index_t Iend = std::numeric_limits<index_t>::min();
        index_t Jend = std::numeric_limits<index_t>::min();
        const index_t nnz = fn;
        std::map<index_t, index_t> counts;
        for (index_t jj = 0; jj < nnz; jj++) {
            const index_t I = fIa[jj];
            const index_t J = fJa[jj];
            counts[I] += 1;
            Istart = std::min(Istart, I);
            Jstart = std::min(Jstart, J);
            Iend = std::max(Iend, I + 1);
            Jend = std::max(Jend, J + 1);
        }
        index_t nrows = Iend - Istart;
        index_t ncols = Jend - Jstart;
        (*tIa_) = (index_t *)malloc((nrows + 1) * sizeof(index_t));
        (*tJa_) = (index_t *)malloc(nnz * sizeof(index_t));
        (*tVa_) = (data_t *)malloc(nnz * sizeof(data_t));
        (*tIa_)[0] = 0;
        for (index_t i = 0; i < nrows; i++) {
            (*tIa_)[i + 1] = (*tIa_)[i] + counts[i + Istart];
        }
        std::vector<index_t> rowcounts(nrows, 0);
        for (index_t jj = 0; jj < nnz; jj++) {
            const index_t I = fIa[jj];
            const index_t J = fJa[jj];
            const data_t V = fVa[jj];
            const index_t i = I - Istart;
            const index_t jj2 = (*tIa_)[i] + rowcounts[i];
            (*tJa_)[jj2] = J;
            (*tVa_)[jj2] = V;
            rowcounts[i] += 1;
        }
        (*nrows_) = nrows;
        (*ncols_) = ncols;
        (*Istart_) = Istart;
        (*Jstart_) = Jstart;
    } else {
        THROW_NOT_IMPL();
    }
}

template<typename index_t = int, typename data_t = double, typename intermediate_t = data_t>
static data_t matresnorm(const index_t nrow, const index_t *Ap, const index_t *Aj, const data_t *Av, const data_t *x, const data_t *b, const VectorNorm restype = VectorNorm::Norm2) {
    intermediate_t norm = 0;
    for (index_t i = 0; i < nrow; i++) {
        const index_t rowstart = Ap[i];
        const index_t rowend = Ap[i + 1];
        intermediate_t acc = 0;
        for (index_t jj = rowstart; jj < rowend; jj++) {
            const index_t j = Aj[jj];
            const data_t v = Av[jj];
            acc += v * x[j];
        }
        intermediate_t ri = b[i] - acc;
        norm += ri * ri;
    }
    if (restype == VectorNorm::Norm2)
        return std::sqrt(norm);
    else if (restype == VectorNorm::Norm2NoSquareRoot)
        return norm;
    return 0;
}

template<typename index_t = int, typename data_t = double, typename intermediate_t = data_t>
static data_t vecnorm(const index_t narr, const data_t *arr, const VectorNorm restype = VectorNorm::Norm2) {
    intermediate_t norm = 0;
    for (index_t i = 0; i < narr; i++)
        norm += arr[i] * arr[i];
    if (restype == VectorNorm::Norm2)
        return std::sqrt(norm);
    else if (restype == VectorNorm::Norm2NoSquareRoot)
        return norm;
    return 0;
}



/* Work in progress */
template <typename index_t = int, typename data_t = double>
struct CSR {
public:
    index_t nrow = 0;
    index_t ncol = 0;
    NonzeroOrder order = NonzeroOrder::Undefined;
    std::vector<index_t> Ia;
    std::vector<index_t> Ja;
    std::vector<data_t> Va;

    CSR() {}

    CSR(
        index_t nrow, index_t ncol,
        index_t *Ia = nullptr, index_t *Ja = nullptr, data_t *Va = nullptr,
        NonzeroOrder order = NonzeroOrder::Undefined
    ) : nrow(nrow), ncol(ncol), order(order)
    {
        if (Ia == nullptr || Ja == nullptr || Va == nullptr) return;
        index_t nnz = Ia[nrow];
        this->Ia.resize(nrow + 1);
        this->Ja.resize(nnz);
        this->Va.resize(nnz);
        std::copy(Ia, Ia + nrow + 1, this->Ia.data());
        std::copy(Ja, Ja + nnz, this->Ja.data());
        std::copy(Va, Va + nnz, this->Va.data());
    }

          index_t & I(index_t i)       { return this->Ia[i]; }
    const index_t & I(index_t i) const { return this->Ia[i]; }
          index_t & J(index_t i)       { return this->Ja[i]; }
    const index_t & J(index_t i) const { return this->Ja[i]; }
          data_t  & V(index_t i)       { return this->Va[i]; }
    const data_t  & V(index_t i) const { return this->Va[i]; }

    index_t nnz()       { return this->Ia[this->nrow]; }

    bool invalid(bool deepcheck = false) {
        if (deepcheck) {
            THROW_NOT_IMPL();
        }
        return (nrow < 0 || ncol < 0 || Ia == nullptr || Ja == nullptr || Va == nullptr);
    }

    void resizeI(index_t newsize) {
    }

    void resizeJ(index_t newsize) {
    }

    static CSR like(const CSR &model)
    {
        CSR tmp;
        tmp.nrow = model.nrow;
        tmp.ncol = model.ncol;
        tmp.order = model.order;
        tmp.valid = model.valid;
        index_t nnz = model.nnz();
        tmp.Ia.resize(tmp.nrow + 1);
        tmp.Ja.resize(nnz);
        tmp.Va.resize(nnz);
        return tmp;
    }

    static CSR copy(const CSR &model)
    {
        CSR tmp;
        tmp.nrow = model.nrow;
        tmp.ncol = model.ncol;
        tmp.order = model.order;
        tmp.valid = model.valid;
        index_t nnz = model.nnz();
        tmp.Ia.resize(tmp.nrow + 1);
        tmp.Ja.resize(nnz);
        tmp.Va.resize(nnz);
        std::copy(model.Ia.begin(), model.Ia.end(), tmp.Ia.begin());
        std::copy(model.Ja.begin(), model.Ja.end(), tmp.Ja.begin());
        std::copy(model.Va.begin(), model.Va.end(), tmp.Va.begin());
        return tmp;
    }

    ~CSR()
    {
        // if (this->Ia != nullptr) free(this->Ia);
        // if (this->Ja != nullptr) free(this->Ja);
        // if (this->Va != nullptr) free(this->Va);
    }

};
