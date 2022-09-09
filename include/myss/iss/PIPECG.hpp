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
class PIPECG : public ISSAbstract<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
    using AsyncIntermediate = AsyncProxy<intermediate_t>;
    using SyncIntermediate = SyncProxy<intermediate_t>;
    using pipe_intermediate_t = typename std::conditional<enable_pipeline, AsyncIntermediate, SyncIntermediate>::type;

public:
    using BASE = ISSAbstract<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    PIPECG() = delete;
    PIPECG(const AType &A) : BASE(A) { }
    PIPECG(const AType &A, const BType &B) : BASE(A, B) { }

    virtual const char *GetName() const {
        return "PIPECG";
    }

    virtual void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const AType &A = this->GetMatrix();
        const BType &B = this->GetPreconditioner();
        LValue<VType> f(b), v(x), r(x), z(x), p(x), n(x), w(x), q(x), u(x), m(x), s(x);

        intermediate_t bnorm = 0, alpha = 1, beta = 1, gammaold = 0;
        pipe_intermediate_t rnorm = 0, delta = 0, gamma = 0;
        bnorm = (f, f);
        r = f - A * v;
        u = B * r;
        w = A * u;

        do {
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
            v += alpha * p;
            u -= alpha * q;
            w -= alpha * z;
            r -= alpha * s;
        } while (++this->iter);

        x = v;
    }

};
