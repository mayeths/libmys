#pragma once

#include <petsc.h>
#include <memory>
#include "VAbstract.hpp"

class VP : public VAbstract<PetscInt, PetscScalar>
{
public:
    Vec vec = nullptr;

    VP() { }
    ~VP() {
        PetscErrorCode ierr;
        if (this->vec != nullptr) {
            ierr = VecDestroy(&this->vec); CHKERRV(ierr);
        }
        this->vec = nullptr;
    }
    static void duplicate(const VP &src, VP &dst) {
        if (&src == &dst) return; else dst.~VP();
        PetscErrorCode ierr;
        VecDuplicate(src.vec, &dst.vec); CHKERRV(ierr);
    }
    static void copy(const VP &src, VP &dst) {
        if (&src == &dst) return;
        PetscErrorCode ierr;
        if (dst.vec == nullptr) {
            VecDuplicate(src.vec, &dst.vec); CHKERRV(ierr);
        }
        ierr = VecCopy(src.vec, dst.vec); CHKERRV(ierr);
    }
    static void swap(VP &src, VP &dst) {
        if (&src == &dst) return; else dst.~VP();
        std::swap(src.vec, dst.vec);
    }
    VP(const VP &src)     { VP::copy(src, *this); } /* Copy Ctor. */
    VP(VP&& src) noexcept { VP::swap(src, *this); } /* Move Ctor. */
    VP& operator=(const VP &src)     { VP::copy(src, *this); return *this; } /* Copy Assign. */
    VP& operator=(VP&& src) noexcept { VP::swap(src, *this); return *this; } /* Move Assign. */


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

    static async<PetscScalar> dot(const VP &x, const VP &y) {
        PetscScalar result;
        VecDot(x.vec, y.vec, &result);
        return async<PetscScalar>(result);
    }

    static async<PetscScalar> idot(const VP &x, const VP &y) {
        VecDotBegin(x.vec, y.vec, NULL);
        return async<PetscScalar>(0, &x, &y, &VP::idot_await);
    }

    static PetscScalar idot_await(const async<PetscScalar> *as) {
        const VP &x = *(static_cast<const VP *>(as->context1));
        const VP &y = *(static_cast<const VP *>(as->context2));
        PetscScalar result = 0;
        VecDotEnd(x.vec, y.vec, &result);
        return result;
    }


    VP& operator+=(const VP &x) { VecAXPY(this->vec, 1, x.vec); return *this; }
    friend VP operator+(VP x, const VP& y) { return x += y; }

    VP& operator-=(const VP &x) { VecAXPY(this->vec, -1, x.vec); return *this; }
    friend VP operator-(VP x, const VP& y) { return x -= y; }

    VP& operator*=(PetscScalar alpha) { VecScale(this->vec, alpha); return *this; }
    friend VP operator*(VP x, PetscScalar alpha) { return x *= alpha; }
    friend VP operator*(PetscScalar alpha, VP x) { return x * alpha; }

    friend async<PetscScalar> operator,(const VP &x, const VP& y) {
        return VP::idot(x, y);
        // return VP::dot(x, y);
    }

};
