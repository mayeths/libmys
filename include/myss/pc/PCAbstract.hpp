#pragma once

template<typename matrix_t, typename vector_t, typename index_t, typename data_t>
class PCAbstract
{
public:
    using VType = vector_t;
    using AType = matrix_t; /*MAbstract<index_t, data_t>;*/
    AType *A = nullptr;

    PCAbstract() : A(nullptr) { }
    PCAbstract(AType &A) { this->A = &A; }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    friend VType operator*(const PCAbstract &pc, const VType &b) {
        VType x(b);
        pc.Apply(b, x);
        return x;
    }
};
