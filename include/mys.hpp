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

#if __cplusplus < 201103L
#error Require at least c++11 to parse *.hpp in libmys
#endif

#include "mys.h"

#include "mys/headers.hpp"
#include "mys/mat.hpp"
#include "mys/memory.hpp"
#include "mys/mpi.hpp"
#include "mys/sor.hpp"
#include "mys/string.hpp"
#include "mys/util.hpp"
#include "mys/vec.hpp"

/*************************/
// myss
/*************************/
#ifndef MYS_NO_MYSS

#include "myss/vec/VBase.hpp"
#include "myss/vec/VDense.hpp"
#ifdef PETSC_DIR
#include "myss/vec/VPetsc.hpp"
#endif /*PETSC_DIR*/

#include "myss/mat/MBase.hpp"
#ifdef PETSC_DIR
#include "myss/mat/MPetsc.hpp"
#endif /*PETSC_DIR*/

#include "myss/pc/PCBase.hpp"
#include "myss/pc/PCNone.hpp"
#include "myss/pc/PCJacobi.hpp"
#ifdef PETSC_DIR
#include "myss/pc/PCPetsc.hpp"
#endif /*PETSC_DIR*/

#include "myss/iss/ISSBase.hpp"
#include "myss/iss/CG.hpp"
#include "myss/iss/PIPECG.hpp"

#endif /*MYS_NO_MYSS*/



/*************************/
// library
/*************************/
#ifdef MYS_USE_LIBRARY

#ifndef MYS_NO_LIBFMT
#include "mys-thirdparty/fmt/core.h"
// #include "mys-thirdparty/fmt/chrono.h" /* std::literals::chrono_literals requires C++14 */
#include "mys-thirdparty/fmt/ranges.h"
#include "mys-thirdparty/fmt/os.h"
#include "mys-thirdparty/fmt/color.h"
#endif /*MYS_NO_LIBFMT*/

#endif /*MYS_HEADER_ONLY*/
