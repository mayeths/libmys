#pragma once

#include "PCBase.hpp"

#if 1

template<typename matrix_t>
class PCJacobi : public PCBase<matrix_t>
{
public:
    using BASE = PCBase<matrix_t>;
    using MType = typename BASE::MType;
    using VType = typename BASE::VType;
    using IType = typename BASE::IType;
    using DType = typename BASE::DType;

    VType diags;

    PCJacobi() = delete;
    ~PCJacobi() {
    }

    static void copy(const PCJacobi &src, PCJacobi &dst) { }
    static void swap(PCJacobi &src, PCJacobi &dst) { }
    PCJacobi(const PCJacobi &src)     { PCJacobi::copy(src, *this); }
    PCJacobi(PCJacobi&& src) noexcept { PCJacobi::swap(src, *this); }
    PCJacobi& operator=(const PCJacobi &src)     { PCJacobi::copy(src, *this); return *this; }
    PCJacobi& operator=(PCJacobi&& src) noexcept { PCJacobi::swap(src, *this); return *this; }

    PCJacobi(MType &A) : BASE(A) {
        this->diags = 1 / A.GetDiagonals();
    }

    virtual void Apply(const VType &b, VType &x, bool xzero = false) const {
        x = b * this->diags;
    }
    virtual const char *GetName() const {
        return "PCJacobi";
    }

};

#endif
