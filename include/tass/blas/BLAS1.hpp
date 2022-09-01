#pragma once

#include "../vector/VAbstract.hpp"
#include "../vector/VD.hpp"
#include "../vector/VP.hpp"

namespace BLAS1
{
    static PetscScalar norm(const VP &x) {
        PetscErrorCode ierr;
        PetscScalar result;
        ierr = VecNorm(*x.vec, NORM_2, &result);
        return result;
    }

    static PetscScalar dot(const VP &x, const VP &y) {
        PetscErrorCode ierr;
        PetscScalar result;
        ierr = VecDot(*x.vec, *y.vec, &result);
        return result;
    }

    static void copy(const VP &x, VP &y) {
        PetscErrorCode ierr;
        ierr = VecCopy(*x.vec, *y.vec); CHKERRV(ierr);
    }

    /* y = alpha x + y */
    static void axpy(VP &y, const PetscScalar alpha, const VP &x) {
        PetscErrorCode ierr;
        ierr = VecAXPY(*y.vec, alpha, *x.vec); CHKERRV(ierr);
    }

    /* y = x + beta y */
    static void aypx(VP &y, const PetscScalar beta, const VP &x) {
        PetscErrorCode ierr;
        ierr = VecAYPX(*y.vec, beta, *x.vec); CHKERRV(ierr);
    }

    /* y = alpha x + beta y */
    static void axpby(VP &y, const PetscScalar alpha, const PetscScalar beta, const VP &x) {
        PetscErrorCode ierr;
        ierr = VecAXPBY(*y.vec, alpha, beta, *x.vec); CHKERRV(ierr);
    }

    /* w = alpha x + y */
    static void waxpy(VP &w, const PetscScalar alpha, const VP &x, const VP &y) {
        PetscErrorCode ierr;
        ierr = VecWAXPY(*w.vec, alpha, *x.vec, *y.vec); CHKERRV(ierr);
    }
};

