#pragma once

#include "../vec/VAbstract.hpp"

template <typename vt, typename it, typename dt>
class MAbstract
{
public:
    using index_t = it;
    using data_t = dt;
    using RealVType = vt;
    using VType = VAbstract<RealVType, index_t, data_t>;
    virtual void Apply(const RealVType &input, RealVType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    friend RealVType operator*(const MAbstract &mat, const RealVType &x) {
        RealVType Ax(x);
        mat.Apply(x, Ax);
        return Ax;
    }
};
