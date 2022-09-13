#pragma once

#include "../pc/PCBase.hpp"
#include "../pc/PCNone.hpp"
#include "../util/AsyncProxy.hpp"

template<typename matrix_t>
class ISSBase
{
public:
    enum StopReason: int {
        NoStopped = 0,
        ConvergedByAtol,
        ConvergedByRtol,
        DivergedByDtol,
        DivergedByMaxIter,
    };
    using MType = matrix_t;
    using PType = PCBase<MType>;
    using VType = typename MType::VType;
    using IType = typename MType::IType;
    using DType = typename MType::DType;
    using ConvergeTestFunction = StopReason(*)(
        const DType &abs, const DType &rel,
        const DType &atol, const DType &rtol, const DType &dtol,
        const IType &iter, const IType &maxiter);
protected:
    /* Intermediate Variables */
    mutable IType iter = 0, stopiter = 0;
    mutable DType stopabs = 0, stoprel = 0;
    mutable StopReason stopreason = StopReason::NoStopped;
    mutable ConvergeTestFunction convergetest = &ISSBase::DefaultConvergeTest;
    mutable const MType *A = nullptr;
    mutable const PType *P = nullptr;
    mutable const PType *defaultP = nullptr;
public:

    /* Parameters Variables */
    IType verbose = -1;
    IType maxiter = 100;
    DType rtol = 1e-6, atol = 1e-6, dtol = 1e6;

    ISSBase() = delete;
    ISSBase(const MType &A) {
        this->A = &A;
        this->P = nullptr;
        this->defaultP = new PCNone<typename PType::MType>();
    }
    ISSBase(const MType &A, const PType &P) {
        this->A = &A;
        this->P = &P;
        this->defaultP = nullptr;
    }
    ~ISSBase() {
        if (this->defaultP) delete this->defaultP;
        this->defaultP = nullptr;
    }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    IType GetNumIterations() const { return this->iter; }
    StopReason GetStopReason() const { return this->stopreason; }
    const MType &GetMatrix() const { return *this->A; }
    const PType &GetPreconditioner() const { return this->P == nullptr ? *this->defaultP : *this->P; }
    void SetMatrix(const MType &A) { this->A = &A; }
    void SetPreconditioner(const PType &P) { this->P = &P; }

    const char *GetStopReasonName() const {
        if (this->stopreason == StopReason::ConvergedByAtol) return "ConvergedByAtol";
        else if (this->stopreason == StopReason::ConvergedByRtol) return "ConvergedByRtol";
        else if (this->stopreason == StopReason::DivergedByDtol) return "DivergedByDtol";
        else if (this->stopreason == StopReason::DivergedByMaxIter) return "DivergedByMaxIter";
        return "NoStopped";
    }

    StopReason Converged(const DType &rnorm, const DType bnorm) const {
        DType abs = std::abs(rnorm);
        DType rel = abs / std::abs(bnorm);
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
        const MType &matrix = this->GetMatrix();
        const PType &precond = this->GetPreconditioner();

        buffer += strformat("%s @%p\n", this->GetName(), this);
        buffer += strformat("  Matrix: %s @%p\n", matrix.GetName(), &matrix);
        if (this->P == nullptr)
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
            const char *rmark = this->stopreason == StopReason::ConvergedByRtol ? ">" :
                                this->stopreason == StopReason::DivergedByDtol  ? "x" : " ";
            const char *imark = this->stopreason == StopReason::DivergedByMaxIter ? "x" : " ";
            buffer += strformat("  %s abs %.17g (atol %.17g)\n", amark , (double)stopabs, (double)atol);
            buffer += strformat("  %s rel %.17g (rtol %.17g dtol %.17g)\n", rmark, (double)stoprel, (double)rtol, (double)dtol);
            buffer += strformat("  %s iter %d (maxiter %d)\n", imark, (int)stopiter, (int)maxiter);
        }
        PRINTF(0, "%s", buffer.c_str());
    }

    static StopReason DefaultConvergeTest(
        const DType &abs, const DType &rel,
        const DType &atol, const DType &rtol, const DType &dtol,
        const IType &iter, const IType &maxiter)
    {
        int pref = trunc(log10(maxiter)) + 1;
        PRINTF(0, "Iteration %*d ||r|| %.17e ||r||/||b|| %.17e\n", pref, iter, abs, rel);
        if (abs <= atol) return StopReason::ConvergedByAtol;
        else if (rel <= rtol) return StopReason::ConvergedByRtol;
        else if (rel >= dtol && iter != 0) return StopReason::DivergedByDtol;
        else if (iter >= maxiter) return StopReason::DivergedByMaxIter;
        return StopReason::NoStopped;
    }
};
