/* https://gist.github.com/2b-t/50d85115db8b12ed263f8231abf07fa2?permalink_comment_id=4274936#gistcomment-4274936 */
#pragma once

#include <type_traits>
#include <complex>
#include <map>
#include <string>

#include <mpi.h>

#include "_config.h"
#include "mpi.h"
#include "macro.h"

/**
 * @brief Return corresponding MPI_Datatype of given type
 * 
 * @details
 * This function is replaced by constant MPI_Datatype with -O3 optimization.
 * See https://rookiehpc.github.io/mpi/docs/mpi_datatype/index.html
 * for datatype and other constants in C (Click FORTRAN-2008 for Fortran).
 * 
 * @tparam T type
 * @return MPI_Datatype of T
 * 
 */
template <typename T>
static MPI_Datatype MPI_TYPE() noexcept
{
  using std::is_same;
  if (is_same<T, char>::value) return MPI_CHAR;
  if (is_same<T, signed char>::value) return MPI_SIGNED_CHAR;
  if (is_same<T, unsigned char>::value) return MPI_UNSIGNED_CHAR;
  if (is_same<T, wchar_t>::value) return MPI_WCHAR;
  if (is_same<T, signed short>::value) return MPI_SHORT;
  if (is_same<T, unsigned short>::value) return MPI_UNSIGNED_SHORT;
  if (is_same<T, signed int>::value) return MPI_INT;
  if (is_same<T, unsigned int>::value) return MPI_UNSIGNED;
  if (is_same<T, signed long int>::value) return MPI_LONG;
  if (is_same<T, unsigned long int>::value) return MPI_UNSIGNED_LONG;
  if (is_same<T, signed long long int>::value) return MPI_LONG_LONG;
  if (is_same<T, unsigned long long int>::value) return MPI_UNSIGNED_LONG_LONG;
  if (is_same<T, float>::value) return MPI_FLOAT;
  if (is_same<T, double>::value) return MPI_DOUBLE;
  if (is_same<T, long double>::value) return MPI_LONG_DOUBLE;
  if (is_same<T, std::int8_t>::value) return MPI_INT8_T;
  if (is_same<T, std::int16_t>::value) return MPI_INT16_T;
  if (is_same<T, std::int32_t>::value) return MPI_INT32_T;
  if (is_same<T, std::int64_t>::value) return MPI_INT64_T;
  if (is_same<T, std::uint8_t>::value) return MPI_UINT8_T;
  if (is_same<T, std::uint16_t>::value) return MPI_UINT16_T;
  if (is_same<T, std::uint32_t>::value) return MPI_UINT32_T;
  if (is_same<T, std::uint64_t>::value) return MPI_UINT64_T;
  if (is_same<T, bool>::value) return MPI_C_BOOL;
  if (is_same<T, std::complex<float>>::value) return MPI_C_COMPLEX;
  if (is_same<T, std::complex<double>>::value) return MPI_C_DOUBLE_COMPLEX;
  if (is_same<T, std::complex<long double>>::value) return MPI_C_LONG_DOUBLE_COMPLEX;
#if defined(ARCH_X64)
  if (is_same<T, __float128>::value) {
    static MPI_Datatype float128_type = MPI_DATATYPE_NULL;
    if (float128_type == MPI_DATATYPE_NULL) {
        MPI_Type_contiguous(sizeof(__float128), MPI_BYTE, &float128_type);
        MPI_Type_commit(&float128_type);
    }
    return float128_type;
  }
#endif
}

_MYS_UNUSED static std::string MPI_TYPENAME(MPI_Datatype dtype) noexcept 
{
    static std::map<MPI_Datatype, std::string> mpi_typename {
        // https://rookiehpc.github.io/mpi/docs/mpi_datatype/index.html
        // C
        {MPI_SIGNED_CHAR, "MPI_SIGNED_CHAR"},
        {MPI_UNSIGNED_CHAR, "MPI_UNSIGNED_CHAR"},
        {MPI_SHORT, "MPI_SHORT"},
        {MPI_UNSIGNED_SHORT, "MPI_UNSIGNED_SHORT"},
        {MPI_INT, "MPI_INT"},
        {MPI_UNSIGNED, "MPI_UNSIGNED"},
        {MPI_LONG, "MPI_LONG"},
        {MPI_UNSIGNED_LONG, "MPI_UNSIGNED_LONG"},
        {MPI_LONG_LONG_INT, "MPI_LONG_LONG_INT"},
        {MPI_LONG_LONG, "MPI_LONG_LONG"},
        {MPI_UNSIGNED_LONG_LONG, "MPI_UNSIGNED_LONG_LONG"},
        {MPI_CHAR, "MPI_CHAR"},
        {MPI_WCHAR, "MPI_WCHAR"},
        {MPI_FLOAT, "MPI_FLOAT"},
        {MPI_DOUBLE, "MPI_DOUBLE"},
        {MPI_LONG_DOUBLE, "MPI_LONG_DOUBLE"},
        {MPI_INT8_T, "MPI_INT8_T"},
        {MPI_UINT8_T, "MPI_UINT8_T"},
        {MPI_INT16_T, "MPI_INT16_T"},
        {MPI_UINT16_T, "MPI_UINT16_T"},
        {MPI_INT32_T, "MPI_INT32_T"},
        {MPI_UINT32_T, "MPI_UINT32_T"},
        {MPI_INT64_T, "MPI_INT64_T"},
        {MPI_UINT64_T, "MPI_UINT64_T"},
        {MPI_C_BOOL, "MPI_C_BOOL"},
        {MPI_C_COMPLEX, "MPI_C_COMPLEX"},
        {MPI_C_FLOAT_COMPLEX, "MPI_C_FLOAT_COMPLEX"},
        {MPI_C_DOUBLE_COMPLEX, "MPI_C_DOUBLE_COMPLEX"},
        {MPI_C_LONG_DOUBLE_COMPLEX, "MPI_C_LONG_DOUBLE_COMPLEX"},
        {MPI_AINT, "MPI_AINT"},
        {MPI_COUNT, "MPI_COUNT"},
        {MPI_OFFSET, "MPI_OFFSET"},
        {MPI_BYTE, "MPI_BYTE"},
        {MPI_PACKED, "MPI_PACKED"},
        {MPI_SHORT_INT, "MPI_SHORT_INT"},
        {MPI_LONG_INT, "MPI_LONG_INT"},
        {MPI_FLOAT_INT, "MPI_FLOAT_INT"},
        {MPI_DOUBLE_INT, "MPI_DOUBLE_INT"},
        {MPI_LONG_DOUBLE_INT, "MPI_LONG_DOUBLE_INT"},
#ifdef MPI_2INT
        {MPI_2INT, "MPI_2INT"},
#endif
        // Fortran
        {MPI_INTEGER, "MPI_INTEGER"},
        {MPI_REAL, "MPI_REAL"},
        {MPI_DOUBLE_PRECISION, "MPI_DOUBLE_PRECISION"},
        {MPI_COMPLEX, "MPI_COMPLEX"},
        {MPI_LOGICAL, "MPI_LOGICAL"},
        {MPI_CHARACTER, "MPI_CHARACTER"},
#ifdef MPI_INTEGER1
        {MPI_INTEGER1, "MPI_INTEGER1"},
#endif
#ifdef MPI_INTEGER2
        {MPI_INTEGER2, "MPI_INTEGER2"},
#endif
#ifdef MPI_INTEGER4
        {MPI_INTEGER4, "MPI_INTEGER4"},
#endif
#ifdef MPI_INTEGER8
        {MPI_INTEGER8, "MPI_INTEGER8"},
#endif
#ifdef MPI_INTEGER16
        {MPI_INTEGER16, "MPI_INTEGER16"},
#endif
#ifdef MPI_REAL2
        {MPI_REAL2, "MPI_REAL2"},
#endif
#ifdef MPI_REAL4
        {MPI_REAL4, "MPI_REAL4"},
#endif
#ifdef MPI_REAL8
        {MPI_REAL8, "MPI_REAL8"},
#endif
#ifdef MPI_REAL16
        {MPI_REAL16, "MPI_REAL16"},
#endif
#ifdef MPI_COMPLEX4
        {MPI_COMPLEX4, "MPI_COMPLEX4"},
#endif
#ifdef MPI_COMPLEX8
        {MPI_COMPLEX8, "MPI_COMPLEX8"},
#endif
#ifdef MPI_COMPLEX16
        {MPI_COMPLEX16, "MPI_COMPLEX16"},
#endif
#ifdef MPI_COMPLEX32
        {MPI_COMPLEX32, "MPI_COMPLEX32"},
#endif
#ifdef MPI_2REAL
        {MPI_2REAL, "MPI_2REAL"},
#endif
#ifdef MPI_2DOUBLE_PRECISION
        {MPI_2DOUBLE_PRECISION, "MPI_2DOUBLE_PRECISION"},
#endif
#ifdef MPI_2INTEGER
        {MPI_2INTEGER, "MPI_2INTEGER"},
#endif
#ifdef MPI_ADDRESS_KIND
        {MPI_ADDRESS_KIND, "MPI_ADDRESS_KIND"},
#endif
#ifdef MPI_COUNT_KIND
        {MPI_COUNT_KIND, "MPI_COUNT_KIND"},
#endif
#ifdef MPI_OFFSET_KIND
        {MPI_OFFSET_KIND, "MPI_OFFSET_KIND"},
#endif
    };
    static size_t user_count = 0;
    try {
        return mpi_typename.at(dtype);
    } catch (std::out_of_range const &) {
        char name[128];
        snprintf(name, sizeof(name), "TYPE_%zu", user_count);
        user_count += 1;
        mpi_typename[dtype] = std::string(name);
        return std::string(name);
    }
}

_MYS_UNUSED static std::string MPI_COMMNAME(MPI_Comm comm) noexcept
{
    static std::map<MPI_Comm, std::string> mpi_commname {
        {MPI_COMM_NULL, "MPI_COMM_NULL"},
        {MPI_COMM_SELF, "MPI_COMM_SELF"},
        {MPI_COMM_WORLD, "MPI_COMM_WORLD"},
    };
    static size_t user_count = 0;
    try {
        return mpi_commname.at(comm);
    } catch (std::out_of_range const &) {
        char name[128];
        snprintf(name, sizeof(name), "COMM_%llu", (unsigned long long)user_count);
        user_count += 1;
        mpi_commname[comm] = std::string(name);
        return std::string(name);
    }
}

_MYS_UNUSED static std::string MPI_OPNAME(MPI_Op op) noexcept
{
    static std::map<MPI_Op, std::string> mpi_opname {
        {MPI_OP_NULL, "MPI_OP_NULL"},
        {MPI_MAX, "MPI_MAX"},
        {MPI_MIN, "MPI_MIN"},
        {MPI_SUM, "MPI_SUM"},
        {MPI_PROD, "MPI_PROD"},
        {MPI_LAND, "MPI_LAND"},
        {MPI_BAND, "MPI_BAND"},
        {MPI_LOR, "MPI_LOR"},
        {MPI_BOR, "MPI_BOR"},
        {MPI_LXOR, "MPI_LXOR"},
        {MPI_BXOR, "MPI_BXOR"},
        {MPI_MINLOC, "MPI_MINLOC"},
        {MPI_MAXLOC, "MPI_MAXLOC"},
        {MPI_REPLACE, "MPI_REPLACE"},
    };
    static size_t user_count = 0;
    try {
        return mpi_opname.at(op);
    } catch (std::out_of_range const &) {
        char name[128];
        snprintf(name, sizeof(name), "OP_%llu", (unsigned long long)user_count);
        user_count += 1;
        mpi_opname[op] = std::string(name);
        return std::string(name);
    }
}

_MYS_UNUSED static std::string MPI_REQUESTNAME(MPI_Request request) noexcept
{
    static std::map<MPI_Request, std::string> mpi_requestname {
        {MPI_REQUEST_NULL, "MPI_REQUEST_NULL"},
    };
    static size_t user_count = 0;
    try {
        return mpi_requestname.at(request);
    } catch (std::out_of_range const &) {
        char name[128];
        snprintf(name, sizeof(name), "REQUEST_%llu", (unsigned long long)user_count);
        user_count += 1;
        mpi_requestname[request] = std::string(name);
        return std::string(name);
    }
}
