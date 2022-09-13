#pragma once

template<typename matrix_t>
class PCAbstract
{
public:
    using MType = matrix_t;
    using VType = typename MType::VType;
    using IType = typename MType::IType;
    using DType = typename MType::DType;
    MType *A = nullptr;

    PCAbstract() : A(nullptr) { }
    PCAbstract(MType &A) { this->A = &A; }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    friend VType operator*(const PCAbstract &pc, const VType &b) {
        VType x(b);
        pc.Apply(b, x);
        return x;
    }
};
