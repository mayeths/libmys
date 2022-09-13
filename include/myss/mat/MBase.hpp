#pragma once

#include "../vec/VBase.hpp"

template <typename vector_t, typename index_t, typename data_t>
class MBase
{
public:
    using IType = index_t;
    using DType = data_t;
    using VType = vector_t;
    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    friend VType operator*(const MBase &mat, const VType &x) {
        VType Ax(x);
        mat.Apply(x, Ax);
        return Ax;
    }
};
