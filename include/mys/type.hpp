#pragma once

#include <complex>
#include <cstdio>
#include <cinttypes>

enum class TypeNameType: int {
    C, /* unsigned long long int (typename in C) */
    Rust, /* u64 (typename in Rust) */
    CPP, /* uint64_t (typename in CPP) */
};

template <typename T, TypeNameType type = TypeNameType::C>
static inline const char * TYPENAME() noexcept
{
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
    if (is_same<T, std::complex<float>>::value) return "std::complex<float>";
    if (is_same<T, std::complex<double>>::value) return "std::complex<double>";
    if (is_same<T, std::complex<long double>>::value) return "std::complex<long double>";
    #if defined(__x86_64__) || defined(__i386__)
    if (is_same<T, __float128>::value) return "__float128";
    #endif
  } else if (type == TypeNameType::Rust) {
  } else if (type == TypeNameType::CPP) {
  }
}

template <typename T>
static inline const char *_fmtspec() noexcept
{
  using std::is_same;
  if (is_same<T, char>::value) return "%c";
  if (is_same<T, char *>::value) return "%s";
  if (is_same<T, int>::value) return "%d";
  if (is_same<T, long>::value) return "%ld";
  if (is_same<T, long long>::value) return "%lld";
  if (is_same<T, unsigned int>::value) return "%u";
  if (is_same<T, unsigned long>::value) return "%lu";
  if (is_same<T, unsigned long long>::value) return "%llu";
  if (is_same<T, float>::value) return "%f";
  if (is_same<T, double>::value) return "%lf";
  return "<unsupported type>";
}
