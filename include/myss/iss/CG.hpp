#pragma once

#include "ISSAbstract.hpp"

template<
    typename matrix_t,
    typename intermediate_t = typename matrix_t::DType> /* intermediate DType */
class CG : public ISSAbstract<matrix_t>
{
public:
    using BASE = ISSAbstract<matrix_t>;
    using IType = typename BASE::IType;
    using DType = typename BASE::DType;
    using VType = typename BASE::VType;
    using MType = typename BASE::MType;
    using PType = typename BASE::PType;
    CG() = delete;
    CG(const MType &A) : BASE(A) { }
    CG(const MType &A, const PType &B) : BASE(A, B) { }

    virtual const char *GetName() const {
        return "CG";
    }

    virtual void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const MType &A = this->GetMatrix();
        const PType &B = this->GetPreconditioner();
        VType r(x), u(x), p(x), s(x);

        intermediate_t bnorm, rnorm, alpha, beta, gamma, gammaold, delta;
        bnorm = (b, b);
        r = b - A * x;
        u = B * r;
        p = u;

        do {
            rnorm = (r, r);
            if (this->Converged(std::sqrt(rnorm), std::sqrt(bnorm)))
                break;

            gammaold = this->iter == 0 ? (intermediate_t)(r, u) : gamma;
            s = A * p;
            delta = (s, p);
            alpha = gammaold / delta;
            x = x + alpha * p;
            r = r - alpha * s;
            u = B * r;
            gamma = (r, u);
            beta = gamma / gammaold;
            p = u + beta * p;
        } while (++this->iter);
    }

};
