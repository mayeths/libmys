#pragma once

#include <complex>
#include <cstdio>
#include <cinttypes>

enum class TypeNameType: int {
    C, /* unsigned long long int (typename in C) */
    Rust, /* u64 (typename in Rust) */
    CPP, /* uint64_t (typename in ) */
};

template <typename T, TypeNameType type = TypeNameType::C>
static inline const char * TYPENAME() noexcept {
  using std::is_same;
  if (type == TypeNameType::C) {
    if (is_same<T, char>::value) return "char";
    if (is_same<T, signed char>::value) return "signed char";
    if (is_same<T, unsigned char>::value) return "unsigned char";
    if (is_same<T, wchar_t>::value) return "wchar_t";
    if (is_same<T, signed short>::value) return "signed short";
    if (is_same<T, unsigned short>::value) return "unsigned short";
    if (is_same<T, signed int>::value) return "signed int";
    if (is_same<T, unsigned int>::value) return "unsigned int";
    if (is_same<T, signed long int>::value) return "signed long int";
    if (is_same<T, unsigned long int>::value) return "unsigned long int";
    if (is_same<T, signed long long int>::value) return "signed long long int";
    if (is_same<T, unsigned long long int>::value) return "unsigned long long int";
    if (is_same<T, float>::value) return "float";
    if (is_same<T, double>::value) return "double";
    if (is_same<T, long double>::value) return "long double";
    if (is_same<T, std::int8_t>::value) return "std::int8_t";
    if (is_same<T, std::int16_t>::value) return "std::int16_t";
    if (is_same<T, std::int32_t>::value) return "std::int32_t";
    if (is_same<T, std::int64_t>::value) return "std::int64_t";
    if (is_same<T, std::uint8_t>::value) return "std::uint8_t";
    if (is_same<T, std::uint16_t>::value) return "std::uint16_t";
    if (is_same<T, std::uint32_t>::value) return "std::uint32_t";
    if (is_same<T, std::uint64_t>::value) return "std::uint64_t";
    if (is_same<T, bool>::value) return "bool";
    if (is_same<T, std::complex<float>>::value) return "std::complex<float";
    if (is_same<T, std::complex<double>>::value) return "std::complex<double";
    if (is_same<T, std::complex<long double>>::value) return "std::complex<long double";
    #if defined(__x86_64__) || defined(__i386__)
    if (is_same<T, __float128>::value) return "__float128";
    #endif
  } else if (type == TypeNameType::Rust) {
  } else if (type == TypeNameType::CPP) {
  }
}

template <typename T> static const char *_fmtspec = "<unsupported type>";
template <> static const char *_fmtspec<char> = "%c";
template <> static const char *_fmtspec<char *> = "%s";
template <> static const char *_fmtspec<int> = "%d";
template <> static const char *_fmtspec<long> = "%ld";
template <> static const char *_fmtspec<long long> = "%lld";
template <> static const char *_fmtspec<long long int> = "%lld";
template <> static const char *_fmtspec<long int> = "%ld";
template <> static const char *_fmtspec<unsigned int> = "%u";
template <> static const char *_fmtspec<unsigned long> = "%lu";
template <> static const char *_fmtspec<unsigned long int> = "%lu";
template <> static const char *_fmtspec<unsigned long long> = "%llu";
template <> static const char *_fmtspec<unsigned long long int> = "%llu";
template <> static const char *_fmtspec<float> = "%f";
template <> static const char *_fmtspec<double> = "%lf";

template <typename UIntValue> void printHex (UIntValue value) {
    printf(hexFormat<UIntValue>, value);
    printf("\n");
}
