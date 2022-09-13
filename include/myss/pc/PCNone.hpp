#pragma once

#include "PCBase.hpp"

template<typename matrix_t>
class PCNone : public PCBase<matrix_t>
{
public:
    using BASE = PCBase<matrix_t>;
    using MType = typename BASE::MType;
    using VType = typename BASE::VType;

    PCNone() : BASE() { }
    ~PCNone() {
    }

    static void copy(const PCNone &src, PCNone &dst) { }
    static void swap(PCNone &src, PCNone &dst) { }
    PCNone(const PCNone &src)     { PCNone::copy(src, *this); } /* Copy Ctor. */
    PCNone(PCNone&& src) noexcept { PCNone::swap(src, *this); } /* Move Ctor. */
    PCNone& operator=(const PCNone &src)     { PCNone::copy(src, *this); return *this; } /* Copy Assign. */
    PCNone& operator=(PCNone&& src) noexcept { PCNone::swap(src, *this); return *this; } /* Move Assign. */

    virtual void Apply(const VType &b, VType &x, bool xzero = false) const {
        x = b;
    }
    virtual const char *GetName() const {
        return "PCNone";
    }

};

