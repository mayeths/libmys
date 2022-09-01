#ifndef GRAPES_COMMON_HPP
#define GRAPES_COMMON_HPP

#define KSP_BIT 32
#define PC_BIT 32

#if KSP_BIT==64
#define KSP_TYPE double
#define KSP_MPI_TYPE MPI_DOUBLE
#elif KSP_BIT==32
#define KSP_TYPE float
#define KSP_MPI_TYPE MPI_FLOAT
#endif

#if PC_BIT==64
#define PC_TYPE double 
#define PC_MPI_TYPE MPI_DOUBLE
#elif PC_BIT==32
#define PC_TYPE float
#define PC_MPI_TYPE MPI_FLOAT
#elif PC_BIT==16
#define PC_TYPE __fp16
#define PC_MPI_TYPE MPI_SHORT
#endif

#define IDX_TYPE int

// #define NDEBUG

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

enum NEIGHBOR_ID {EAST, WEST, SOUTH, NORTH, NUM_NEIGHBORS};

template<typename T>
bool check_power2(T num) {
    while (num > 1) {
        T new_num = num >> 1;
        if (num != new_num * 2) return false;
        num = new_num;
    }
    return true;
}

#define CHECK_LOCAL_HALO(x , y) \
    assert((x).local_x == (y).local_x && (x).local_y == (y).local_y && (x).local_z == (y).local_z && \
           (x).halo_x == (y).halo_x  &&  (x).halo_y == (y).halo_y  &&  (x).halo_z == (y).halo_z);


/* https://gist.github.com/2b-t/50d85115db8b12ed263f8231abf07fa2?permalink_comment_id=4274936#gistcomment-4274936 */
#include <type_traits>
#include <complex>
template <typename T>
static inline MPI_Datatype MPI_dtype() noexcept {
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
        static MPI_Datatype f128_dt = MPI_DATATYPE_NULL;
        if (f128_dt == MPI_DATATYPE_NULL) {
            MPI_Type_contiguous(sizeof(__float128), MPI_BYTE, &f128_dt);
            MPI_Type_commit(&f128_dt);
        }
        return f128_dt;
    }
#endif
}

#endif