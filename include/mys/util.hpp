#pragma once

template<class DATA_T, class FDATA_T = DATA_T>
static int WriteVector(const char *fname, DATA_T *arr, int n) {
    FILE *fp = fopen(fname, "wb");
    ASSERT(fp != NULL, "Failed to open %s", fname);
    for (int i = 0; i < n; i++) {
        FDATA_T tmp = static_cast<FDATA_T>(arr[i]);
        fwrite(&tmp, sizeof(FDATA_T), 1, fp);
    }
    {
        fclose(fp);
        printf("Saved %s with len %d and type %s\n", fname, n, typeid(FDATA_T).name());
    }
    return 0;
}
template<class DATA_T, class FDATA_T = DATA_T>
static int ReadVector(const char *fname, DATA_T **arr_, int *n_) {
    FILE *fp = fopen(fname, "rb");
    ASSERT(fp != NULL, "Failed to open %s", fname);
    {
        fseek(fp, 0, SEEK_END);
        (*n_) = ftell(fp) / sizeof(FDATA_T);
        fseek(fp, 0, SEEK_SET);
        (*arr_) = (DATA_T *)calloc((*n_), sizeof(DATA_T));
    }
    for (int i = 0; i < (*n_); i++) {
        FDATA_T tmp;
        fread(&tmp, sizeof(FDATA_T), 1, fp);
        (*arr_)[i] = static_cast<DATA_T>(tmp);
    }
    {
        ASSERT(getc(fp) == EOF, "Expect %s meet EOF after read %d values", fname, (*n_));
        fclose(fp);
        printf("Loaded %s with len %d and type %s\n", fname, (*n_), typeid(FDATA_T).name());
    }
    return 0;
}

template<class INDEX_T, class DATA_T>
static DATA_T CalcResidual(INDEX_T nrow, INDEX_T *Ap, INDEX_T *Aj, DATA_T *Av, DATA_T *x, DATA_T *b) {
    std::vector<DATA_T> r(nrow);
    for (int i = 0; i < nrow; i++) {
        const int rowstart = Ap[i];
        const int rowend = Ap[i + 1];
        DATA_T acc = 0;
        for (int jj = rowstart; jj < rowend; jj++) {
            int j = Aj[jj];
            DATA_T v = Av[jj];
            acc += v * x[j];
        }
        r[i] = b[i] - acc;
    }
    DATA_T norm2 = 0;
    for (int i = 0; i < r.size(); i++) {
        norm2 += r[i] * r[i];
    }
    return sqrt(norm2);
}

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
