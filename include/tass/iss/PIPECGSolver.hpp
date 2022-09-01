#pragma once

#include "ISSAbstract.hpp"
#include "../blas/BLAS1.hpp"
#include "../blas/BLAS2.hpp"

template<
    typename matrix_t,
    typename vector_t,
    typename index_t = int,
    typename data_t = double,
    typename pcdata_t = data_t,       /* preconditioner data_t */
    typename intermediate_t = data_t> /* intermediate data_t */
class PIPECGSolver : public ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
public:
    using BASE = ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    PIPECGSolver() : BASE() { }
    PIPECGSolver(AType &A) : BASE(A) { }
    PIPECGSolver(AType &A, BType &B) : BASE(A, B) { }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const AType &A = *(this->A);
        const BType &B = *(this->B);
        VType r(x), z(x), p(x), n(x), w(x), q(x), u(x), m(x), s(x);

        intermediate_t bnorm = std::sqrt((b, b));
        r = b - A * x;
        u = B * r;
        w = A * u;

        intermediate_t gamma, gammaold, delta, alpha, beta;

        for (this->iter = 0; this->iter < this->maxiter; this->iter++) {
            intermediate_t rnorm = std::sqrt((r, r));
            intermediate_t rel = rnorm / bnorm;
            DEBUG(0, "iteration %4d ||r|| %.17e ||r||/||b|| %.17e", this->iter, rnorm, rel);
            if (rnorm < this->atol)
                break;

            gammaold = gamma;
            gamma = (r, u);
            delta = (w, u);
            m = B * w;
            n = A * m;

            if (this->iter > 0) {
                beta = gamma / gammaold;
                alpha = gamma / (delta - beta / alpha * gamma);
            } else {
                beta = 0;
                alpha = gamma / delta;
            }

            z = beta * z + n;
            q = beta * q + m;
            p = beta * p + u;
            s = beta * s + w;
            x += alpha * p;
            u -= alpha * q;
            w -= alpha * z;
            r -= alpha * s;
        }
    }

};
