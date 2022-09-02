#pragma once

#include "ISSAbstract.hpp"

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

        intermediate_t gammaold, alpha, beta;
        async<intermediate_t> gamma, delta, rnorm;

        for (this->iter = 0; this->iter < this->maxiter; this->iter++) {
            rnorm = (r, r);
            if (this->Converged(std::sqrt(rnorm), bnorm))
                break;

            gammaold = gamma;
            gamma = (r, u);
            delta = (w, u);
            m = B * w;
            n = A * m;

            beta = this->iter == 0 ? 0 : gamma / gammaold;
            alpha = gamma / (delta - beta / alpha * gamma);
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
