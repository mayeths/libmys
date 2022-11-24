/* https://gist.github.com/2b-t/50d85115db8b12ed263f8231abf07fa2?permalink_comment_id=4274936#gistcomment-4274936 */
#pragma once

#include <type_traits>
#include <complex>
#include <map>

#if !defined(MPI_VERSION)
#if !defined(MYS_NO_MPI)
#error Require <mpi.h> or macro MYS_NO_MPI
#else

/* TODO: Void mpi functions here if not presented.
   Other files rely on MPI should include this file first. */

#endif
#endif

#include "config.h"

/**
 * @brief Return corresponding MPI_Datatype of given type
 * 
 * @details
 * This function will be replaced by constant MPI_Datatype in -O3 optimization.
 * 
 * @tparam T type
 * @return MPI_Datatype of T
 * 
 */
template <typename T>
static inline MPI_Datatype MPI_TYPE() noexcept {
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

static const std::map<MPI_Datatype, size_t> __mpi_size_mapper {
  {MPI_CHAR, sizeof(char)},
  {MPI_SIGNED_CHAR, sizeof(signed char)},
  {MPI_UNSIGNED_CHAR, sizeof(unsigned char)},
  {MPI_WCHAR, sizeof(wchar_t)},
  {MPI_SHORT, sizeof(signed short)},
  {MPI_UNSIGNED_SHORT, sizeof(unsigned short)},
  {MPI_INT, sizeof(signed int)},
  {MPI_UNSIGNED, sizeof(unsigned int)},
  {MPI_LONG, sizeof(signed long int)},
  {MPI_UNSIGNED_LONG, sizeof(unsigned long int)},
  {MPI_LONG_LONG, sizeof(signed long long int)},
  {MPI_UNSIGNED_LONG_LONG, sizeof(unsigned long long int)},
  {MPI_FLOAT, sizeof(float)},
  {MPI_DOUBLE, sizeof(double)},
  {MPI_LONG_DOUBLE, sizeof(long double)},
  {MPI_INT8_T, sizeof(std::int8_t)},
  {MPI_INT16_T, sizeof(std::int16_t)},
  {MPI_INT32_T, sizeof(std::int32_t)},
  {MPI_INT64_T, sizeof(std::int64_t)},
  {MPI_UINT8_T, sizeof(std::uint8_t)},
  {MPI_UINT16_T, sizeof(std::uint16_t)},
  {MPI_UINT32_T, sizeof(std::uint32_t)},
  {MPI_UINT64_T, sizeof(std::uint64_t)},
  {MPI_C_BOOL, sizeof(bool)},
  {MPI_C_COMPLEX, sizeof(std::complex<float)},
  {MPI_C_DOUBLE_COMPLEX, sizeof(std::complex<double)},
  {MPI_C_LONG_DOUBLE_COMPLEX, sizeof(std::complex<long double)},
};

/**
 * @brief Return corresponding size of MPI_Datatype
 * 
 * @details
 * This function will be replaced by constant size_t in -O3 optimization.
 * 
 * @param D MPI_Datatype
 * @return size of D
 * 
 */
static inline size_t MPI_SIZE(MPI_Datatype D) noexcept {
  return __mpi_size_mapper[D];
}
