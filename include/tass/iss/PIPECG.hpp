#pragma once

#include "ISSAbstract.hpp"

template<
    typename matrix_t,
    typename vector_t,
    bool enable_pipeline = true,
    typename index_t = int,
    typename data_t = double,
    typename pcdata_t = data_t,       /* preconditioner data_t */
    typename intermediate_t = data_t> /* intermediate data_t */
class PIPECG : public ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
    using AsyncIntermediate = AsyncProxy<intermediate_t>;
    using SyncIntermediate = SyncProxy<intermediate_t>;
    using pipe_intermediate_t = typename std::conditional<enable_pipeline, AsyncIntermediate, SyncIntermediate>::type;
public:
    using BASE = ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    PIPECG() : BASE() { }
    PIPECG(AType &A) : BASE(A) { }
    PIPECG(AType &A, BType &B) : BASE(A, B) { }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const AType &A = *(this->A);
        const BType &B = *(this->B);
        VType r(x), z(x), p(x), n(x), w(x), q(x), u(x), m(x), s(x);

        intermediate_t bnorm = 0, alpha = 1, beta = 1, gammaold = 0;
        pipe_intermediate_t rnorm = 0, delta = 0, gamma = 0;
        bnorm = (b, b);
        r = b - A * x;
        u = B * r;
        w = A * u;

        for (this->iter = 0; this->iter < this->maxiter; this->iter++) {
            gammaold = gamma;
            rnorm = (r, r);
            gamma = (r, u);
            delta = (w, u);
            m = B * w;
            n = A * m;

            if (this->Converged(std::sqrt(rnorm), std::sqrt(bnorm)))
                break;

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
