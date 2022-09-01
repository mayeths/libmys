#ifndef GRAPES_SEQ_STRUCT_MV_HPP
#define GRAPES_SEQ_STRUCT_MV_HPP

#include "common.hpp"

template<typename idx_t, typename data_t>
class seq_structVector {
public:
    idx_t local_x;// lon方向的格点数(仅含计算区域)
    idx_t local_y;// lat方向的格点数(仅含计算区域)
    idx_t local_z;// 垂直方向的格点数(仅含计算区域)
    idx_t halo_x;// lon方向的halo区宽度
    idx_t halo_y;// lat方向的halo区宽度
    idx_t halo_z;// 垂直方向的halo区宽度
    data_t * data;

    // 数据存储顺序从内到外为(k, j, i)
    idx_t slice_k_size;
    idx_t slice_ki_size;

    seq_structVector(idx_t lx, idx_t ly, idx_t lz, idx_t hx, idx_t hy, idx_t hz);
    // 拷贝构造函数，开辟同样规格的data
    seq_structVector(const seq_structVector & model);
    ~seq_structVector();

    void init_debug(idx_t off_x, idx_t off_y, idx_t off_z);
    void print_level(idx_t ilev);

    void operator=(data_t val);
    void set_halo(data_t val);
};

template<typename idx_t, typename data_t>
class seq_structMatrix {
public:
    idx_t num_diag;// 矩阵对角线数（19）
    idx_t local_x;// lon方向的格点数(仅含计算区域)
    idx_t local_y;// lat方向的格点数(仅含计算区域)
    idx_t local_z;// 垂直方向的格点数(仅含计算区域)
    idx_t halo_x;// lon方向的halo区宽度
    idx_t halo_y;// lat方向的halo区宽度
    idx_t halo_z;// 垂直方向的halo区宽度
    data_t * data;

    // 数据存储顺序从内到外为(diag, k, j, i)
    idx_t slice_dk_size;
    idx_t slice_dki_size;

    seq_structMatrix(idx_t num_d, idx_t lx, idx_t ly, idx_t lz, idx_t hx, idx_t hy, idx_t hz);
    // 拷贝构造函数，开辟同样规格的data
    seq_structMatrix(const seq_structMatrix & model);
    ~seq_structMatrix();

    void init_debug(idx_t off_x, idx_t off_y, idx_t off_z);
    void print_level_diag(idx_t ilev, idx_t idiag);

    void extract_diag(idx_t idx_diag) const;

    void Mult(const seq_structVector<idx_t, data_t> & x, seq_structVector<idx_t, data_t> & y) const;
};

/*
 * * * * * seq_structVetor * * * * * 
 */

template<typename idx_t, typename data_t>
seq_structVector<idx_t, data_t>::seq_structVector(idx_t lx, idx_t ly, idx_t lz, idx_t hx, idx_t hy, idx_t hz)
    : local_x(lx), local_y(ly), local_z(lz), halo_x(hx), halo_y(hy), halo_z(hz)
{
    idx_t   tot_x = local_x + 2 * halo_x,
            tot_y = local_y + 2 * halo_y,
            tot_z = local_z + 2 * halo_z;
    data = new data_t[tot_x * tot_y * tot_z];
#ifdef DEBUG
    for (idx_t i = 0; i < tot_x * tot_y * tot_z; i++) data[i] = -99;
#endif

    slice_k_size  = local_z + 2 * halo_z;
    slice_ki_size = slice_k_size * (local_x + 2 * halo_x); 
}

template<typename idx_t, typename data_t>
seq_structVector<idx_t, data_t>::seq_structVector(const seq_structVector & model)
    : local_x(model.local_x), local_y(model.local_y), local_z(model.local_z),
      halo_x (model.halo_x) , halo_y (model.halo_y ), halo_z (model.halo_z) , 
      slice_k_size(model.slice_k_size), slice_ki_size(model.slice_ki_size)
{
    idx_t   tot_x = local_x + 2 * halo_x,
            tot_y = local_y + 2 * halo_y,
            tot_z = local_z + 2 * halo_z;
    data = new data_t[tot_x * tot_y * tot_z];
}

template<typename idx_t, typename data_t>
seq_structVector<idx_t, data_t>::~seq_structVector() {
    delete data;
}

template<typename idx_t, typename data_t>
void seq_structVector<idx_t, data_t>::init_debug(idx_t off_x, idx_t off_y, idx_t off_z) 
{
    idx_t tot = slice_ki_size * (local_y + 2 * halo_y);
    for (idx_t i = 0; i < tot; i++)
        data[i] = 0.0;

    idx_t xbeg = halo_x, xend = xbeg + local_x,
            ybeg = halo_y, yend = ybeg + local_y,
            zbeg = halo_z, zend = zbeg + local_z;
    for (idx_t j = ybeg; j < yend; j++) {
        for (idx_t i = xbeg; i < xend; i++) {
            for (idx_t k = zbeg; k < zend; k++) {
                data[k + i * slice_k_size + j * slice_ki_size] 
                    = 100.0 * (off_x + i - xbeg) + off_y + j - ybeg + 1e-2 * (off_z + k - zbeg);
            }
        }
    }
}

template<typename idx_t, typename data_t>
void seq_structVector<idx_t, data_t>::print_level(idx_t ilev) 
{
    assert(ilev >= 0 && ilev < local_z);
    idx_t xbeg = 0, xend = xbeg + local_x + 2 * halo_x,
            ybeg = 0, yend = ybeg + local_y + 2 * halo_y;
    printf("lev %d: \n", ilev);
    for (idx_t j = ybeg; j < yend; j++) {
        for (idx_t i = xbeg; i < xend; i++) {
            printf("%12.7e ", data[ilev + i * slice_k_size + j * slice_ki_size]);
        }
        printf("\n");
    }
}

template<typename idx_t, typename data_t>
void seq_vec_dot(const seq_structVector<idx_t, data_t> & x, 
             const seq_structVector<idx_t, data_t> & y,
             void * res, int size_of_res) {
    CHECK_LOCAL_HALO(x, y);

    const idx_t xbeg = x.halo_x, xend = xbeg + x.local_x,
                ybeg = x.halo_y, yend = ybeg + x.local_y,
                zbeg = x.halo_z, zend = zbeg + x.local_z;
    const idx_t slice_k_size = x.slice_k_size, slice_ki_size = x.slice_ki_size;

#define CALC_CODE \
    for (idx_t j = ybeg; j < yend; j++) \
        for (idx_t i = xbeg; i < xend; i++) \
            for (idx_t k = zbeg; k < zend; k++) { \
                idx_t loc = k + i * slice_k_size + j * slice_ki_size; \
                dot += x.data[loc] * y.data[loc]; \
            }

    if (size_of_res == 8) {
        double dot = 0.0;
        CALC_CODE
        *((double*)res) = dot;
    } else if (size_of_res == 4) {
        float dot = 0.0;
        CALC_CODE
        *((float*)res) = dot;
    } else {
        printf("INVALID dot type of %d bytes\n", size_of_res);
    }
#undef CALC_CODE
}

template<typename idx_t, typename data_t, typename scalar_t>
void seq_vec_add(const seq_structVector<idx_t, data_t> & v1, scalar_t alpha, 
                 const seq_structVector<idx_t, data_t> & v2, seq_structVector<idx_t, data_t> & v) 
{
    CHECK_LOCAL_HALO(v1, v2);
    CHECK_LOCAL_HALO(v1, v );
    
    const data_t * v1_data = v1.data;
    const data_t * v2_data = v2.data;
          data_t * res_data = v.data;
    
    const idx_t ibeg = v1.halo_x, iend = ibeg + v1.local_x,
                jbeg = v1.halo_y, jend = jbeg + v1.local_y,
                kbeg = v1.halo_z, kend = kbeg + v1.local_z;
    const idx_t vec_k_size = v1.slice_k_size, vec_ki_size = v1.slice_ki_size;
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
            for (idx_t k = kbeg; k < kend; k++) {
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
                res_data[VECIDX(k, i, j)] = v1_data[VECIDX(k, i, j)] + alpha * v2_data[VECIDX(k, i, j)];
#undef VECIDX
            }
        }
    }
}

template<typename idx_t, typename data_t>
void seq_vec_copy(const seq_structVector<idx_t, data_t> & src, seq_structVector<idx_t, data_t> & dst)
{
    CHECK_LOCAL_HALO(src, dst);
    
    const data_t * src_data = src.data;
          data_t * dst_data = dst.data;
    const idx_t ibeg = src.halo_x, iend = ibeg + src.local_x,
                jbeg = src.halo_y, jend = jbeg + src.local_y,
                kbeg = src.halo_z, kend = kbeg + src.local_z;
    const idx_t vec_k_size = src.slice_k_size, vec_ki_size = src.slice_ki_size;
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
            for (idx_t k = kbeg; k < kend; k++) {
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
                dst_data[VECIDX(k, i, j)] = src_data[VECIDX(k, i, j)];
#undef VECIDX
            }
        }
    }
}

template<typename idx_t, typename data_t>
void seq_structVector<idx_t, data_t>::operator=(data_t val) {
    idx_t   xbeg = halo_x, xend = xbeg + local_x,
            ybeg = halo_y, yend = ybeg + local_y,
            zbeg = halo_z, zend = zbeg + local_z;
    // TODO: 可以优化不必每一次在最内维循环都计算loc
    for (idx_t j = ybeg; j < yend; j++)
        for (idx_t i = xbeg; i < xend; i++)
            for (idx_t k = zbeg; k < zend; k++) {
                idx_t loc = k + i * slice_k_size + j * slice_ki_size;
                data[loc] = val;
            }
}

template<typename idx_t, typename data_t>
void seq_structVector<idx_t, data_t>::set_halo(data_t val) {
    for (idx_t j = 0; j < halo_y; j++) {
        for (idx_t i = 0; i < halo_x * 2 + local_x; i++)
            for (idx_t k = 0; k < halo_z * 2 + local_z; k++)
                data[k + i * slice_k_size + j * slice_ki_size] = val;
    }

    for (idx_t j = halo_y; j < halo_y + local_y; j++) {
        for (idx_t i = 0; i < halo_x; i++) 
            for (idx_t k = 0; k < halo_z * 2 + local_z; k++)
                data[k + i * slice_k_size + j * slice_ki_size] = val;
        
        for (idx_t i = halo_x; i < halo_x + local_x; i++) {
            for (idx_t k = 0; k < halo_z; k++)
                data[k + i * slice_k_size + j * slice_ki_size] = val;
            for (idx_t k = halo_z + local_z; k < halo_z * 2 + local_z; k++)
                data[k + i * slice_k_size + j * slice_ki_size] = val;
        }

        for (idx_t i = halo_x + local_x; i < halo_x * 2 + local_x; i++) {
            for (idx_t k = 0; k < halo_z * 2 + local_z; k++)
                data[k + i * slice_k_size + j * slice_ki_size] = val;
        }
    }

    for (idx_t j = halo_y + local_y; j < halo_y * 2 + local_y; j++) {
        for (idx_t i = 0; i < halo_x * 2 + local_x; i++)
            for (idx_t k = 0; k < halo_z * 2 + local_z; k++)
                data[k + i * slice_k_size + j * slice_ki_size] = val;
    }

#ifdef DEBUG
    for (idx_t j = 0; j < halo_y * 2 + local_y; j++)
        for (idx_t i = 0; i < halo_x * 2 + local_x; i++)
            for (idx_t k = 0; k < halo_z * 2 + local_z; k++)
                if (data[k + i * slice_k_size + j * slice_ki_size] != 0.0) {
                    printf("%d %d %d %.5e\n", j, i, k, data[k + i * slice_k_size + j * slice_ki_size]);
                }
#endif
}

template<typename idx_t, typename data_t, typename scalar_t>
void seq_vec_mul_by_scalar(const scalar_t coeff, const seq_structVector<idx_t, data_t> & src, seq_structVector<idx_t, data_t> & dst) 
{
    CHECK_LOCAL_HALO(src, dst);
    
    const data_t * src_data = src.data;
          data_t * dst_data = dst.data;
    const idx_t ibeg = src.halo_x, iend = ibeg + src.local_x,
                jbeg = src.halo_y, jend = jbeg + src.local_y,
                kbeg = src.halo_z, kend = kbeg + src.local_z;
    const idx_t vec_k_size = src.slice_k_size, vec_ki_size = src.slice_ki_size;
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
            for (idx_t k = kbeg; k < kend; k++) {
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
                dst_data[VECIDX(k, i, j)] = coeff * src_data[VECIDX(k, i, j)];
#undef VECIDX
            }
        }
    }
}

template<typename idx_t, typename data_t, typename scalar_t>
void seq_vec_scale(const scalar_t coeff, seq_structVector<idx_t, data_t> & vec) {
    
    data_t * data = vec.data;
    const idx_t ibeg = vec.halo_x, iend = ibeg + vec.local_x,
                jbeg = vec.halo_y, jend = jbeg + vec.local_y,
                kbeg = vec.halo_z, kend = kbeg + vec.local_z;
    const idx_t vec_k_size = vec.slice_k_size, vec_ki_size = vec.slice_ki_size;
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
            for (idx_t k = kbeg; k < kend; k++) {
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
                data[VECIDX(k, i, j)] *= coeff;
#undef VECIDX
            }
        }
    }
}

/*
 * * * * * seq_structMatrix * * * * * 
 */

template<typename idx_t, typename data_t>
seq_structMatrix<idx_t, data_t>::seq_structMatrix(idx_t num_d, idx_t lx, idx_t ly, idx_t lz, idx_t hx, idx_t hy, idx_t hz)
    : num_diag(num_d), local_x(lx), local_y(ly), local_z(lz), halo_x(hx), halo_y(hy), halo_z(hz)
{
    idx_t tot = num_diag * (local_x + 2 * halo_x) * (local_y + 2 * halo_y) * (local_z + 2 * halo_z);
    data = new data_t[tot];
#ifdef DEBUG
    for (idx_t i = 0; i < tot; i++) data[i] = -9999.9;
#endif

    slice_dk_size  = num_diag * (local_z + 2 * halo_z);
    slice_dki_size = slice_dk_size * (local_x + 2 * halo_x);
}

template<typename idx_t, typename data_t>
seq_structMatrix<idx_t, data_t>::seq_structMatrix(const seq_structMatrix & model)
    : num_diag(model.num_diag),
      local_x(model.local_x), local_y(model.local_y), local_z(model.local_z),
      halo_x (model.halo_x) , halo_y (model.halo_y ), halo_z (model.halo_z) , 
      slice_dk_size(model.slice_dk_size), slice_dki_size(model.slice_dki_size)
{
    idx_t tot = num_diag * (local_x + 2 * halo_x) * (local_y + 2 * halo_y) * (local_z + 2 * halo_z);
    data = new data_t[tot];
}

template<typename idx_t, typename data_t>
seq_structMatrix<idx_t, data_t>::~seq_structMatrix() {
    delete data;
}

#ifdef DEBUG
template<typename idx_t, typename data_t>
void seq_structMatrix<idx_t, data_t>::init_debug(idx_t off_x, idx_t off_y, idx_t off_z) 
{
    idx_t tot = slice_dki_size * (local_y + 2 * halo_y);
    for (idx_t i = 0; i < tot; i++)
        data[i] = 0.0;

    idx_t   xbeg = halo_x, xend = xbeg + local_x,
            ybeg = halo_y, yend = ybeg + local_y,
            zbeg = halo_z, zend = zbeg + local_z;
    for (idx_t j = ybeg; j < yend; j++) {
        for (idx_t i = xbeg; i < xend; i++) {
            for (idx_t k = zbeg; k < zend; k++)
                for (idx_t d = 0; d < num_diag; d++) {
                    data[d + k * num_diag + i * slice_dk_size + j * slice_dki_size] 
                        = 100.0 * (off_x + i - xbeg) + off_y + j - ybeg + 1e-2 * (off_z + k - zbeg) + 1e-4 * d;
            }
        }
    }
}
#endif

template<typename idx_t, typename data_t>
void seq_structMatrix<idx_t, data_t>::print_level_diag(idx_t ilev, idx_t idiag) 
{
    assert(ilev >= 0 && ilev < local_z && idiag >= 0 && idiag < num_diag);
    idx_t   xbeg = 0, xend = xbeg + local_x + 2 * halo_x,
            ybeg = 0, yend = ybeg + local_y + 2 * halo_y;
    printf("lev %d with %d-th diag: \n", ilev, idiag);
    for (idx_t j = ybeg; j < yend; j++) {
        for (idx_t i = xbeg; i < xend; i++) {
            printf("%12.6f ", data[idiag + ilev * num_diag + i * slice_dk_size + j * slice_dki_size]);
        }
        printf("\n");
    }
}

template<typename idx_t, typename data_t>
void seq_structMatrix<idx_t, data_t>::Mult(const seq_structVector<idx_t, data_t> & x, seq_structVector<idx_t, data_t> & y) const
{
    CHECK_LOCAL_HALO(*this, x);
    CHECK_LOCAL_HALO(x , y);

    const data_t * A = data;
    const data_t * x_data = x.data;
    data_t * y_data = y.data;

    idx_t   ibeg = halo_x, iend = ibeg + local_x,
            jbeg = halo_y, jend = jbeg + local_y,
            kbeg = halo_z, kend = kbeg + local_z;
    /*  
            /-------18------/                    
          15 |     14     16|                   
         /---|---17------/  |                  
        |    |           |  |                 
        |    8------4----|--5                 
        |  1 |     0     |2 |       z   y    
        7----|---3-------6  |       ^  ^       
        |    |           |  |       | /     
        |    /------13---|--/       |/       
        |  10      9     |11        O-------> x  
        |/-------12------|/   
     *
     */

    idx_t vec_k_size = x.slice_k_size, vec_ki_size = x.slice_ki_size;
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
#define MATIDX(d, k, i, j) (d) + (k) * num_diag + (i) * slice_dk_size + (j) * slice_dki_size
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
            // bottom (Omit 5 neighbour points bellow it)
            {
                idx_t k = kbeg;
                y_data[VECIDX(k  , i  , j  )] = 
                  + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
                  + A[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  + A[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + A[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  + A[MATIDX( 4, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  + A[MATIDX( 5, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + A[MATIDX( 6, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + A[MATIDX( 7, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  + A[MATIDX( 8, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                //   + A[MATIDX( 9, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]
                //   + A[MATIDX(10, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                //   + A[MATIDX(11, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                //   + A[MATIDX(12, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                //   + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)]
                  + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                  + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];
            }

            // middle
            for (idx_t k = kbeg + 1; k < kend - 1; k++) {
                idx_t loc = k + i * vec_k_size + j * vec_ki_size;
                y_data[VECIDX(k  , i  , j  )] = 
                  + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
                  + A[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  + A[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + A[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  + A[MATIDX( 4, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  + A[MATIDX( 5, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + A[MATIDX( 6, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + A[MATIDX( 7, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  + A[MATIDX( 8, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                  + A[MATIDX( 9, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]
                  + A[MATIDX(10, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  + A[MATIDX(11, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + A[MATIDX(12, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)]
                  + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                  + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];
            }

            // top (Omit 5 neighbour points above it)
            {
                idx_t k = kend - 1;
                y_data[VECIDX(k  , i  , j  )] = 
                  + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
                  + A[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  + A[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + A[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  + A[MATIDX( 4, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  + A[MATIDX( 5, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + A[MATIDX( 6, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + A[MATIDX( 7, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  + A[MATIDX( 8, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                  + A[MATIDX( 9, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]
                  + A[MATIDX(10, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  + A[MATIDX(11, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + A[MATIDX(12, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)];
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                //   + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                //   + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                //   + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                //   + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];
            }
#undef MATIDX
#undef VECIDX
        }
    }

}

#endif