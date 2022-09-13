#pragma once

#include <map>
#include <petsc.h>
#include "MAbstract.hpp"
#include "../vec/VPetsc.hpp"

class MPetsc : public MAbstract<VPetsc, PetscInt, PetscScalar>
{
public:
    using BASE = MAbstract<VPetsc, PetscInt, PetscScalar>;
    using VType = BASE::VType;
    using IType = BASE::IType;
    using DType = BASE::DType;
    PetscInt Istart, Iend, Jstart, Jend;
    PetscInt NROWS, NCOLS, nrows, ncols;
    Mat mat;

    MPetsc(const MPetsc &source) {
        PetscErrorCode ierr;
        MatDuplicate(source.mat, MAT_DO_NOT_COPY_VALUES, &this->mat); CHKERRV(ierr);
    }

    /* Construct by COO */
    MPetsc(PetscInt nnz, PetscInt *Ia, PetscInt *Ja, PetscScalar *Va) {
        PetscInt Istart = std::numeric_limits<PetscInt>::max();
        PetscInt Jstart = std::numeric_limits<PetscInt>::max();
        PetscInt Iend = std::numeric_limits<PetscInt>::min();
        PetscInt Jend = std::numeric_limits<PetscInt>::min();
        PetscInt maxrownnz = std::numeric_limits<PetscInt>::min();
        {
            std::map<PetscInt, PetscInt> rownnz;
            for (PetscInt jj = 0; jj < nnz; jj++) {
                const PetscInt I = Ia[jj];
                const PetscInt J = Ja[jj];
                rownnz[I] += 1;
                maxrownnz = std::max(maxrownnz, rownnz[I]);
                Istart = std::min(Istart, I);
                Jstart = std::min(Jstart, J);
                Iend = std::max(Iend, I + 1);
                Jend = std::max(Jend, J + 1);
            }
            this->nrows = Iend - Istart;
            this->ncols = Jend - Jstart;
        }

        PetscErrorCode ierr;
        ierr = MatCreate(PETSC_COMM_WORLD, &this->mat); CHKERRV(ierr);
        ierr = MatSetSizes(this->mat, this->nrows, this->nrows, PETSC_DETERMINE, PETSC_DETERMINE); CHKERRV(ierr);
        ierr = MatSetFromOptions(this->mat); CHKERRV(ierr);
        ierr = MatMPIAIJSetPreallocation(this->mat, maxrownnz, NULL, maxrownnz, NULL); CHKERRV(ierr);
        ierr = MatSeqAIJSetPreallocation(this->mat, maxrownnz, NULL); CHKERRV(ierr);
        ierr = MatSetOption(this->mat, MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE); CHKERRV(ierr);
        ierr = MatGetOwnershipRange(this->mat, &this->Istart, &this->Iend); CHKERRV(ierr);
        ierr = MatGetOwnershipRangeColumn(this->mat, &this->Jstart, &this->Jend); CHKERRV(ierr);
        ierr = MatGetSize(this->mat, &this->NROWS, &this->NCOLS); CHKERRV(ierr);
        ASSERT_EQ(this->Iend - this->Istart, this->nrows);
        for (PetscInt jj = 0; jj < nnz; jj++) {
            const int I = Ia[jj];
            const int J = Ja[jj];
            const double V = Va[jj];
            ierr = MatSetValues(this->mat, 1, &I, 1, &J, &V, INSERT_VALUES); CHKERRV(ierr);
        }
        ierr = MatAssemblyBegin(this->mat, MAT_FINAL_ASSEMBLY); CHKERRV(ierr);
        ierr = MatAssemblyEnd(this->mat, MAT_FINAL_ASSEMBLY); CHKERRV(ierr);
    }

    /* Construct by CSR */
    MPetsc(PetscInt nrows, PetscInt ncols, PetscInt *Ap, PetscInt *Aj, PetscScalar *Av) {
        this->nrows = nrows;
        this->ncols = ncols;
        PetscInt maxrownnz = std::numeric_limits<PetscInt>::min();
        for (PetscInt i = 0; i < this->nrows; i++) {
            const PetscInt rowstart = Ap[i];
            const PetscInt rowend = Ap[i + 1];
            maxrownnz = std::max(maxrownnz, rowend - rowstart);
        }
        PetscErrorCode ierr;
        ierr = MatCreate(PETSC_COMM_WORLD, &this->mat); CHKERRV(ierr);
        ierr = MatSetSizes(this->mat, nrows, nrows, PETSC_DETERMINE, PETSC_DETERMINE); CHKERRV(ierr);
        ierr = MatSetFromOptions(this->mat); CHKERRV(ierr);
        ierr = MatMPIAIJSetPreallocation(this->mat, 1, NULL, 1, NULL); CHKERRV(ierr);
        ierr = MatSeqAIJSetPreallocation(this->mat, 1, NULL); CHKERRV(ierr);
        ierr = MatSetOption(this->mat, MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE); CHKERRV(ierr);
        ierr = MatGetOwnershipRange(this->mat, &this->Istart, &this->Iend); CHKERRV(ierr);
        ierr = MatGetOwnershipRangeColumn(this->mat, &this->Jstart, &this->Jend); CHKERRV(ierr);
        ierr = MatGetSize(this->mat, &this->NROWS, &this->NCOLS); CHKERRV(ierr);
        ASSERT_EQ(this->Iend - this->Istart, this->nrows);
        for (PetscInt i = 0; i < this->nrows; i++) {
            const PetscInt rowstart = Ap[i];
            const PetscInt rowend = Ap[i + 1];
            for (PetscInt jj = rowstart; jj < rowend; jj++) {
                const PetscInt J = Aj[jj];
                const PetscScalar V = Av[jj];
                const PetscInt I = this->Istart + i;
                ierr = MatSetValues(this->mat, 1, &I, 1, &J, &V, INSERT_VALUES); CHKERRV(ierr);
            }
        }
        ierr = MatAssemblyBegin(this->mat, MAT_FINAL_ASSEMBLY); CHKERRV(ierr);
        ierr = MatAssemblyEnd(this->mat, MAT_FINAL_ASSEMBLY); CHKERRV(ierr);
    }

    ~MPetsc() {
        PetscErrorCode ierr;
        ierr = MatDestroy(&this->mat); CHKERRV(ierr);
    }


    void CreateVecs(VPetsc &x, VPetsc &b) {
        PetscErrorCode ierr;
        ierr = MatCreateVecs(this->mat, &x.vec, &b.vec); CHKERRV(ierr);
    }

    PetscReal Norm() {
        PetscReal norm;
        MatNorm(this->mat, NORM_FROBENIUS, &norm);
        return norm;
    }

    virtual void Apply(const VPetsc &x, VPetsc &y, bool xzero = false) const {
        MatMult(this->mat, x.vec, y.vec);
    }
    virtual const char *GetName() const {
        return "MPetsc";
    }

};
