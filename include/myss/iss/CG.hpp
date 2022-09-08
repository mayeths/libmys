#pragma once

#include "ISSAbstract.hpp"

template<
    typename matrix_t,
    typename vector_t,
    typename index_t = int,
    typename data_t = double,
    typename pcdata_t = data_t,       /* preconditioner data_t */
    typename intermediate_t = data_t> /* intermediate data_t */
class CG : public ISSAbstract<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
public:
    using BASE = ISSAbstract<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    CG() = delete;
    CG(const AType &A) : BASE(A) { }
    CG(const AType &A, const BType &B) : BASE(A, B) { }

    virtual const char *GetName() const {
        return "CG";
    }

    virtual void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const AType &A = this->GetMatrix();
        const BType &B = this->GetPreconditioner();
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
