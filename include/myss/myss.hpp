
#include "mys.hpp"

/* Sparse Solver Library (myss) */

#ifndef MYS_NO_MYSS
#include "./vec/VBase.hpp"
#include "./vec/VDense.hpp"
#ifdef PETSC_DIR
#include "./vec/VPetsc.hpp"
#endif /*PETSC_DIR*/

#include "./mat/MBase.hpp"
#ifdef PETSC_DIR
#include "./mat/MPetsc.hpp"
#endif /*PETSC_DIR*/

#include "./pc/PCBase.hpp"
#include "./pc/PCNone.hpp"
#include "./pc/PCJacobi.hpp"
#ifdef PETSC_DIR
#include "./pc/PCPetsc.hpp"
#endif /*PETSC_DIR*/

#include "./iss/ISSBase.hpp"
#include "./iss/CG.hpp"
#include "./iss/PIPECG.hpp"
#endif /*MYS_NO_MYSS*/
