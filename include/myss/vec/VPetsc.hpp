#pragma once

#include <petsc.h>
#include <memory>
#include "VAbstract.hpp"

class VPetsc : public VAbstract<VPetsc, PetscInt, PetscScalar>
{
public:
    using BASE = VAbstract<VPetsc, PetscInt, PetscScalar>;
    using IType = BASE::IType;
    using DType = BASE::DType;
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
        ierr = VecDuplicate(src.vec, &dst.vec); CHKERRV(ierr);
    }
    static void fill(const VPetsc &src, PetscScalar val) {
        PetscErrorCode ierr;
        ierr = VecSet(src.vec, val); CHKERRV(ierr);
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
        if (&src == &dst) return;
        std::swap(src.vec, dst.vec);
    }
    VPetsc(const VPetsc &src) { VPetsc::copy(src, *this); }
    VPetsc(const BASE &src)   { VPetsc::copy(static_cast<const VPetsc&>(src), *this); }
    VPetsc(VPetsc&& src) noexcept { VPetsc::swap(src, *this); }
    VPetsc& operator=(const VPetsc &src)     { VPetsc::copy(src, *this); return *this; }
    VPetsc& operator=(VPetsc&& src) noexcept { VPetsc::swap(src, *this); return *this; }

    bool iscompact(const BASE &other) {
        return true;
    }

    static void AXPBY(VPetsc &w, PetscScalar alpha, const VPetsc &x, PetscScalar beta, const VPetsc &y) {
        PetscErrorCode ierr;
        if (&x == &y) {
            ierr = VecAXPY(w.vec, alpha, x.vec); CHKERRV(ierr);
            ierr = VecAXPY(w.vec, beta, y.vec); CHKERRV(ierr);
        } else if (&w != &x && &x != &y) {
            ierr = VecAXPBYPCZ(w.vec, alpha, beta, 0, x.vec, y.vec); CHKERRV(ierr);
        } else {
            FAILED("%p %p %p", &w, &x, &y);
        }
    }

    static void Set(VPetsc &y, PetscScalar alpha, ValueSetOp op = ValueSetOp::Insert) {
        PetscErrorCode ierr;
        if (op == ValueSetOp::Insert) {
            ierr = VecSet(y.vec, alpha); CHKERRV(ierr);
        } else if (op == ValueSetOp::Shift) {
            ierr = VecShift(y.vec, alpha); CHKERRV(ierr);
        } else if (op == ValueSetOp::Scale) {
            ierr = VecScale(y.vec, alpha); CHKERRV(ierr);
        }
    }

    static AsyncProxy<PetscScalar> AsyncDot(const VPetsc &x, const VPetsc &y) {
        VecDotBegin(x.vec, y.vec, NULL);
        auto context = new std::pair<const VPetsc*, const VPetsc*>(&x, &y);
        return AsyncProxy<PetscScalar>(0, context, &VPetsc::AwaitDot);
    }

    static PetscScalar AwaitDot(const AsyncProxy<PetscScalar> *proxy) {
        auto context = (std::pair<const VPetsc*, const VPetsc*> *)proxy->context();
        const VPetsc *x = context->first;
        const VPetsc *y = context->second;
        delete context;

        PetscScalar result = 0;
        VecDotEnd(x->vec, y->vec, &result);
        return result;
    }


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

};
