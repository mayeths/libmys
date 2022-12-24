#pragma once

#include <memory>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <math.h>
#include "VBase.hpp"
#include "mys/raii.hpp"

class VCSR : public VBase<VCSR, int, double>
{
public:
    using BASE = VBase<VCSR, int, double>;
    using IType = BASE::IType;
    using DType = BASE::DType;

    int global_size = -1;
    int local_size = -1;
    int local_disp = -1;
    std::vector<double> values;
    guard_t guard;

    VCSR() { }
    ~VCSR() {
        this->global_size = -1;
        this->local_disp = -1;
        this->local_size = -1;
        this->values.clear();
        this->guard.reset();
    }

    VCSR(const int global_size, const int local_size, const int local_disp, const double *arr) {
        this->global_size = global_size;
        this->local_size = local_size;
        this->local_disp = local_disp;
        this->values.resize(local_size, 0);
        if (arr != NULL) {
            std::copy(arr, arr + local_size, this->values.data());
        }
        this->guard.set();
    }

    // void ResizeForHalo(int halo_total_size)
    // {
    //     this->guard.ensure();
    //     int old = this->values.size();
    //     this->values.resize(this->values.size() + halo_total_size, 0);
    //     // DEBUG(0, "Resized from %d to %d", old, this->values.size());
    // }

    static VCSR FromGlobalVector(const double *arr, const int global_size, const int local_begin, const int local_end)
    {
        VCSR res;
        res.global_size = global_size;
        res.local_size = local_end - local_begin;
        res.local_disp = local_begin;
        res.values.resize(res.local_size);
        for (int i = local_begin; i < local_end; i++) {
            res.values[i - local_begin] = arr[i];
        }
        res.guard.set();
        return res;
    }

    static void copy(const VCSR &src, VCSR &dst) {
        if (&src == &dst) return;
        dst.global_size = dst.global_size;
        dst.local_size = dst.local_size;
        dst.local_disp = dst.local_disp;
        dst.values.clear();
        dst.values.resize(src.values.size());
        std::copy(src.values.begin(), src.values.end(), dst.values.begin());
        dst.guard = src.guard;
    }
    static void swap(VCSR &src, VCSR &dst) {
        if (&src == &dst) return;
        std::swap(src.global_size, dst.global_size);
        std::swap(src.local_size, dst.local_size);
        std::swap(src.local_disp, dst.local_disp);
        std::swap(src.values, dst.values);
        std::swap(src.guard, dst.guard);
    }
    VCSR(const VCSR &src) { VCSR::copy(src, *this); }
    VCSR(VCSR&& src) noexcept { VCSR::swap(src, *this); }
    VCSR& operator=(const VCSR &src)     { VCSR::copy(src, *this); return *this; }
    VCSR& operator=(VCSR&& src) noexcept { VCSR::swap(src, *this); return *this; }

    static void AXPBY(VCSR &w, double alpha, const VCSR &x, double beta, const VCSR &y) {
        w.guard.ensure();
        x.guard.ensure();
        y.guard.ensure();
        for (size_t i = 0; i < y.values.size(); i++) {
            w.values[i] = alpha * x.values[i] + beta * y.values[i];
        }
    }

    static void ElementWiseOp(VCSR &y, const VCSR &x, ElementOp op) {
        x.guard.ensure();
        y.guard.ensure();
        if (op == ElementOp::Replace) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i];
            }
        } else if (op == ElementOp::Shift) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i] + y.values[i];
            }
        } else if (op == ElementOp::Scale) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i] * y.values[i];
            }
        } else if (op == ElementOp::Reciprocal) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = x.values[i] / y.values[i];
            }
        } else {
            throw std::runtime_error("Not supported.");
        }
    }

    static void ElementWiseOp(VCSR &y, double alpha, ElementOp op) {
        y.guard.ensure();
        if (op == ElementOp::Replace) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha;
            }
        } else if (op == ElementOp::Shift) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha + y.values[i];
            }
        } else if (op == ElementOp::Scale) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha * y.values[i];
            }
        } else if (op == ElementOp::Reciprocal) {
            for (size_t i = 0; i < y.values.size(); i++) {
                y.values[i] = alpha / y.values[i];
            }
        } else {
            throw std::runtime_error("Not supported.");
        }
    }

    static AsyncProxy<double> AsyncDot(const VCSR &x, const VCSR &y) {
        x.guard.ensure();
        y.guard.ensure();
        ASSERT_EQ(x.values.size(), y.values.size());
        auto context = new std::pair<const VCSR*, const VCSR*>(&x, &y);
        return AsyncProxy<double>(0, context, &VCSR::AwaitDot);
    }

    static double AwaitDot(const AsyncProxy<double> *proxy) {
        auto context = (std::pair<const VCSR*, const VCSR*> *)proxy->context();
        const VCSR *x = context->first;
        const VCSR *y = context->second;
        delete context;

        double result = 0;
        for (int i = 0; i < x->values.size(); i++) {
            result += x->values[i] * y->values[i];
        }
        return result;
    }

    double Norm(std::string type = "2") {
        this->guard.ensure();
        if (type == "2") {
            double result = 0;
            for (size_t i = 0; i < this->values.size(); i++) {
                result += this->values[i] * this->values[i];
            }
            return std::sqrt(result);
        } else if (type == "inf") {
            double result = 0;
            for (size_t i = 0; i < this->values.size(); i++) {
                result += fabs(this->values[i]);
            }
            return result;
        }
        return -1;
    }

};
