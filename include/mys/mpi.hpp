/* https://gist.github.com/2b-t/50d85115db8b12ed263f8231abf07fa2?permalink_comment_id=4274936#gistcomment-4274936 */
#pragma once

#include <type_traits>
#include <complex>
#include <mpi.h>

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
#if defined(__x86_64__) || defined(__i386__)
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

