#pragma once

#include "ISSAbstract.hpp"

template<
    typename matrix_t,
    bool enable_pipeline = true,
    typename intermediate_t = typename matrix_t::DType> /* intermediate DType */
class PIPECG : public ISSAbstract<matrix_t>
{
public:
    using BASE = ISSAbstract<matrix_t>;
    using IType = typename BASE::IType;
    using DType = typename BASE::DType;
    using VType = typename BASE::VType;
    using MType = typename BASE::MType;
    using PType = typename BASE::PType;
    using IntermediateType = intermediate_t;
    using PipeIntermediateType = typename std::conditional<
        enable_pipeline,
        AsyncProxy<IntermediateType>,
        SyncProxy<IntermediateType>
    >::type;

    PIPECG() = delete;
    PIPECG(const MType &A) : BASE(A) { }
    PIPECG(const MType &A, const PType &B) : BASE(A, B) { }

    virtual const char *GetName() const {
        return "PIPECG";
    }

    virtual void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        const MType &A = this->GetMatrix();
        const PType &B = this->GetPreconditioner();
        VType r(x), z(x), p(x), n(x), w(x), q(x), u(x), m(x), s(x);

        IntermediateType bnorm = 0, alpha = 1, beta = 1, gammaold = 0;
        PipeIntermediateType rnorm = 0, delta = 0, gamma = 0;
        bnorm = (b, b);
        r = b - A * x;
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
            x += alpha * p;
            u -= alpha * q;
            w -= alpha * z;
            r -= alpha * s;
        } while (++this->iter);
    }

};
