#pragma once

#include "ISSAbstract.hpp"

template<
    typename matrix_t,
    typename vector_t,
    typename index_t = int,
    typename data_t = double,
    typename pcdata_t = data_t,       /* preconditioner data_t */
    typename intermediate_t = data_t> /* intermediate data_t */
class GCR : public ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
public:
    using BASE = ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    GCR() : BASE() { }
    GCR(AType &A) : BASE(A) { }
    GCR(AType &A, BType &B) : BASE(A, B) { }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const AType &A = *(this->A);
        const BType &B = *(this->B);
        VType r(x), u(x), p(x), s(x);

        intermediate_t bnorm, rnorm, alpha, beta, gamma, gammaold, delta;
        bnorm = (b, b);
        r = b - A * x;
        u = B * r;
        p = u;

        for (this->iter = 0; this->iter < this->maxiter; this->iter++) {
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
        }
    }

};