#pragma once

#include <limits>
#include <vector>
#include <map>
#include <algorithm>
#include <typeinfo>
#include <cmath>
#include "_config.h"
#include "macro.h"

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
enum class FindZerosStrategy: int {
    DiagZeros,         /* count missing Aj[jj] == i or Aj[jj] == i && Av[jj] == 0 */
    DiagMissing,       /* count missing Aj[jj] == i */
    ExplicitZeros,     /* count Av[jj] == 0 */
    ExplicitDiagZeros, /* count Av[jj] == 0 */
};
enum class DataCleanStrategy: int {
    ToSquare,     /* Eliminate col < 0 || col >= nrows */
    OnlyNegative, /* Eliminate col < 0 */
};
enum class VectorNorm: int {
    Norm2 = 2,
    Norm2NoSquareRoot,
};

/* reindex Ap and Aj between Fortran and C */
template<typename index_t = int, typename data_t = double>
static int reindex(
    index_t nrow, index_t *Ap, index_t *Aj,
    MatrixIndexing oldindexing, MatrixIndexing newindexing)
{
    index_t oindexing = static_cast<index_t>(oldindexing);
    index_t nindexing = static_cast<index_t>(newindexing);
    index_t move = nindexing - oindexing;

    for (index_t ii = 0; ii < nrow; ii++) {
        index_t rowstart = Ap[ii] - oindexing;
        index_t rowstop = Ap[ii + 1] - oindexing;
        for (index_t jj = rowstart; jj < rowstop; jj++) {
            Aj[jj] = Aj[jj] + move;
        }
        Ap[ii] += move;
    }
    Ap[nrow] += move;

    return 0;
}

/* rebase Aj only */
template<typename index_t = int, typename data_t = double>
static void rebase(index_t nrow, index_t *Ap, index_t *Aj, index_t oldbase, index_t newbase = 0)
{
    const index_t move = newbase - oldbase;
    for (index_t i = 0; i < nrow; i++) {
        const index_t rowstart = Ap[i];
        const index_t rowstop = Ap[i + 1];
        for (index_t jj = rowstart; jj < rowstop; jj++) {
            Aj[jj] += move;
        }
    }
}

template<typename index_t = int, typename data_t = double>
static void dupcopy(
    const index_t nrows,
    const index_t *Ap, const index_t *Aj, const data_t *Av,
    index_t **tAp_, index_t **tAj_, data_t **tAv_)
{
    const index_t nnz = Ap[nrows];
    (*tAp_) = (index_t *)malloc((nrows + 1) * sizeof(index_t));
    (*tAj_) = (index_t *)malloc(nnz * sizeof(index_t));
    (*tAv_) = (data_t *)malloc(nnz * sizeof(data_t));
    std::copy(Ap, Ap + nrows + 1, (*tAp_));
    std::copy(Aj, Aj + nnz, (*tAj_));
    std::copy(Av, Av + nnz, (*tAv_));
}




template<typename index_t = int, typename data_t = double>
static int reorder(
    index_t nrow, index_t *Ap, index_t *Aj, data_t *Av,
    index_t base = 0, NonzeroOrder neworder = NonzeroOrder::StrictAscending)
{
    for (index_t ii = 0; ii < nrow; ii++) {
        index_t i = ii + base;
        index_t rowstart = Ap[ii] - base;
        index_t rowstop = Ap[ii + 1] - base;
        index_t rownnz = rowstop - rowstart;

        typedef std::pair<index_t, data_t> tuple_t;

        // ASSERT(rownnz > 0, "Expect rownnz %d > 0 base %d i %d rowstart %d rowstop %d", rownnz, base, i, rowstart, rowstop);
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
            if (row.begin() != row.end()) it_start = row.begin() + 1;
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
static index_t findncols(const index_t nrows, const index_t * const Ap, const index_t * const Aj, const index_t base = 0)
{
    index_t mincol = std::numeric_limits<index_t>::max();
    index_t maxcol = std::numeric_limits<index_t>::min();
    for (index_t jj = 0; jj < Ap[nrows]; jj++) {
        mincol = std::min(mincol, Aj[jj]);
        maxcol = std::max(maxcol, Aj[jj]);
    }
    return maxcol + 1 - base;
}

template<typename index_t = int, typename data_t = double>
static index_t findzeros(const index_t nrows, index_t *Ap, index_t * Aj, data_t *Av, FindZerosStrategy strategy = FindZerosStrategy::DiagZeros, const index_t base = 0)
{
    index_t nmissingdiags = 0;
    index_t nexpdiagzeros = 0;
    index_t nexpzeros = 0;
    for (index_t i = 0; i < nrows; i++) {
        const index_t gi = base + i;
        const index_t rowstart = Ap[i];
        const index_t rowend = Ap[i + 1];
        bool missingdiag = true;
        for (index_t jj = rowstart; jj < rowend; jj++) {
            const index_t gj = Aj[jj];
            const data_t v = Av[jj];
            if (v == 0) nexpzeros += 1;
            if (gi == gj) {
                missingdiag = false;
                if (v == 0) {
                    nexpdiagzeros += 1;
                }
            }
        }
        if (missingdiag)
            nmissingdiags += 1;
    }
    if (strategy == FindZerosStrategy::DiagZeros)
        return nmissingdiags + nexpdiagzeros;
    if (strategy == FindZerosStrategy::DiagMissing)
        return nmissingdiags;
    else if (strategy == FindZerosStrategy::ExplicitDiagZeros)
        return nexpdiagzeros;
    else if (strategy == FindZerosStrategy::ExplicitZeros)
        return nexpzeros;
    return -1;
}

template<typename index_t = int, typename data_t = double>
static void dataclean(const index_t nrows, index_t *Ap, index_t * Aj, data_t *Av, DataCleanStrategy strategy = DataCleanStrategy::ToSquare, const index_t base = 0)
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
        // const index_t gi = base + i;
        const index_t rowstart = Ap[i];
        const index_t rowend = Ap[i + 1];
        // index_t rownnz = 0;
        for (index_t jj = rowstart; jj < rowend; jj++) {
            const index_t gj = Aj[jj];
            const data_t v = Av[jj];
            bool uf = gj < base;
            bool of = gj >= base + nrows;
            bool out = uf || of;
            if (strategy == DataCleanStrategy::ToSquare && out)
                continue;
            else if (strategy == DataCleanStrategy::OnlyNegative && uf)
                continue;
            newAj[count] = gj;
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
            const index_t gi = fIa[jj];
            const index_t gj = fJa[jj];
            counts[gi] += 1;
            Istart = std::min(Istart, gi);
            Jstart = std::min(Jstart, gj);
            Iend = std::max(Iend, gi + 1);
            Jend = std::max(Jend, gj + 1);
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
            const index_t gi = fIa[jj];
            const index_t gj = fJa[jj];
            const data_t V = fVa[jj];
            const index_t i = gi - Istart;
            const index_t jj2 = (*tIa_)[i] + rowcounts[i];
            (*tJa_)[jj2] = gj;
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

template<class DATA_T, class FDATA_T = DATA_T>
static int WriteVector(const char *fname, DATA_T *arr, int n) {
    FILE *fp = fopen(fname, "wb");
    ASSERT(fp != NULL, "Failed to open %s", fname);
    for (int i = 0; i < n; i++) {
        FDATA_T tmp = static_cast<FDATA_T>(arr[i]);
        fwrite(&tmp, sizeof(FDATA_T), 1, fp);
    }
    {
        fclose(fp);
        printf("Saved %s with len %d and type %s\n", fname, n, typeid(FDATA_T).name());
    }
    return 0;
}
template<class DATA_T, class FDATA_T = DATA_T>
static int ReadVector(const char *fname, DATA_T **arr_, int *n_) {
    FILE *fp = fopen(fname, "rb");
    ASSERT(fp != NULL, "Failed to open %s", fname);
    {
        fseek(fp, 0, SEEK_END);
        (*n_) = ftell(fp) / sizeof(FDATA_T);
        fseek(fp, 0, SEEK_SET);
        (*arr_) = (DATA_T *)calloc((*n_), sizeof(DATA_T));
    }
    for (int i = 0; i < (*n_); i++) {
        FDATA_T tmp;
        fread(&tmp, sizeof(FDATA_T), 1, fp);
        (*arr_)[i] = static_cast<DATA_T>(tmp);
    }
    {
        ASSERT(getc(fp) == EOF, "Expect %s meet EOF after read %d values", fname, (*n_));
        fclose(fp);
        printf("Loaded %s with len %d and type %s\n", fname, (*n_), typeid(FDATA_T).name());
    }
    return 0;
}

template<class INDEX_T, class DATA_T>
static DATA_T CalcResidual(INDEX_T nrow, INDEX_T *Ap, INDEX_T *Aj, DATA_T *Av, DATA_T *x, DATA_T *b) {
    std::vector<DATA_T> r(nrow);
    for (int i = 0; i < nrow; i++) {
        const int rowstart = Ap[i];
        const int rowend = Ap[i + 1];
        DATA_T acc = 0;
        for (int jj = rowstart; jj < rowend; jj++) {
            int j = Aj[jj];
            DATA_T v = Av[jj];
            acc += v * x[j];
        }
        r[i] = b[i] - acc;
    }
    DATA_T norm2 = 0;
    for (int i = 0; i < r.size(); i++) {
        norm2 += r[i] * r[i];
    }
    return sqrt(norm2);
}
