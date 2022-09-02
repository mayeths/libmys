#pragma once

#include "../pc/PCAbstract.hpp"
#include "../util/async.hpp"

template<typename matrix_t, typename vector_t, typename index_t, typename data_t, typename pcdata_t>
class ISS
{
public:
    using VType = vector_t;
    using AType = matrix_t;
    using BType = PCAbstract<matrix_t, vector_t, index_t, pcdata_t>;
    using ConvergedFunction = bool(*)(const data_t &rnorm, const data_t &bnorm, const data_t &atol, const data_t &rtol, const index_t &iter);
    AType *A = nullptr;
    BType *B = nullptr;

    index_t verbose = -1;
    index_t maxiter = 100;
    data_t rtol = 1e-6, atol = 1e-6;

    mutable index_t iter, converged;
    mutable data_t norm;
    mutable ConvergedFunction convergetest = &ISS::DefaultConvergeTest;

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
    bool Converged(const data_t &norm, const data_t bnorm) const { return this->convergetest(norm, bnorm, this->atol, this->rtol, this->iter); }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;

    static bool DefaultConvergeTest(const data_t &rnorm, const data_t &bnorm, const data_t &atol, const data_t &rtol, const index_t &iter) {
        data_t abs = std::abs(rnorm);
        data_t rel = abs / bnorm;
        PRINTF(0, "Iteration %4d ||r|| %.17e ||r||/||b|| %.17e\n", iter, abs, rel);
        return (abs < atol || rel < rtol) ? true : false;
    }
};
