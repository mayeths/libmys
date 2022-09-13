#pragma once

template<typename matrix_t>
class PCBase
{
public:
    using MType = matrix_t;
    using VType = typename MType::VType;
    using IType = typename MType::IType;
    using DType = typename MType::DType;
    MType *A = nullptr;

    PCBase() : A(nullptr) { }
    PCBase(MType &A) { this->A = &A; }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;
    virtual const char *GetName() const = 0;

    friend VType operator*(const PCBase &pc, const VType &b) {
        VType x(b);
        pc.Apply(b, x);
        return x;
    }
};
