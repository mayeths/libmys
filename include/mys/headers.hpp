/**
 * @file headers.h
 * @author mayeths (wow@mayeths.com)
 * @version 1.0
 * @brief Basic C++ header of libmys. (Require C++11)
 * 
 * Includes necessary C++ headers here.
 * No other *.h and *.hpp headers in libmys should be include here except headers.h.
 */
#pragma once

#if __cplusplus < 201103L
#error Require at least c++11 to parse *.hpp in libmys
#endif

/* C headers */
#include "headers.h"

/* C++ headers */
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
