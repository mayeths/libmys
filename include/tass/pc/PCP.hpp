#pragma once

#include <petsc.h>
#include "PCAbstract.hpp"
#include "../matrix/MP.hpp"
#include "../vector/VP.hpp"

class PCP : public PCAbstract<MP, VP, PetscInt, PetscScalar>
{
public:
    using BASE = PCAbstract<MP, VP, PetscInt, PetscScalar>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;

    PC pc;

    PCP() : BASE() {
        PetscErrorCode ierr;
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
    }

    PCP(AType &A, PCType type = PCSOR) : BASE(A) {
        PetscErrorCode ierr;
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
        ierr = PCSetType(this->pc, type); CHKERRV(ierr);
        DEBUG(0, "Using PETSC precond %s", type);
        ierr = PCSetOperators(this->pc, A.mat, A.mat); CHKERRV(ierr);
        ierr = PCSetUp(this->pc); CHKERRV(ierr);
    }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        PetscErrorCode ierr;
        ierr = PCApply(this->pc, b.vec, x.vec); CHKERRV(ierr);
    }

};








