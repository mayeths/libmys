#pragma once

#include "../vec/VAbstract.hpp"

template <typename vt, typename it, typename dt>
class MAbstract
{
public:
    using index_t = it;
    using data_t = dt;
    using VType = vt;
    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    friend VType operator*(const MAbstract &mat, const VType &x) {
        VType Ax(x);
        mat.Apply(x, Ax);
        return Ax;
    }
};
