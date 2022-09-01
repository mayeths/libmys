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

    PC pc = nullptr;

    PCP() : BASE() {
        PetscErrorCode ierr;
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
    }
    ~PCP() {
        PetscErrorCode ierr;
        if (this->pc != nullptr) {
            ierr = PCDestroy(&pc); CHKERRV(ierr);
        }
        this->pc = nullptr;
    }
    static void duplicate(const PCP &src, PCP &dst) {
        if (&src != &dst) dst.~PCP(); else return;
        PetscErrorCode ierr;
        MPI_Comm comm;
        PCType type;
        Mat Amat, Pmat;
        ierr = PetscObjectGetComm((PetscObject)src.pc, &comm); CHKERRV(ierr);
        ierr = PCGetType(src.pc, &type); CHKERRV(ierr);
        ierr = PCGetOperators(src.pc, &Amat, &Pmat); CHKERRV(ierr);
        ierr = PCCreate(comm, &dst.pc); CHKERRV(ierr);
        ierr = PCSetType(dst.pc, type); CHKERRV(ierr);
        ierr = PCSetOperators(dst.pc, Amat, Pmat); CHKERRV(ierr);
    }
    static void swap(PCP &src, PCP &dst) {
        if (&src != &dst) dst.~PCP(); else return;
        std::swap(src.pc, dst.pc);
    }
    PCP(const PCP &src)     { PCP::duplicate(src, *this); } /* Copy Ctor. */
    PCP(PCP&& src) noexcept { PCP::swap(src, *this);      } /* Move Ctor. */
    PCP& operator=(const PCP &src)     { PCP::duplicate(src, *this); return *this; } /* Copy Assign. */
    PCP& operator=(PCP&& src) noexcept { PCP::swap(src, *this); return *this;      } /* Move Assign. */


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








