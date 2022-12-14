#pragma once

#include "../util/AsyncProxy.hpp"

template<typename vector_t> class AX;
template<typename vector_t> class AXPBY;

enum class ElementOp : int {
    Replace,    /* y[i] = x[i]        or y[i] = alpha        */
    Shift,      /* y[i] = x[i] + y[i] or y[i] = alpha + y[i] */
    Scale,      /* y[i] = x[i] * y[i] or y[i] = alpha * y[i] */
    Reciprocal, /* y[i] = x[i] / y[i] or y[i] = alpha / y[i] */
};

template<typename vector_t, typename index_t, typename data_t>
class VBase
{
public:
    using VType = vector_t;
    using IType = index_t;
    using DType = data_t;
    VBase() { }

    VBase(const VType &src)     { VType::copy(src, static_cast<VType&>(*this)); }
    VBase(VType&& src) noexcept { VType::swap(src, static_cast<VType&>(*this)); }
    VType& operator=(const AX<VType> &src)     { return static_cast<VType&>(*this).operator=(src.eval()); }
    VType& operator=(const AXPBY<VType> &src)  { return static_cast<VType&>(*this).operator=(src.eval()); }
    VType& operator=(AX<VType>&& src) noexcept    { return static_cast<VType&>(*this).operator=(src.eval()); }
    VType& operator=(AXPBY<VType>&& src) noexcept { return static_cast<VType&>(*this).operator=(src.eval()); }
    VType& operator=(DType alpha) noexcept { VType::ElementWiseOp(static_cast<VType&>(*this), alpha, ElementOp::Replace); return static_cast<VType&>(*this); }

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
    friend VType operator+(const VType &x, const VType& y) {
        return AXPBY<VType>(static_cast<DType>(1), x, static_cast<DType>(1), y);
    }
    friend VType operator+(const VType &x, DType alpha) {
        VType y = x;
        VType::ElementWiseOp(y, alpha, ElementOp::Shift);
        return y;
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
    friend VType operator-(VType x, const VType& y) {
        return AXPBY<VType>(static_cast<DType>(1), x, static_cast<DType>(-1), y);
    }

    friend AX<VType> operator*(const VType &x, DType alpha) {
        return AX<VType>(alpha, x);
    }
    friend AX<VType> operator*(DType alpha, const VType &x) {
        return AX<VType>(alpha, x);
    }

    friend VType operator*(const VType &x, const VType &y) {
        VType w = x;
        VType::ElementWiseOp(w, y, ElementOp::Scale);
        return w;
    }
    friend VType operator/(const VType &x, const VType &y) {
        VType w = x;
        VType::ElementWiseOp(w, y, ElementOp::Reciprocal);
        return w;
    }
    friend AX<VType> operator/(const VType &x, DType alpha) {
        return AX<VType>(static_cast<DType>(-1) / alpha, x);
    }
    friend VType operator/(DType alpha, const VType &x) {
        VType y = x;
        VType::ElementWiseOp(y, alpha, ElementOp::Reciprocal);
        return y;
    }

    friend AsyncProxy<DType> operator,(const VType &x, const VType& y) {
        return VType::AsyncDot(x, y);
    }
    friend DType dot(const VType &x, const VType &y) {
        return (x, y).await();
    }

};


template<typename vector_t>
class AX
{
    using VType = vector_t;
    using IType = typename VType::IType;
    using DType = typename VType::DType;
    const DType alpha = static_cast<DType>(1);
    const VType *x = nullptr;
public:

    AX(const DType &alpha, const VType &x) : alpha(alpha), x(&x) { }

    VType eval() const {
        VType w = *this->x;
        VType::ElementWiseOp(w, this->alpha, ElementOp::Scale);
        return w;
    }

    operator VType() const {
        return this->eval();
    }

    friend AX<VType> operator*(const AX<VType> &lhs, const DType &scale) {
        return AX<VType>(lhs.x, lhs.alpha * scale);
    }
    friend AX<VType> operator*(const AX<VType> &lhs, const AsyncProxy<DType> &ascale) {
        DType scale = ascale.await();
        return lhs * scale;
    }
    friend AX<VType> operator*(const DType &scale, const AX<VType> &rhs) {
        return rhs * scale;
    }
    friend AX<VType> operator*(const AsyncProxy<DType> &ascale, const AX<VType> &rhs) {
        return rhs * ascale;
    }

    friend AXPBY<VType> operator+(const VType &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(static_cast<DType>(1), lhs, rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator+(const AX<VType> &lhs, const VType &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, static_cast<DType>(1), rhs);
    }
    friend AXPBY<VType> operator+(const AX<VType> &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator+(const AX<VType> &lhs, const AXPBY<VType> &rhs) {
        return lhs + rhs.eval();
    }

    friend AXPBY<VType> operator-(const AX<VType> &lhs, const VType &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, static_cast<DType>(-1), rhs);
    }
    friend AXPBY<VType> operator-(const VType &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(static_cast<DType>(1), lhs, -rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator-(const AX<VType> &lhs, const AX<VType> &rhs) {
        return AXPBY<VType>(lhs.alpha, *lhs.x, rhs.alpha, *rhs.x);
    }
    friend AXPBY<VType> operator-(const AX<VType> &lhs, const AXPBY<VType> &rhs) {
        return lhs - rhs.eval();
    }
};


template<typename vector_t>
class AXPBY
{
    using VType = vector_t;
    using IType = typename VType::IType;
    using DType = typename VType::DType;
    const DType alpha = static_cast<DType>(1);
    const DType beta = static_cast<DType>(1);
    const VType *x = nullptr;
    const VType *y = nullptr;
public:

    AXPBY(const DType &alpha, const VType &x, const DType &beta, const VType &y) : alpha(alpha), x(&x), beta(beta), y(&y) { }

    VType eval() const {
        VType w = *this->x;
        VType::AXPBY(w, this->alpha, *this->x, this->beta, *this->y);
        return w;
    }

    operator VType() const {
        return this->eval();
    }

    friend AXPBY<VType> operator*(const AXPBY<VType> &lhs, const DType &scale) {
        return AXPBY<VType>(lhs.alpha * scale, lhs.x, lhs.beta * scale, lhs.y);
    }
    friend AXPBY<VType> operator*(const AXPBY<VType> &lhs, const AsyncProxy<DType> &ascale) {
        DType scale = ascale.await();
        return lhs * scale;
    }
    friend AXPBY<VType> operator*(const DType &scale, const AXPBY<VType> &rhs) {
        return rhs * scale;
    }
    friend AXPBY<VType> operator*(const AsyncProxy<DType> &ascale, const AXPBY<VType> &rhs) {
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




