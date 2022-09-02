#pragma once

#include "ISSAbstract.hpp"
#include "../util/async.hpp"

template<
    typename matrix_t,
    typename vector_t,
    typename index_t = int,
    typename data_t = double,
    typename pcdata_t = data_t,       /* preconditioner data_t */
    typename intermediate_t = data_t> /* intermediate data_t */
class CGSolver : public ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
public:
    using BASE = ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    CGSolver() : BASE() { }
    CGSolver(AType &A) : BASE(A) { }
    CGSolver(AType &A, BType &B) : BASE(A, B) { }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const AType &A = *(this->A);
        const BType &B = *(this->B);
        VType r(x), u(x), p(x), s(x);

        intermediate_t bnorm = std::sqrt((b, b));
        r = b - A * x;
        u = B * r;
        p = u;

        intermediate_t gammaold, alpha, beta;
        async<intermediate_t> gamma, delta;

        for (this->iter = 0; this->iter < this->maxiter; this->iter++) {
            gammaold = this->iter == 0 ? (r, u) : gamma;
            intermediate_t rnorm = std::sqrt((r, r));
            intermediate_t rel = rnorm / bnorm;
            DEBUG(0, "iteration %4d ||r|| %.17e ||r||/||b|| %.17e", this->iter, rnorm, rel);
            if (rnorm < this->atol)
                break;

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
