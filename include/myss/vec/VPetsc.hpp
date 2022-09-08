#pragma once

#include <petsc.h>
#include <memory>
#include "VAbstract.hpp"
#include "../util/AsyncProxy.hpp"

class VPetsc : public VAbstract<PetscInt, PetscScalar>
{
public:
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

    static AsyncProxy<PetscScalar> dot(const VPetsc &x, const VPetsc &y) {
        PetscScalar result;
        VecDot(x.vec, y.vec, &result);
        return AsyncProxy<PetscScalar>(result);
    }

    static AsyncProxy<PetscScalar> idot(const VPetsc &x, const VPetsc &y) {
        VecDotBegin(x.vec, y.vec, NULL);
        auto context = new std::pair<const VPetsc*, const VPetsc*>(&x, &y);
        return AsyncProxy<PetscScalar>(0, context, &VPetsc::idot_await);
    }

    static PetscScalar idot_await(const AsyncProxy<PetscScalar> *as) {
        auto context = (std::pair<const VPetsc*, const VPetsc*> *)as->context();
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

    friend AsyncProxy<PetscScalar> operator,(const VPetsc &x, const VPetsc& y) {
        return VPetsc::idot(x, y);
        // return VPetsc::dot(x, y);
    }

};
