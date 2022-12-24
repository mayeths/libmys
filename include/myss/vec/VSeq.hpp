#pragma once

#include <memory>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <math.h>
#include "VBase.hpp"
#include "mys/raii.hpp"

class VSeq : public VBase<VSeq, int, double>
{
public:
    using BASE = VBase<VSeq, int, double>;
    using IType = BASE::IType;
    using DType = BASE::DType;

    int nrows = -1;
    std::vector<double> values;
    guard_t guard;

    VSeq() { }
    ~VSeq() {
        this->nrows = -1;
        this->values.clear();
        this->guard.reset();
    }

    VSeq(const std::vector<double> values) {
        this->nrows = values.size();
        this->values.resize(this->nrows);
        std::copy(values.begin(), values.end(), this->values.begin());
        this->guard.set();
    }

    static void copy(const VSeq &src, VSeq &dst) {
        if (&src == &dst) return;
        dst.nrows = src.nrows;
        dst.values.clear();
        dst.values.resize(src.values.size());
        std::copy(src.values.begin(), src.values.end(), dst.values.begin());
        dst.guard = src.guard;
    }
    static void swap(VSeq &src, VSeq &dst) {
        if (&src == &dst) return;
        std::swap(src.nrows, dst.nrows);
        std::swap(src.values, dst.values);
        std::swap(src.guard, dst.guard);
    }
    VSeq(const VSeq &src) { VSeq::copy(src, *this); }
    VSeq(VSeq&& src) noexcept { VSeq::swap(src, *this); }
    VSeq& operator=(const VSeq &src)     { VSeq::copy(src, *this); return *this; }
    VSeq& operator=(VSeq&& src) noexcept { VSeq::swap(src, *this); return *this; }

    static void AXPBY(VSeq &w, double alpha, const VSeq &x, double beta, const VSeq &y) {
        w.guard.ensure();
        x.guard.ensure();
        y.guard.ensure();
        ASSERT_EQ(w.values.size(), x.values.size());
        ASSERT_EQ(x.values.size(), y.values.size());
        #pragma omp parallel for
        for (size_t i = 0; i < y.values.size(); i++) {
            w.values[i] = alpha * x.values[i] + beta * y.values[i];
        }
    }

    static void ElementWiseOp(VSeq &y, const VSeq &x, ElementOp op) {
        x.guard.ensure();
        y.guard.ensure();
        ASSERT_EQ(x.values.size(), y.values.size());
        if (op == ElementOp::Replace) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i];
            }
        } else if (op == ElementOp::Shift) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i] + y.values[i];
            }
        } else if (op == ElementOp::Scale) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i] * y.values[i];
            }
        } else if (op == ElementOp::Reciprocal) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i] / y.values[i];
            }
        } else {
            throw std::runtime_error("Not supported.");
        }
    }

    static void ElementWiseOp(VSeq &y, double alpha, ElementOp op) {
        y.guard.ensure();
        if (op == ElementOp::Replace) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha;
            }
        } else if (op == ElementOp::Shift) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha + y.values[i];
            }
        } else if (op == ElementOp::Scale) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha * y.values[i];
            }
        } else if (op == ElementOp::Reciprocal) {
            #pragma omp parallel for
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha / y.values[i];
            }
        } else {
            throw std::runtime_error("Not supported.");
        }
    }

    static AsyncProxy<double> AsyncDot(const VSeq &x, const VSeq &y) {
        x.guard.ensure();
        y.guard.ensure();
        ASSERT_EQ(x.values.size(), y.values.size());
        auto context = new std::pair<const VSeq*, const VSeq*>(&x, &y);
        return AsyncProxy<double>(0, context, &VSeq::AwaitDot);
    }

    static double AwaitDot(const AsyncProxy<double> *proxy) {
        auto context = (std::pair<const VSeq*, const VSeq*> *)proxy->context();
        const VSeq *x = context->first;
        const VSeq *y = context->second;
        delete context;

        double result = 0;
        #pragma omp parallel for reduction(+ : result)
        for (int i = 0; i < x->values.size(); i++) {
            result += x->values[i] * y->values[i];
        }
        return result;
    }

};

