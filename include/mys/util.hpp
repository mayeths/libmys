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
