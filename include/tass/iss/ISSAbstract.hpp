#pragma once

#include "../pc/PCAbstract.hpp"

template<typename matrix_t, typename vector_t, typename index_t, typename data_t, typename pcdata_t>
class ISS
{
public:
    using VType = vector_t;
    using AType = matrix_t;
    using BType = PCAbstract<matrix_t, vector_t, index_t, pcdata_t>;
    AType *A = nullptr;
    BType *B = nullptr;

    index_t verbose = -1;
    index_t maxiter = 100;
    data_t rtol = 1e-6, atol = 1e-6;

    mutable index_t iter, converged;
    mutable data_t norm;

    ISS() : A(nullptr), B(nullptr) { }
    ISS(AType &A) { this->A = &A; this->B = nullptr; }
    ISS(AType &A, BType &B) { this->A = &A; this->B = &B; }

    void SetRtol(data_t num) { this->rtol = num; }
    void SetAtol(data_t num) { this->atol = num; }
    void SetMaxIter(index_t num) { this->maxiter = num; }
    void SetPrintLevel(index_t num) { this->verbose = num; }

    index_t GetNumIterations() const { return iter; }
    index_t GetConverged() const { return converged; }
    data_t GetFinalNorm() const { return norm; }

    void SetOperator(const AType &op) { this->A = &op; }
    void SetPreconditioner(const BType &op) { this->B = op; }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
};
