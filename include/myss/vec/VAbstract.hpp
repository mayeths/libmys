#pragma once

#include "../util/TypeTraits.hpp"
#include "../util/AsyncProxy.hpp"

template<typename vt, typename it, typename dt>
class VAbstract
{
public:
    using vector_t = vt;
    using index_t = it;
    using data_t = dt;
    explicit VAbstract() { }
    explicit VAbstract(const VAbstract&) { } /* Copy */
    explicit VAbstract(VAbstract&&) { } /* Move */

    vector_t& operator+=(const vector_t &x) {
        vector_t &self = static_cast<vector_t&>(*this);
        vector_t::axpy(self, 1, x);
        return self;
    }
    friend vector_t operator+(vector_t x, const vector_t& y) {
        return x += y;
    }

    vector_t& operator-=(const vector_t &x) {
        vector_t &self = static_cast<vector_t&>(*this);
        vector_t::axpy(self, -1, x);
        return self;
    }
    friend vector_t operator-(vector_t x, const vector_t& y) {
        return x -= y;
    }

    vector_t& operator*=(PetscScalar alpha) {
        vector_t &self = static_cast<vector_t&>(*this);
        vector_t::scale(self, alpha);
        return self;
    }
    friend vector_t operator*(vector_t x, PetscScalar alpha) {
        return x *= alpha;
    }
    friend vector_t operator*(PetscScalar alpha, vector_t x) {
        return x *= alpha;
    }

    friend AsyncProxy<data_t> operator,(const vector_t &x, const vector_t& y) {
        return vector_t::async_dot(x, y);
    }

    friend data_t dot(const vector_t &x, const vector_t &y) {
        return vector_t::async_dot(x, y).await();
    }

};


enum class BlasType: int {
    NONE   = 0,
    SCALE  = 10, /* r = alpha q + beta * p (where r = p, alpha = 0.0) | p = beta * p           */
    AXPY   = 20, /* r = alpha q + beta * p (where r = p, beta = 1.0)  | p = alpha q + p        */
    BYPX   = 21, /* r = alpha q + beta * p (where r = p, alpha = 1.0) | p = q + beta p         */
    AXPBY  = 22, /* r = alpha q + beta * p (where r = p)              | p = alpha q + beta * p */
    WAXPBY = 30, /* r = alpha q + beta * p */
};


template<typename VType>
class LValue;
template<typename VType>
class RValue;

template<typename VType>
class RValue
{
    using index_t = typename VType::index_t;
    using data_t = typename VType::data_t;
    mutable const VType *q = nullptr;
    mutable const VType *p = nullptr;
    // mutable const VType *r = nullptr;
    mutable BlasType stage = BlasType::NONE;
public:

    RValue(const VType &v) {
        this->v = &v;
    }

    operator VType() const {
        return this->v;
    }

    friend RValue<VType> operator+(const RValue<VType> &q, const RValue<VType>& p) {
        RValue<VType> result;
        return q += p;
    }
};

template<typename VType>
class LValue : public VType
{
    using index_t = typename VType::index_t;
    using data_t = typename VType::data_t;
public:
    LValue() : VType() { };
    LValue(const VType &src) : VType(src) { } /* Copy Ctor. */
    LValue(VType&& src) noexcept : VType(src) { } /* Move Ctor. */
    LValue<VType>& operator=(const VType &src) { VType::operator=(src); return *this; }
    LValue<VType>& operator=(VType&& src) noexcept { VType::operator=(src); return *this; }

    friend LValue<VType> operator+(LValue<VType> x, const LValue<VType>& y) { return x += y; }
    friend LValue<VType> operator-(LValue<VType> x, const LValue<VType>& y) { return x -= y; }
    friend LValue<VType> operator*(LValue<VType> x, data_t alpha) { return x *= alpha; }
    friend LValue<VType> operator*(data_t alpha, LValue<VType> x) { return x * alpha; }

    friend AsyncProxy<typename LValue<VType>::data_t> operator,(const LValue<VType> &x, const LValue<VType>& y) {
    return LValue<VType>::async_dot(x, y);
}

};

