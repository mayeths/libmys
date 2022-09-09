#pragma once

#include <petsc.h>
#include "PCAbstract.hpp"
#include "../mat/MPetsc.hpp"
#include "../vec/VPetsc.hpp"
#include <hpss_c.h>

#if 1
static HpssSmootherHandle smoother = nullptr;
class PCPetscHpss : public PCAbstract<MPetsc>
{
public:
    using BASE = PCAbstract<MPetsc>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using index_t = typename BASE::index_t;
    using data_t = typename BASE::data_t;


    PC pc = nullptr;
    PetscInt nrows = 0;
    PetscInt *Ap = nullptr;
    PetscInt *Aj = nullptr;
    PetscScalar *Av = nullptr;

    PCPetscHpss() : BASE() {
        PetscErrorCode ierr;
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
    }

    PCPetscHpss(AType &A, PetscInt nrows, PetscInt *Ap, PetscInt *Aj, PetscScalar *Av, PetscInt Istart = 0) : BASE(A) {
        // PetscErrorCode ierr;
        // ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
        // ierr = PCSetType(this->pc, PCSOR); CHKERRV(ierr);
        // ierr = PCSetOperators(this->pc, A.mat, A.mat); CHKERRV(ierr);
        // ierr = PCSetUp(this->pc); CHKERRV(ierr);
        PetscErrorCode ierr;
        this->nrows = nrows;
        this->Ap = Ap;
        this->Aj = Aj;
        this->Av = Av;
        // dupcopy(nrows, Ap, Aj, Av, &this->Ap, &this->Aj, &this->Av);
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
        ierr = PCSetType(this->pc, PCSHELL); CHKERRV(ierr);
        ierr = PCSetOperators(this->pc, A.mat, A.mat); CHKERRV(ierr);
        ierr = PCShellSetContext(this->pc, (void*)this); CHKERRV(ierr);
        ierr = PCShellSetApply(this->pc, &PCPetscHpss::MyPCapply); CHKERRV(ierr);
        ierr = PCShellSetSetUp(this->pc, &PCPetscHpss::MyPCsetup); CHKERRV(ierr);
        ierr = PCSetUp(this->pc); CHKERRV(ierr);
    }

    static PetscErrorCode MyPCsetup(PC pc) {
        PetscErrorCode ierr;
        PCPetscHpss* self = NULL;
        ierr = PCShellGetContext(pc, &self); CHKERRQ(ierr);
        HpssAnalyseConfig analyseConfig;
        HpssInitAnalyseConfig(&analyseConfig);
        analyseConfig.valueType  = Float64;
        analyseConfig.base       = 0;
        analyseConfig.symmetry   = NonSymmetric;
        analyseConfig.numThreads = 1;
        analyseConfig.sorType    = (HpssSorType)0;
        analyseConfig.iluLevel   = 0;
        analyseConfig.debug      = 0;

        HpssStructuralInfo info;
        info.type = UnStructural;
        analyseConfig.structuralInfo = info;

        for (int i = 0; i < self->nrows; i++) {
            const int rowstart = self->Ap[i];
            const int rowend = self->Ap[i + 1];
            for (int jj = rowstart; jj < rowend; jj++) {
                const int j = self->Aj[jj];
                const double v = self->Av[jj];
                ASSERT_BETWEEN_IE(0, j, self->nrows);
            }
        }

        HpssCreateSmoother(&smoother, Sor);
        HpssSmootherAnalyseFromCsr(smoother, self->nrows, self->nrows, self->Ap, self->Aj, NULL, &analyseConfig);
        HpssSmootherPrepareFromCsr(smoother, NULL, NULL, self->Av, NULL);
        // BARRIER();
        // exit(0);
        return 0;
    }

    static PetscErrorCode MyPCapply(PC pc, Vec petsc_b, Vec petsc_x) {
        PetscFunctionBegin;
        PetscErrorCode ierr;
        PCPetscHpss* self = NULL;
        // DEBUG(0, "1.0"); BARRIER();
        ierr = PCShellGetContext(pc, &self); CHKERRQ(ierr);
        PetscInt Istart, Iend;
        // DEBUG(0, "1.1"); BARRIER();
        // ierr = VecGetOwnershipRange(petsc_b, &Istart, &Iend); CHKERRQ(ierr);
        // DEBUG(0, "1.2"); BARRIER();
        // ASSERT_EQ(Iend - Istart, self->nrows);
        // DEBUG(0, "1.3"); BARRIER();
        // ierr = VecGetOwnershipRange(petsc_x, &Istart, &Iend); CHKERRQ(ierr);
        // DEBUG(0, "1.4"); BARRIER();
        // ASSERT_EQ(Iend - Istart, self->nrows);
        // DEBUG(0, "1.5"); BARRIER();
        PetscScalar *xarr;
        // DEBUG(0, "1.6"); BARRIER();
        const PetscScalar *barr;
        ierr = VecGetArrayRead(petsc_b, &barr); CHKERRQ(ierr);
        // DEBUG(0, "1.7"); BARRIER();
        ierr = VecGetArray(petsc_x, &xarr); CHKERRQ(ierr);
        HpssApplyConfig applyConfig;
        // DEBUG(0, "1.8"); BARRIER();
        HpssInitApplyConfig(&applyConfig);
        applyConfig.bPermType = 0;
        applyConfig.xPermType = 0;
        // DEBUG(0, "1.9"); BARRIER();
        // HpssSmootherApply(smoother, barr, xarr, &applyConfig);
        std::copy(barr, barr + self->nrows, xarr);
        // DEBUG(0, "1.10"); BARRIER();
        ierr = VecRestoreArrayRead(petsc_b, &barr); CHKERRQ(ierr);
        // DEBUG(0, "1.11"); BARRIER();
        ierr = VecRestoreArray(petsc_x, &xarr); CHKERRQ(ierr);
        // DEBUG(0, "1.12"); BARRIER();
        PetscFunctionReturn(0);
    }

    virtual void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        PetscErrorCode ierr;
        ierr = PCApply(this->pc, b.vec, x.vec); CHKERRV(ierr);
    }
    virtual const char *GetName() const {
        return "PCPetscHpss";
    }

};






#else

static HpssSmootherHandle hpss_smoother;


class PCPetscHpss : public PCAbstract<MPetsc>
{
public:
    using BASE = PCAbstract<MPetsc>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;

    PC pc;
    PetscInt nrows = 0;
    PetscInt *Ap = nullptr;
    PetscInt *Aj = nullptr;
    PetscScalar *Av = nullptr;

    PCPetscHpss() : BASE() {
        PetscErrorCode ierr;
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
    }

    PCPetscHpss(AType &A, PetscInt nrows, PetscInt *Ap, PetscInt *Aj, PetscScalar *Av, PetscInt Istart = 0) : BASE(A) {
        PetscErrorCode ierr;
        ierr = PCCreate(MPI_COMM_WORLD, &this->pc); CHKERRV(ierr);
        ierr = PCSetType(this->pc, PCSOR); CHKERRV(ierr);
        ierr = PCSetOperators(this->pc, A.mat, A.mat); CHKERRV(ierr);
        ierr = PCSetUp(this->pc); CHKERRV(ierr);
    }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        PetscErrorCode ierr;
        ierr = PCApply(this->pc, *b.vec, *x.vec); CHKERRV(ierr);
    }

};



#endif

