#pragma once

#include "PCAbstract.hpp"

template<typename mt>
class PCNone : public PCAbstract<mt>
{
public:
    using AType = mt;
    using BASE = PCAbstract<mt>;
    using VType = typename BASE::VType;
    using RealVType = typename AType::RealVType;

    PCNone() : BASE() { }
    ~PCNone() {
    }

    static void copy(const PCNone &src, PCNone &dst) { }
    static void swap(PCNone &src, PCNone &dst) { }
    PCNone(const PCNone &src)     { PCNone::copy(src, *this); } /* Copy Ctor. */
    PCNone(PCNone&& src) noexcept { PCNone::swap(src, *this); } /* Move Ctor. */
    PCNone& operator=(const PCNone &src)     { PCNone::copy(src, *this); return *this; } /* Copy Assign. */
    PCNone& operator=(PCNone&& src) noexcept { PCNone::swap(src, *this); return *this; } /* Move Assign. */

    virtual void Apply(const RealVType &b, RealVType &x, bool xzero = false) const {
        x = b;
    }
    virtual const char *GetName() const {
        return "PCNone";
    }

};

