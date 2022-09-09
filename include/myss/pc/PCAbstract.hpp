#pragma once

template<typename mt>
class PCAbstract
{
public:
    using AType = mt;
    using VType = typename AType::VType;
    using index_t = typename AType::index_t;
    using data_t = typename AType::data_t;
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
