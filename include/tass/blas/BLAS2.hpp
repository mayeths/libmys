#pragma once

#include "../matrix/MAbstract.hpp"
#include "../matrix/MP.hpp"

#include "../vector/VAbstract.hpp"
#include "../vector/VD.hpp"
#include "../vector/VP.hpp"


namespace BLAS2
{
    static void mult(const MP &mat, const VP &x, VP &y) {
        PetscErrorCode ierr;
        ierr = MatMult(mat.mat, *x.vec, *y.vec); CHKERRV(ierr);
    }
};
