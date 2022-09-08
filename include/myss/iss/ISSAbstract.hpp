#pragma once

#include "../pc/PCAbstract.hpp"
#include "../pc/PCNone.hpp"
#include "../util/AsyncProxy.hpp"

template<typename matrix_t, typename vector_t, typename index_t, typename data_t, typename pcdata_t>
class ISSAbstract
{
public:
    enum StopReason: int {
        NoStopped = 0,
        ConvergedByAtol,
        ConvergedByRtol,
        DivergedByDtol,
        DivergedByMaxIter,
    };
    using VType = vector_t;
    using AType = matrix_t;
    using BType = PCAbstract<matrix_t, vector_t, index_t, pcdata_t>;
    using ConvergeTestFunction = StopReason(*)(
        const data_t &abs, const data_t &rel,
        const data_t &atol, const data_t &rtol, const data_t &dtol,
        const index_t &iter, const index_t &maxiter);
protected:
    /* Intermediate Variables */
    mutable index_t iter = 0, stopiter = 0;
    mutable data_t stopabs = 0, stoprel = 0;
    mutable StopReason stopreason = StopReason::NoStopped;
    mutable ConvergeTestFunction convergetest = &ISSAbstract::DefaultConvergeTest;
    mutable const AType *A = nullptr;
    mutable const BType *B = nullptr;
    mutable const BType *defaultB = nullptr;
public:

    /* Parameters Variables */
    index_t verbose = -1;
    index_t maxiter = 100;
    data_t rtol = 1e-6, atol = 1e-6, dtol = 1e6;

    ISSAbstract() = delete;
    ISSAbstract(const AType &A) {
        this->A = &A;
        this->B = nullptr;
        this->defaultB = new PCNone<matrix_t, vector_t, index_t, pcdata_t>();
    }
    ISSAbstract(const AType &A, const BType &B) {
        this->A = &A;
        this->B = &B;
        this->defaultB = nullptr;
    }
    ~ISSAbstract() {
        if (this->defaultB) delete this->defaultB;
        this->defaultB = nullptr;
    }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    index_t GetNumIterations() const { return this->iter; }
    StopReason GetStopReason() const { return this->stopreason; }
    const AType &GetMatrix() const { return *this->A; }
    const BType &GetPreconditioner() const { return this->B == nullptr ? *this->defaultB : *this->B; }
    void SetMatrix(const AType &A) { this->A = &A; }
    void SetPreconditioner(const BType &B) { this->B = &B; }

    const char *GetStopReasonName() const {
        if (this->stopreason == StopReason::ConvergedByAtol) return "ConvergedByAtol";
        else if (this->stopreason == StopReason::ConvergedByRtol) return "ConvergedByRtol";
        else if (this->stopreason == StopReason::DivergedByDtol) return "DivergedByDtol";
        else if (this->stopreason == StopReason::DivergedByMaxIter) return "DivergedByMaxIter";
        return "NoStopped";
    }

    StopReason Converged(const data_t &rnorm, const data_t bnorm) const {
        data_t abs = std::abs(rnorm);
        data_t rel = abs / std::abs(bnorm);
        this->stopreason = this->convergetest(
            abs, rel,
            this->atol, this->rtol, this->dtol,
            this->iter, this->maxiter
        );
        if (this->stopreason != StopReason::NoStopped) {
            this->stopabs = abs;
            this->stoprel = rel;
            this->stopiter = this->iter;
        }
        return this->stopreason;
    }

    const void View(const std::string &prefix = "") const {
        std::string buffer = prefix;
        const AType &matrix = this->GetMatrix();
        const BType &precond = this->GetPreconditioner();

        buffer += strformat("%s @%p\n", this->GetName(), this);
        buffer += strformat("  Matrix: %s @%p\n", matrix.GetName(), &matrix);
        if (this->B == nullptr)
            buffer += strformat("  Preconditioner: No preconditioning\n");
        else
            buffer += strformat("  Preconditioner: %s @%p\n", precond.GetName(), &precond);

        buffer += strformat("  Status: %s\n", this->GetStopReasonName());
        if (this->stopreason == StopReason::NoStopped) {
            buffer += strformat(
                "    atol %.17g rtol %.17g dtol %.17g maxiter %d\n",
                (double)atol, (double)rtol, (double)dtol, (int)maxiter
            );
        } else {
            const char *amark = this->stopreason == StopReason::ConvergedByAtol ? ">" : " ";
            const char *rmark = this->stopreason == StopReason::ConvergedByRtol ||
                                this->stopreason == StopReason::DivergedByDtol ? ">" : " ";
            const char *imark = this->stopreason == StopReason::DivergedByMaxIter ? ">" : " ";
            buffer += strformat("  %s abs %.17g (atol %.17g)\n", amark , (double)stopabs, (double)atol);
            buffer += strformat("  %s rel %.17g (rtol %.17g dtol %.17g)\n", rmark, (double)stoprel, (double)rtol, (double)dtol);
            buffer += strformat("  %s iter %d (maxiter %d)\n", imark, (int)stopiter, (int)maxiter);
        }
        PRINTF(0, "%s", buffer.c_str());
    }

    static StopReason DefaultConvergeTest(
        const data_t &abs, const data_t &rel,
        const data_t &atol, const data_t &rtol, const data_t &dtol,
        const index_t &iter, const index_t &maxiter)
    {
        PRINTF(0, "Iteration %4d ||r|| %.17e ||r||/||b|| %.17e\n", iter, abs, rel);
        if (abs <= atol) return StopReason::ConvergedByAtol;
        else if (rel <= rtol) return StopReason::ConvergedByRtol;
        else if (rel >= dtol) return StopReason::DivergedByDtol;
        else if (iter >= maxiter) return StopReason::DivergedByMaxIter;
        return StopReason::NoStopped;
    }
};
