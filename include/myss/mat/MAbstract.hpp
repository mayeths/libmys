#pragma once

template<typename vt>
class MAbstract
{
public:
    using VType = vt;
    using index_t = typename VType::index_t;
    using data_t = typename VType::data_t;
    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    friend VType operator*(const MAbstract &mat, const VType &x) {
        VType Ax(x);
        mat.Apply(x, Ax);
        return Ax;
    }
};
