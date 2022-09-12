#pragma once

#include "../util/TypeTraits.hpp"
#include "../util/AsyncProxy.hpp"

template<typename vt> class AX;
template<typename vt> class AXPBY;

enum class ValueSetOp : int {
    Insert,
    Shift,
    Scale,
};

template<typename vt, typename it, typename dt>
class VAbstract
{
public:
    using vector_t = vt;
    using index_t = it;
    using data_t = dt;
    explicit VAbstract() { }
    virtual bool iscompact(const VAbstract<vt, it, dt> &other) { return true; }

    // explicit VAbstract(const VAbstract&) { } /* Copy */
    // explicit VAbstract(VAbstract&&) { } /* Move */
    VAbstract(const vector_t &src)     { vector_t::copy(src, static_cast<vector_t&>(*this)); } /* Copy Ctor. */
    VAbstract(vector_t&& src) noexcept { vector_t::swap(src, static_cast<vector_t&>(*this)); } /* Move Ctor. */
    vector_t& operator=(const AX<vector_t> &src)     { return static_cast<vector_t&>(*this).operator=(src.eval()); } /* Copy Assign. */
    vector_t& operator=(AX<vector_t>&& src) noexcept { return static_cast<vector_t&>(*this).operator=(src.eval()); } /* Move Assign. */
    vector_t& operator=(const AXPBY<vector_t> &src)     { return static_cast<vector_t&>(*this).operator=(src.eval()); } /* Copy Assign. */
    vector_t& operator=(AXPBY<vector_t>&& src) noexcept { return static_cast<vector_t&>(*this).operator=(src.eval()); } /* Move Assign. */

    vector_t& operator+=(const vector_t &x) {
        vector_t &self = static_cast<vector_t&>(*this);
        self = self + x;
        return self;
    }
    vector_t& operator+=(const AX<vector_t> &x) {
        vector_t &self = static_cast<vector_t&>(*this);
        self = self + x;
        return self;
    }
    friend vector_t operator+(vector_t x, const VAbstract<vt, it, dt>& y) {
        return AXPBY<vector_t>(static_cast<data_t>(1), x, static_cast<data_t>(1), y);
    }
    friend vector_t operator+(VAbstract<vt, it, dt> x, const vector_t& y) {
        return AXPBY<vector_t>(static_cast<data_t>(1), x, static_cast<data_t>(1), y);
    }
    friend vector_t operator+(vector_t x, const vector_t& y) {
        return AXPBY<vector_t>(static_cast<data_t>(1), x, static_cast<data_t>(1), y);
    }

    vector_t& operator-=(const vector_t &x) {
        vector_t &self = static_cast<vector_t&>(*this);
        self = self - x;
        return self;
    }
    vector_t& operator-=(const AX<vector_t> &x) {
        vector_t &self = static_cast<vector_t&>(*this);
        self = self - x;
        return self;
    }
    friend vector_t operator-(vector_t x, const VAbstract<vt, it, dt>& y) {
        return AXPBY<vector_t>(static_cast<data_t>(1), x, static_cast<data_t>(-1), y);
    }
    friend vector_t operator-(VAbstract<vt, it, dt> x, const vector_t& y) {
        return AXPBY<vector_t>(static_cast<data_t>(1), x, static_cast<data_t>(-1), y);
    }
    friend vector_t operator-(vector_t x, const vector_t& y) {
        return AXPBY<vector_t>(static_cast<data_t>(1), x, static_cast<data_t>(-1), y);
    }

    friend AX<vector_t> operator*(data_t alpha, const VAbstract<vt, it, dt> &x) {
        return AX<vector_t>(alpha, x);
    }
    friend AX<vector_t> operator*(const VAbstract<vt, it, dt> &x, data_t alpha) {
        return AX<vector_t>(alpha, x);
    }
    friend AX<vector_t> operator*(const vector_t &x, data_t alpha) {
        return AX<vector_t>(alpha, x);
    }
    friend AX<vector_t> operator*(data_t alpha, vector_t x) {
        return AX<vector_t>(alpha, x);
    }

    friend AsyncProxy<data_t> operator,(const VAbstract<vt, it, dt> &x, const VAbstract<vt, it, dt>& y) {
        return vector_t::AsyncDot(x, y);
    }
    friend AsyncProxy<data_t> operator,(const VAbstract<vt, it, dt> &x, const vector_t& y) {
        return vector_t::AsyncDot(x, y);
    }
    friend AsyncProxy<data_t> operator,(const vector_t &x, const VAbstract<vt, it, dt>& y) {
        return vector_t::AsyncDot(x, y);
    }
    friend AsyncProxy<data_t> operator,(const vector_t &x, const vector_t& y) {
        return vector_t::AsyncDot(x, y);
    }
    friend data_t dot(const vector_t &x, const vector_t &y) {
        return (x, y).await();
    }

};


template<typename vt>
class AX
{
    using vector_t = vt;
    using index_t = typename vector_t::index_t;
    using data_t = typename vector_t::data_t;
    const data_t alpha = static_cast<data_t>(1);
    const vector_t *x = nullptr;
public:

    AX(const data_t &alpha, const vector_t &x) : alpha(alpha), x(&x) { }
    vector_t eval() const {
        vector_t w;
        vector_t::duplicate(*this->x, w);
        vector_t::AXPBY(w, this->alpha, *this->x, static_cast<data_t>(0), *this->x);
        return w;
    }
    friend AX<vector_t> operator*(const AX<vector_t> &lhs, const data_t &scale) {
        return AX<vector_t>(lhs.x, lhs.alpha * scale);
    }
    friend AX<vector_t> operator*(const AX<vector_t> &lhs, const AsyncProxy<data_t> &ascale) {
        data_t scale = ascale.await();
        return lhs * scale;
    }
    friend AX<vector_t> operator*(const data_t &scale, const AX<vector_t> &rhs) {
        return rhs * scale;
    }
    friend AX<vector_t> operator*(const AsyncProxy<data_t> &ascale, const AX<vector_t> &rhs) {
        return rhs * ascale;
    }

    friend AXPBY<vector_t> operator+(const AX<vector_t> &lhs, const VAbstract<vector_t, index_t, data_t> &rhs) {
        return AXPBY<vector_t>(lhs.alpha, *lhs.x, static_cast<data_t>(1), rhs);
    }
    friend AXPBY<vector_t> operator+(const vector_t &lhs, const AX<vector_t> &rhs) {
        return AXPBY<vector_t>(static_cast<data_t>(1), lhs, rhs.alpha, *rhs.x);
    }
    friend AXPBY<vector_t> operator+(const AX<vector_t> &lhs, const vector_t &rhs) {
        return AXPBY<vector_t>(lhs.alpha, *lhs.x, static_cast<data_t>(1), rhs);
    }
    friend AXPBY<vector_t> operator+(const AX<vector_t> &lhs, const AX<vector_t> &rhs) {
        return AXPBY<vector_t>(lhs.alpha, *lhs.x, rhs.alpha, *rhs.x);
    }
    friend AXPBY<vector_t> operator+(const AX<vector_t> &lhs, const AXPBY<vector_t> &rhs) {
        return lhs + rhs.eval();
    }

    friend AXPBY<vector_t> operator-(const AX<vector_t> &lhs, const VAbstract<vector_t, index_t, data_t> &rhs) {
        return AXPBY<vector_t>(lhs.alpha, *lhs.x, static_cast<data_t>(-1), rhs);
    }
    friend AXPBY<vector_t> operator-(const AX<vector_t> &lhs, const vector_t &rhs) {
        return AXPBY<vector_t>(lhs.alpha, *lhs.x, static_cast<data_t>(-1), rhs);
    }
    friend AXPBY<vector_t> operator-(const vector_t &lhs, const AX<vector_t> &rhs) {
        return AXPBY<vector_t>(static_cast<data_t>(1), lhs, -rhs.alpha, *rhs.x);
    }
    friend AXPBY<vector_t> operator-(const AX<vector_t> &lhs, const AX<vector_t> &rhs) {
        return AXPBY<vector_t>(lhs.alpha, *lhs.x, rhs.alpha, *rhs.x);
    }
    friend AXPBY<vector_t> operator-(const AX<vector_t> &lhs, const AXPBY<vector_t> &rhs) {
        return lhs - rhs.eval();
    }
};


template<typename vt>
class AXPBY
{
    using vector_t = vt;
    using index_t = typename vector_t::index_t;
    using data_t = typename vector_t::data_t;
    const data_t alpha = static_cast<data_t>(1);
    const data_t beta = static_cast<data_t>(1);
    const vector_t *x = nullptr;
    const vector_t *y = nullptr;
public:

    AXPBY(const data_t &alpha, const VAbstract<vector_t, index_t, data_t> &x, const data_t &beta, const VAbstract<vector_t, index_t, data_t> &y) : alpha(alpha), x(static_cast<const vector_t*>(&x)), beta(beta), y(static_cast<const vector_t*>(&y)) { }
    vector_t eval() const {
        vector_t w;
        vector_t::duplicate(*this->x, w);
        vector_t::AXPBY(w, this->alpha, *this->x, this->beta, *this->y);
        return w;
    }

    operator vector_t() const {
        return this->eval();
    }

    friend AXPBY<vector_t> operator*(const AXPBY<vector_t> &lhs, const data_t &scale) {
        return AXPBY<vector_t>(lhs.alpha * scale, lhs.x, lhs.beta * scale, lhs.y);
    }
    friend AXPBY<vector_t> operator*(const AXPBY<vector_t> &lhs, const AsyncProxy<data_t> &ascale) {
        data_t scale = ascale.await();
        return lhs * scale;
    }
    friend AXPBY<vector_t> operator*(const data_t &scale, const AXPBY<vector_t> &rhs) {
        return rhs * scale;
    }
    friend AXPBY<vector_t> operator*(const AsyncProxy<data_t> &ascale, const AXPBY<vector_t> &rhs) {
        return rhs * ascale;
    }

    friend AXPBY<vector_t> operator+(const AXPBY<vector_t> &lhs, const vector_t &rhs) {
        return lhs.eval() + rhs;
    }
    friend AXPBY<vector_t> operator+(const AXPBY<vector_t> &lhs, const AX<vector_t> &rhs) {
        return lhs.eval() + rhs;
    }
    friend AXPBY<vector_t> operator+(const AXPBY<vector_t> &lhs, const AXPBY<vector_t> &rhs) {
        return lhs.eval() + rhs.eval();
    }

    friend AXPBY<vector_t> operator-(const AXPBY<vector_t> &lhs, const vector_t &rhs) {
        return lhs.eval() - rhs;
    }
    friend AXPBY<vector_t> operator-(const AXPBY<vector_t> &lhs, const AX<vector_t> &rhs) {
        return lhs.eval() - rhs;
    }
    friend AXPBY<vector_t> operator-(const AXPBY<vector_t> &lhs, const AXPBY<vector_t> &rhs) {
        return lhs.eval() - rhs.eval();
    }

};




