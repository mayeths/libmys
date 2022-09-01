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
    }

    VP(const VP &source) : VP() {
        PetscErrorCode ierr;
        VecDuplicate(source.vec, &this->vec); CHKERRV(ierr);
    }

    VP(VP &&source) {
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

};
