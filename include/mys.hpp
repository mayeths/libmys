/**
 * @file mys.hpp
 * @author mayeths (wow@mayeths.com)
 * @version 1.0
 * @brief Include all libmys C/C++ headers into one header (Require C++11)
 * 
 * Extend the C headers by functionalities of C++
 * The difference between *.hpp and *.h version:
 *   - *.hpp extend *.h to provide more user-friendly interfaces
 *   - *.hpp use C++ class to declare useful aux structure (like CSR and vector)
 */
#pragma once

#include "mys.h"

#include "mys/headers.hpp"
#include "mys/mat.hpp"
#include "mys/memory.hpp"
#include "mys/mpi.hpp"
#include "mys/sor.hpp"
#include "mys/util.hpp"
#include "mys/vec.hpp"

#include "myss/vec/VAbstract.hpp"
#include "myss/vec/VDense.hpp"
#ifdef PETSC_DIR
#include "myss/vec/VPetsc.hpp"
#endif /*PETSC_DIR*/

#include "myss/mat/MAbstract.hpp"
#ifdef PETSC_DIR
#include "myss/mat/MPetsc.hpp"
#endif /*PETSC_DIR*/

#include "myss/pc/PCAbstract.hpp"
#include "myss/pc/PCNone.hpp"
#ifdef PETSC_DIR
#include "myss/pc/PCPetsc.hpp"
#include "myss/pc/PCPetscHpss.hpp"
#endif /*PETSC_DIR*/

#include "myss/iss/ISSAbstract.hpp"
#include "myss/iss/CG.hpp"
#include "myss/iss/PIPECG.hpp"
