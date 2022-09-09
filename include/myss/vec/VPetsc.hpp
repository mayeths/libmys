#pragma once

#include <petsc.h>
#include <memory>
#include "VAbstract.hpp"
#include "../util/AsyncProxy.hpp"

class VPetsc : public VAbstract<PetscInt, PetscScalar>
{
public:
    using data_t = VAbstract<PetscInt, PetscScalar>::data_t;
    Vec vec = nullptr;

    VPetsc() { }
    ~VPetsc() {
        PetscErrorCode ierr;
        if (this->vec != nullptr) {
            ierr = VecDestroy(&this->vec); CHKERRV(ierr);
        }
        this->vec = nullptr;
    }
    static void duplicate(const VPetsc &src, VPetsc &dst) {
        if (&src == &dst) return; else dst.~VPetsc();
        PetscErrorCode ierr;
        VecDuplicate(src.vec, &dst.vec); CHKERRV(ierr);
    }
    static void copy(const VPetsc &src, VPetsc &dst) {
        if (&src == &dst) return;
        PetscErrorCode ierr;
        if (dst.vec == nullptr) {
            VecDuplicate(src.vec, &dst.vec); CHKERRV(ierr);
        }
        ierr = VecCopy(src.vec, dst.vec); CHKERRV(ierr);
    }
    static void swap(VPetsc &src, VPetsc &dst) {
        if (&src == &dst) return; else dst.~VPetsc();
        std::swap(src.vec, dst.vec);
    }
    VPetsc(const VPetsc &src)     { VPetsc::copy(src, *this); } /* Copy Ctor. */
    VPetsc(VPetsc&& src) noexcept { VPetsc::swap(src, *this); } /* Move Ctor. */
    VPetsc& operator=(const VPetsc &src)     { VPetsc::copy(src, *this); return *this; } /* Copy Assign. */
    VPetsc& operator=(VPetsc&& src) noexcept { VPetsc::swap(src, *this); return *this; } /* Move Assign. */


    void SetValues(const PetscScalar *arr) {
        PetscErrorCode ierr;
        PetscInt Istart, Iend;
        ierr = VecGetOwnershipRange(this->vec, &Istart, &Iend); CHKERRV(ierr);
        for (PetscInt I = Istart; I < Iend; I++) {
            const PetscInt i = I - Istart;
            PetscScalar v = static_cast<PetscScalar>(arr[i]);
            ierr = VecSetValues(this->vec, 1, &I, &v, INSERT_VALUES); CHKERRV(ierr);
        }
        ierr = VecAssemblyBegin(this->vec); CHKERRV(ierr);
        ierr = VecAssemblyEnd(this->vec); CHKERRV(ierr);
    }

    PetscReal Norm() {
        PetscReal norm;
        VecNorm(this->vec, NORM_2, &norm);
        return norm;
    }

    static AsyncProxy<PetscScalar> async_dot(const VPetsc &x, const VPetsc &y) {
        VecDotBegin(x.vec, y.vec, NULL);
        auto context = new std::pair<const VPetsc*, const VPetsc*>(&x, &y);
        return AsyncProxy<PetscScalar>(0, context, &VPetsc::await_dot);
    }

    static PetscScalar await_dot(const AsyncProxy<PetscScalar> *proxy) {
        auto context = (std::pair<const VPetsc*, const VPetsc*> *)proxy->context();
        const VPetsc *x = context->first;
        const VPetsc *y = context->second;
        delete context;

        PetscScalar result = 0;
        VecDotEnd(x->vec, y->vec, &result);
        return result;
    }


    VPetsc& operator+=(const VPetsc &x) { VecAXPY(this->vec, 1, x.vec); return *this; }
    friend VPetsc operator+(VPetsc x, const VPetsc& y) { return x += y; }

    VPetsc& operator-=(const VPetsc &x) { VecAXPY(this->vec, -1, x.vec); return *this; }
    friend VPetsc operator-(VPetsc x, const VPetsc& y) { return x -= y; }

    VPetsc& operator*=(PetscScalar alpha) { VecScale(this->vec, alpha); return *this; }
    friend VPetsc operator*(VPetsc x, PetscScalar alpha) { return x *= alpha; }
    friend VPetsc operator*(PetscScalar alpha, VPetsc x) { return x * alpha; }
};


template <typename VType>
AsyncProxy<typename VType::data_t> dot(const VType &x, const VType &y) {
        auto result = VType::async_dot(x, y);
        result.await();
        return result;
}

template <typename VType>
AsyncProxy<typename VType::data_t> operator,(const VType &x, const VType& y) {
    // return trycall_async_dot(x, y, std::integral_constant<bool, has_async_dot<VType>::value>());
    // return dot(x, y);
    return VType::async_dot(x, y);
}


// https://en.cppreference.com/w/cpp/meta
// https://www.fluentcpp.com/2018/05/18/make-sfinae-pretty-2-hidden-beauty-sfinae/
// https://stackoverflow.com/a/23133787
// https://stackoverflow.com/a/87846
// #define SFINAE_HAS_SIGNATURE(funcName, signature)                 \
//     template <typename U>                                                     \
//     class has_##funcName                                                          \
//     {                                                                         \
//     private:                                                                  \
//         template<typename T, T> struct SFINAE;                                \
//         template<typename T> static int check(SFINAE<signature, &T::funcName>*); \
//         template<typename T> static char check(...);                          \
//     public:                                                                   \
//         static const bool value = sizeof(check<U>(0)) == sizeof(int);         \
//         /*static_assert(value, "Please check you definition of " #funcName " has signature: " #signature); */\
//     };

// SFINAE_HAS_SIGNATURE(async_dot, AsyncProxy<typename T::data_t> (*)(const U&, const U&));
// template <typename VType>
// AsyncProxy<typename VType::data_t> trycall_async_dot(const VType& x, const VType& y, std::true_type) {
//     return VType::async_dot(x, y);
// }
// template <typename VType> /* Comment this can throw error on compile-time instead of run-time */
// AsyncProxy<typename VType::data_t> trycall_async_dot(const VType&, const VType&, std::false_type)
// {
// }
