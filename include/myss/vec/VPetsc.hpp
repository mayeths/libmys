#pragma once

#include <petsc.h>
#include <memory>
#include "VAbstract.hpp"

class VPetsc : public VAbstract<VPetsc, PetscInt, PetscScalar>
{
public:
    using index_t = VAbstract<VPetsc, PetscInt, PetscScalar>::index_t;
    using data_t = VAbstract<VPetsc, PetscInt, PetscScalar>::data_t;
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

    static void axpy(VPetsc &y, PetscScalar alpha, const VPetsc &x) {
        VecAXPY(y.vec, alpha, x.vec);
    }

    static void scale(VPetsc &y, PetscScalar alpha) {
        VecScale(y.vec, alpha);
    }

};
