#pragma once

#include "PCAbstract.hpp"

template<typename matrix_t, typename vector_t, typename index_t = int, typename data_t = double>
class PCNone : public PCAbstract<matrix_t, vector_t, index_t, data_t>
{
public:
    using BASE = PCAbstract<matrix_t, vector_t, index_t, data_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;

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

