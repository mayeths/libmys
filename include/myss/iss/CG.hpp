#pragma once

#include "ISSAbstract.hpp"

template<
    typename matrix_t,
    typename intermediate_t = typename matrix_t::data_t> /* intermediate data_t */
class CG : public ISSAbstract<matrix_t>
{
public:
    using BASE = ISSAbstract<matrix_t>;
    using index_t = typename BASE::index_t;
    using data_t = typename BASE::data_t;
    using pcdata_t = typename BASE::data_t;
    using VType = typename BASE::VType;
    using RealVType = typename BASE::RealVType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    CG() = delete;
    CG(const AType &A) : BASE(A) { }
    CG(const AType &A, const BType &B) : BASE(A, B) { }

    virtual const char *GetName() const {
        return "CG";
    }

    virtual void Apply(const RealVType &b, RealVType &x, bool xzero = false) const
    {
        const AType &A = this->GetMatrix();
        const BType &B = this->GetPreconditioner();
        RealVType r(x), u(x), p(x), s(x);

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
