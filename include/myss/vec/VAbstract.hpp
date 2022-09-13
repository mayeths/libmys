#pragma once

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
    using VType = vt;
    using index_t = it;
    using data_t = dt;
    explicit VAbstract() { }
    virtual bool iscompact(const VAbstract<vt, it, dt> &other) { return true; }

    // explicit VAbstract(const VAbstract&) { } /* Copy */
    // explicit VAbstract(VAbstract&&) { } /* Move */
    VAbstract(const VType &src)     { VType::copy(src, static_cast<VType&>(*this)); } /* Copy Ctor. */
    VAbstract(VType&& src) noexcept { VType::swap(src, static_cast<VType&>(*this)); } /* Move Ctor. */
    VType& operator=(const AX<VType> &src)     { return static_cast<VType&>(*this).operator=(src.eval()); } /* Copy Assign. */
    VType& operator=(AX<VType>&& src) noexcept { return static_cast<VType&>(*this).operator=(src.eval()); } /* Move Assign. */
    VType& operator=(const AXPBY<VType> &src)     { return static_cast<VType&>(*this).operator=(src.eval()); } /* Copy Assign. */
    VType& operator=(AXPBY<VType>&& src) noexcept { return static_cast<VType&>(*this).operator=(src.eval()); } /* Move Assign. */

    VType& operator+=(const VType &x) {
        VType &self = static_cast<VType&>(*this);
        self = self + x;
        return self;
    }
    VType& operator+=(const AX<VType> &x) {
        VType &self = static_cast<VType&>(*this);
        self = self + x;
        return self;
    }
    VType& operator+=(const AXPBY<VType> &x) {
        VType &self = static_cast<VType&>(*this);
        self = self + x;
        return self;
    }
    // friend VType operator+(VType x, const VAbstract<vt, it, dt>& y) {
    //     return AXPBY<VType>(static_cast<data_t>(1), x, static_cast<data_t>(1), y);
    // }
    // friend VType operator+(VAbstract<vt, it, dt> x, const VType& y) {
    //     return AXPBY<VType>(static_cast<data_t>(1), x, static_cast<data_t>(1), y);
    // }
    friend VType operator+(VType x, const VType& y) {
        return AXPBY<VType>(static_cast<data_t>(1), x, static_cast<data_t>(1), y);
    }

    VType& operator-=(const VType &x) {
        VType &self = static_cast<VType&>(*this);
        self = self - x;
        return self;
    }
    VType& operator-=(const AX<VType> &x) {
        VType &self = static_cast<VType&>(*this);
        self = self - x;
        return self;
    }
    VType& operator-=(const AXPBY<VType> &x) {
        VType &self = static_cast<VType&>(*this);
        self = self - x;
        return self;
    }
    // friend VType operator-(VType x, const VAbstract<vt, it, dt>& y) {
    //     return AXPBY<VType>(static_cast<data_t>(1), x, static_cast<data_t>(-1), y);
    // }
    // friend VType operator-(VAbstract<vt, it, dt> x, const VType& y) {
    //     return AXPBY<VType>(static_cast<data_t>(1), x, static_cast<data_t>(-1), y);
    // }
    friend VType operator-(VType x, const VType& y) {
        return AXPBY<VType>(static_cast<data_t>(1), x, static_cast<data_t>(-1), y);
    }

    // friend AX<VType> operator*(data_t alpha, const VAbstract<vt, it, dt> &x) {
    //     return AX<VType>(alpha, x);
    // }
    // friend AX<VType> operator*(const VAbstract<vt, it, dt> &x, data_t alpha) {
    //     return AX<VType>(alpha, x);
    // }
    friend AX<VType> operator*(const VType &x, data_t alpha) {
        return AX<VType>(alpha, x);
    }
    friend AX<VType> operator*(data_t alpha, VType x) {
        return AX<VType>(alpha, x);
    }

    // friend AsyncProxy<data_t> operator,(const VAbstract<vt, it, dt> &x, const VAbstract<vt, it, dt>& y) {
    //     return VType::AsyncDot(x, y);
    // }
    // friend AsyncProxy<data_t> operator,(const VAbstract<vt, it, dt> &x, const VType& y) {
    //     return VType::AsyncDot(x, y);
    // }
    // friend AsyncProxy<data_t> operator,(const VType &x, const VAbstract<vt, it, dt>& y) {
    //     return VType::AsyncDot(x, y);
    // }
    friend AsyncProxy<data_t> operator,(const VType &x, const VType& y) {
        return VType::AsyncDot(x, y);
    }
    friend data_t dot(const VType &x, const VType &y) {
        return (x, y).await();
    }

};


template<typename vt>
class AX
{
    using VType = vt;
    using index_t = typename VType::index_t;
    using data_t = typename VType::data_t;
    const data_t alpha = static_cast<data_t>(1);
    const VType *x = nullptr;
public:

    AX(const data_t &alpha, const VType &x) : alpha(alpha), x(&x) { }
    VType eval() const {
        VType w;
        VType::duplicate(*this->x, w);
        VType::AXPBY(w, this->alpha, *this->x, static_cast<data_t>(0), *this->x);
        return w;
    }
    friend AX<VType> operator*(const AX<VType> &lhs, const data_t &scale) {
        return AX<VType>(lhs.x, lhs.alpha * scale);
    }
    friend AX<VType> operator*(const AX<VType> &lhs, const AsyncProxy<data_t> &ascale) {
        data_t scale = ascale.await();
        return lhs * scale;
    }
    friend AX<VType> operator*(const data_t &scale, const AX<VType> &rhs) {
        return rhs * scale;
    }
    friend AX<VType> operator*(const AsyncProxy<data_t> &ascale, const AX<VType> &rhs) {
        return rhs * ascale;
    }

    // friend AXPBY<VType> operator+(const AX<VType> &lhs, const VAbstract<VType, index_t, data_t> &rhs) {
    //     return AXPBY<VType>(lhs.alpha, *lhs.x, static_cast<data_t>(1), rhs);
    // }
    friend AXPBY<VType> operator+(const VType &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(static_cast<data_t>(1), lhs, rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator+(const AX<VType> &lhs, const VType &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, static_cast<data_t>(1), rhs);
    }
    friend AXPBY<VType> operator+(const AX<VType> &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator+(const AX<VType> &lhs, const AXPBY<VType> &rhs) {
        return lhs + rhs.eval();
    }

    // friend AXPBY<VType> operator-(const AX<VType> &lhs, const VAbstract<VType, index_t, data_t> &rhs) {
    //     return AXPBY<VType>(lhs.alpha, *lhs.x, static_cast<data_t>(-1), rhs);
    // }
    friend AXPBY<VType> operator-(const AX<VType> &lhs, const VType &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, static_cast<data_t>(-1), rhs);
    }
    friend AXPBY<VType> operator-(const VType &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(static_cast<data_t>(1), lhs, -rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator-(const AX<VType> &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator-(const AX<VType> &lhs, const AXPBY<VType> &rhs) {
        return lhs - rhs.eval();
    }
};


template<typename vt>
class AXPBY
{
    using VType = vt;
    using index_t = typename VType::index_t;
    using data_t = typename VType::data_t;
    const data_t alpha = static_cast<data_t>(1);
    const data_t beta = static_cast<data_t>(1);
    const VType *x = nullptr;
    const VType *y = nullptr;
public:

    AXPBY(const data_t &alpha, const VType &x, const data_t &beta, const VType &y) : alpha(alpha), x(&x), beta(beta), y(&y) { }
    VType eval() const {
        VType w;
        VType::duplicate(*this->x, w);
        VType::AXPBY(w, this->alpha, *this->x, this->beta, *this->y);
        return w;
    }

    operator VType() const {
        return this->eval();
    }

    friend AXPBY<VType> operator*(const AXPBY<VType> &lhs, const data_t &scale) {
        return AXPBY<VType>(lhs.alpha * scale, lhs.x, lhs.beta * scale, lhs.y);
    }
    friend AXPBY<VType> operator*(const AXPBY<VType> &lhs, const AsyncProxy<data_t> &ascale) {
        data_t scale = ascale.await();
        return lhs * scale;
    }
    friend AXPBY<VType> operator*(const data_t &scale, const AXPBY<VType> &rhs) {
        return rhs * scale;
    }
    friend AXPBY<VType> operator*(const AsyncProxy<data_t> &ascale, const AXPBY<VType> &rhs) {
        return rhs * ascale;
    }

    friend AXPBY<VType> operator+(const AXPBY<VType> &lhs, const VType &rhs) {
        return lhs.eval() + rhs;
    }
    friend AXPBY<VType> operator+(const AXPBY<VType> &lhs, const AX<VType> &rhs) {
        return lhs.eval() + rhs;
    }
    friend AXPBY<VType> operator+(const AXPBY<VType> &lhs, const AXPBY<VType> &rhs) {
        return lhs.eval() + rhs.eval();
    }

    friend AXPBY<VType> operator-(const AXPBY<VType> &lhs, const VType &rhs) {
        return lhs.eval() - rhs;
    }
    friend AXPBY<VType> operator-(const AXPBY<VType> &lhs, const AX<VType> &rhs) {
        return lhs.eval() - rhs;
    }
    friend AXPBY<VType> operator-(const AXPBY<VType> &lhs, const AXPBY<VType> &rhs) {
        return lhs.eval() - rhs.eval();
    }

};




