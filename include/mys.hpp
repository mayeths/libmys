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
#include "mys/allocator.hpp"
#include "mys/blas.hpp"
#include "mys/mat.hpp"
#include "mys/sor.hpp"
#include "mys/util.hpp"
#include "mys/vec.hpp"
