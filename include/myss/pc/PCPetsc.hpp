#pragma once

#include <petsc.h>
#include "PCAbstract.hpp"
#include "../mat/MPetsc.hpp"
#include "../vec/VPetsc.hpp"

class PCPetsc : public PCAbstract<MPetsc, VPetsc, PetscInt, PetscScalar>
{
public:
    using BASE = PCAbstract<MPetsc, VPetsc, PetscInt, PetscScalar>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;

    PC pc = nullptr;

    PCPetsc() : BASE() {
        PetscErrorCode ierr;
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
    }
    ~PCPetsc() {
        PetscErrorCode ierr;
        if (this->pc != nullptr) {
            ierr = PCDestroy(&pc); CHKERRV(ierr);
        }
        this->pc = nullptr;
    }
    static void duplicate(const PCPetsc &src, PCPetsc &dst) {
        if (&src != &dst) dst.~PCPetsc(); else return;
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
    static void swap(PCPetsc &src, PCPetsc &dst) {
        if (&src != &dst) dst.~PCPetsc(); else return;
        std::swap(src.pc, dst.pc);
    }
    PCPetsc(const PCPetsc &src)     { PCPetsc::duplicate(src, *this); } /* Copy Ctor. */
    PCPetsc(PCPetsc&& src) noexcept { PCPetsc::swap(src, *this);      } /* Move Ctor. */
    PCPetsc& operator=(const PCPetsc &src)     { PCPetsc::duplicate(src, *this); return *this; } /* Copy Assign. */
    PCPetsc& operator=(PCPetsc&& src) noexcept { PCPetsc::swap(src, *this); return *this;      } /* Move Assign. */


    PCPetsc(AType &A, PCType type = PCSOR) : BASE(A) {
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







