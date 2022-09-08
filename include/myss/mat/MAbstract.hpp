#pragma once

template<typename VType, typename index_t, typename data_t>
class MAbstract
{
public:
    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;

    friend VType operator*(const MAbstract &mat, const VType &x) {
        VType Ax(x);
        mat.Apply(x, Ax);
        return Ax;
    }
};
