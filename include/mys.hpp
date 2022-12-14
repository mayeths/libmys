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

#if __cplusplus < 201103L
#error Require at least c++11 to parse *.hpp in libmys
#endif

/* Primary Library (mys) */

#include "mys.h"
#include "mys/linalg.hpp"
#include "mys/memory.hpp"
#ifndef MYS_NO_MPI
#include "mys/mpi.hpp"
#endif
#include "mys/raii.hpp"
#include "mys/string.hpp"
#include "mys/type.hpp"
