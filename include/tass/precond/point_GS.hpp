#ifndef GRAPES_POINT_GS_HPP
#define GRAPES_POINT_GS_HPP

#include "precond.hpp"
#include "../utils/par_struct_mat.hpp"

template<typename idx_t, typename data_t>
class PointGS : public Solver<idx_t, data_t> {
public:
    // when used as precondition, just do once, and zero-val optimization
    // could be exploited 
    bool used_as_precond = true;
    // 对称GS：0 for sym, 1 for forward, -1 backward
    SCAN_TYPE scan_type = SYMMETRIC;

    // weighted Jacobi
    mutable data_t weight = 1.0;

    // operator (often as matrix-A)
    const Operator<idx_t, data_t> * oper = nullptr;

    // separate diagonal values if for efficiency concern is needed
    // should only be used when separation is cheap
    mutable bool LDU_separated = false;
    mutable seq_structVector<idx_t, data_t> * D = nullptr;
    mutable seq_structMatrix<idx_t, data_t> * L = nullptr;
    mutable seq_structMatrix<idx_t, data_t> * U = nullptr;

    PointGS() : Solver<idx_t, data_t>(0, true) {  }

    // 按照x的规格进行构造结构化向量，并从oper中分离LDU
    void separate_LDU(const par_structVector<idx_t, data_t> & x) const ;

    virtual void SetOperator(const Operator<idx_t, data_t> & op) {
        oper = & op;

        this->input_dim[0] = op.input_dim[0];
        this->input_dim[1] = op.input_dim[1];
        this->input_dim[2] = op.input_dim[2];

        this->output_dim[0] = op.output_dim[0];
        this->output_dim[1] = op.output_dim[1];
        this->output_dim[2] = op.output_dim[2];
    }

    virtual void SetRelaxWeight(data_t wt) {weight = wt; }
    virtual void SetScanType(SCAN_TYPE type) {scan_type = type;}

    // 近似求解一个（残差）方程，以b为右端向量，返回x为近似解
    // 原方程 A*x = b，分解系数矩阵 A := D + L + U
    // Richardson迭代法：x_{k+1} = x_{k} + w*M^{-1}*(b - A*x_{k})
    // 按GS迭代法：前扫取M = 1/w*(D + w*L)，则有
    // x_{k+1} = x_{k} + w*(D+w*L)^{-1}*(b - (D+L+U)*x_{k})
    // (D+w*L)*x_{k+1} = (D+w*L)*x_{k} + w*(b - (D+L+U)*x_{k})
    // D*x_{k+1} = D*x_{k} + w*(b - (D+U)*x_{k} - L*x_{k+1})
    //   x_{k+1} =   x_{k} + w*D^{-1}*(b - (D+U)*x_{k} - L*x_{k+1})  也可以写成 
    //   x_{k+1} = (1-w)*x_{k} + w*D^{-1}*(b - U*x_{k} - L*x_{k+1})
    // 当对称GS时，后扫将L和U的位置互换
    //   x_{k+2} =   x_{k+1} + w*D^{-1}*(b - (D+L)*x_{k+1} - U*x_{k+2})
    virtual void Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;

    void ForwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;
    void BackwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;

    // 前扫用0初值时
    // D*x_{k+1} = w*(b - L*x_{k})
    void ForwardPass_ZERO(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;
    // 后扫用0初值时
    // D*x_{k+2} = w*(0 - D*x_{k+1} - U*x_{k+2}) + 2*D*x_{k+1}
    void BackwardPass_ZERO(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;


    virtual ~PointGS();
};

template<typename idx_t, typename data_t>
PointGS<idx_t, data_t>::~PointGS() {
    if (D != nullptr) delete D;
    if (L != nullptr) delete L;
    if (U != nullptr) delete U;
}

template<typename idx_t, typename data_t>
void PointGS<idx_t, data_t>::Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const {
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);

    if (used_as_precond) {
        // if (!LDU_separated) separate_LDU(x);

        switch (scan_type)
        {
        // TODO: 零初值优化，注意在对称GS中的后扫和单纯后扫的零初值优化是不一样的，小心！！！
        // 需要注意这个x传进来时可能halo区并不是0，如果不设置0并通信更新halo区，会导致使用错误的值，从而收敛性变慢
        // 还需要注意，这个x传进来时可能数据区不是0，如果不采用0初值的优化代码，而直接采用naive版本但是又没有数据区清零，则会用到旧值，从而收敛性变慢
        case SYMMETRIC:
            x.set_val(0.0);
            x.update_halo();
            ForwardPass(b, x);

            x.update_halo();
            BackwardPass(b, x);
            break;
        case FORWARD:
            x.set_val(0.0);
            x.update_halo();
            ForwardPass(b, x);
            break;
        case BACKWARD:
            x.set_val(0.0);
            x.update_halo();
            BackwardPass(b, x);
            break; 
        default:// do nothing, just act as an identity operator
            vec_copy(b, x);
            break;
        }
    }
    else {// 用作迭代算子
        MPI_Comm comm = b.comm_pkg->cart_comm;
        int my_pid;
        MPI_Comm_rank(comm, &my_pid);
        if (my_pid == 0)
            printf("Error: PointGS: usage of non-preconditioner not implemented\n");
        MPI_Abort(comm, -3000);
    }
}

#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
#define MATIDX(d, k, i, j) (d) + (k) * num_diag + (i) * slice_dk_size + (j) * slice_dki_size


template<typename idx_t, typename data_t>
void PointGS<idx_t, data_t>::ForwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
{
    const seq_structVector<idx_t, data_t> & b_vec = *(b.local_vector);
          seq_structVector<idx_t, data_t> & x_vec = *(x.local_vector);
    const seq_structMatrix<idx_t, data_t> & A_mat = *(((par_structMatrix<idx_t, data_t> *)(this->oper))->local_matrix);
    CHECK_LOCAL_HALO(x_vec, b_vec);
    CHECK_LOCAL_HALO(x_vec, A_mat);

    const data_t * b_data = b_vec.data;
          data_t * x_data = x_vec.data;
          data_t * A      = A_mat.data;

    const idx_t ibeg = b_vec.halo_x, iend = ibeg + b_vec.local_x,
                jbeg = b_vec.halo_y, jend = jbeg + b_vec.local_y,
                kbeg = b_vec.halo_z, kend = kbeg + b_vec.local_z;
    const idx_t vec_k_size = x_vec.slice_k_size, vec_ki_size = x_vec.slice_ki_size;
    const idx_t num_diag = A_mat.num_diag, slice_dk_size = A_mat.slice_dk_size, slice_dki_size = A_mat.slice_dki_size;

    const data_t one_minus_weight = 1.0 - weight;
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
            // bottom
            {
                idx_t k = kbeg;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
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
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// U*x_{k} + L*x_{k+1}
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - U*x_{k} - L*x_{k+1}
                x_data[VECIDX(k, i, j)] *= one_minus_weight;
                x_data[VECIDX(k, i, j)] += weight * tmp / diag_val;
            }

            for (idx_t k = kbeg + 1; k < kend - 1; k++) {
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
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
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// U*x_{k} + L*x_{k+1}
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - U*x_{k} - L*x_{k+1}
                x_data[VECIDX(k, i, j)] *= one_minus_weight;
                x_data[VECIDX(k, i, j)] += weight * tmp / diag_val;
            }

            // top
            {
                idx_t k = kend - 1;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
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
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)];// (D+U)*x_{k} + L*x_{k+1}
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                //   + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                //   + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                //   + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                //   + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - U*x_{k} - L*x_{k+1}
                x_data[VECIDX(k, i, j)] *= one_minus_weight;
                x_data[VECIDX(k, i, j)] += weight * tmp / diag_val;
            }
        }
    }
}

template<typename idx_t, typename data_t>
void PointGS<idx_t, data_t>::BackwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
{
    const seq_structVector<idx_t, data_t> & b_vec = *(b.local_vector);
          seq_structVector<idx_t, data_t> & x_vec = *(x.local_vector);
    const seq_structMatrix<idx_t, data_t> & A_mat = *(((par_structMatrix<idx_t, data_t> *)(this->oper))->local_matrix);
    CHECK_LOCAL_HALO(x_vec, b_vec);
    CHECK_LOCAL_HALO(x_vec, A_mat);

    const data_t * b_data = b_vec.data;
          data_t * x_data = x_vec.data;
          data_t * A      = A_mat.data;

    const idx_t ibeg = b_vec.halo_x, iend = ibeg + b_vec.local_x,
                jbeg = b_vec.halo_y, jend = jbeg + b_vec.local_y,
                kbeg = b_vec.halo_z, kend = kbeg + b_vec.local_z;
    const idx_t vec_k_size = x_vec.slice_k_size, vec_ki_size = x_vec.slice_ki_size;
    const idx_t num_diag = A_mat.num_diag, slice_dk_size = A_mat.slice_dk_size, slice_dki_size = A_mat.slice_dki_size;
    
    const data_t one_minus_weight = 1.0 - weight;
    for (idx_t j = jend - 1; j >= jbeg; j--) {
        for (idx_t i = iend - 1; i >= ibeg; i--) {
            // top
            {
                idx_t k = kend - 1;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
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
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)];// (D+L)*x_{k} + U*x_{k+1}
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                //   + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                //   + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                //   + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                //   + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - U*x_{k+1} - L*x_{k}
                x_data[VECIDX(k, i, j)] *= one_minus_weight;
                x_data[VECIDX(k, i, j)] += weight * tmp / diag_val;
            }

            for (idx_t k = kend - 2; k >= kbeg + 1; k--) {
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
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
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// (D+L)*x_{k} + U*x_{k+1}
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - U*x_{k+1} - L*x_{k}
                x_data[VECIDX(k, i, j)] *= one_minus_weight;
                x_data[VECIDX(k, i, j)] += weight * tmp / diag_val;
            }

            // bottom
            {
                idx_t k = kbeg;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]
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
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// (D+L)*x_{k} + U*x_{k+1}
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - U*x_{k+1} - L*x_{k}
                x_data[VECIDX(k, i, j)] *= one_minus_weight;
                x_data[VECIDX(k, i, j)] += weight * tmp / diag_val;
            }
        }
    }
}

#if 0
template<typename idx_t, typename data_t>
void PointGS<idx_t, data_t>::separate_LDU(const par_structVector<idx_t, data_t> & x) const {
    assert(this->oper != nullptr);
    // 提取矩阵对角元到向量，提取L和U到另一个矩阵
    assert(this->oper->input_dim[0] == this->oper->output_dim[0] && this->oper->input_dim[0] == x.global_size_x &&
           this->oper->input_dim[1] == this->oper->output_dim[1] && this->oper->input_dim[1] == x.global_size_y &&
           this->oper->input_dim[2] == this->oper->output_dim[2] && this->oper->input_dim[2] == x.global_size_z);

    const seq_structMatrix<idx_t, data_t> & mat = *(((par_structMatrix<idx_t, data_t> *) oper)->local_matrix);
    assert((mat.num_diag - 1) % 2 ==0);

    D = new seq_structVector<idx_t, data_t>(*(x.local_vector));
    L = new seq_structMatrix<idx_t, data_t>( (mat.num_diag - 1) / 2, 
                                            mat.local_x, mat.local_y, mat.local_z, mat.halo_x, mat.halo_y, mat.halo_z);
    U = new seq_structMatrix<idx_t, data_t>(*L);

    const idx_t jbeg = 0, jend = mat.local_x + 2 * mat.halo_x,
                ibeg = 0, iend = mat.local_y + 2 * mat.halo_y,
                kbeg = 0, kend = mat.local_z + 2 * mat.halo_z;
    const idx_t vec_k_size = D->slice_k_size, vec_ki_size = D->slice_ki_size;

    for (idx_t j = jbeg; j < jend; j++)
        for (idx_t i = ibeg; i < iend; i++)
            for (idx_t k = kbeg; k < kend; k++) {
#define IDX(mat, d, k, i, j) (d) + (k)*(mat).num_diag + (i)*(mat).slice_dk_size + (j)*(mat).slice_dki_size
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
                
                    D : 0
                    依据更新中心点时，该点是否已被更新来分类L和U
                    L : 1, 3, 6, 7,  9, 10, 12, 15, 17
                    U : 2, 4, 5, 8, 11, 13, 14, 16, 18
                */
                D->data[VECIDX(      k, i, j)] = mat.data[IDX(mat, 0, k, i, j)];

                L->data[IDX(*L, 0, k, i, j)] = mat.data[IDX(mat, 1, k, i, j)];
                L->data[IDX(*L, 1, k, i, j)] = mat.data[IDX(mat, 3, k, i, j)];
                L->data[IDX(*L, 2, k, i, j)] = mat.data[IDX(mat, 6, k, i, j)];
                L->data[IDX(*L, 3, k, i, j)] = mat.data[IDX(mat, 7, k, i, j)];
                L->data[IDX(*L, 4, k, i, j)] = mat.data[IDX(mat, 9, k, i, j)];
                L->data[IDX(*L, 5, k, i, j)] = mat.data[IDX(mat,10, k, i, j)];
                L->data[IDX(*L, 6, k, i, j)] = mat.data[IDX(mat,12, k, i, j)];
                L->data[IDX(*L, 7, k, i, j)] = mat.data[IDX(mat,15, k, i, j)];
                L->data[IDX(*L, 8, k, i, j)] = mat.data[IDX(mat,17, k, i, j)];

                U->data[IDX(*U, 0, k, i, j)] = mat.data[IDX(mat, 2, k, i, j)];
                U->data[IDX(*U, 1, k, i, j)] = mat.data[IDX(mat, 4, k, i, j)];
                U->data[IDX(*U, 2, k, i, j)] = mat.data[IDX(mat, 5, k, i, j)];
                U->data[IDX(*U, 3, k, i, j)] = mat.data[IDX(mat, 8, k, i, j)];
                U->data[IDX(*U, 4, k, i, j)] = mat.data[IDX(mat,11, k, i, j)];
                U->data[IDX(*U, 5, k, i, j)] = mat.data[IDX(mat,13, k, i, j)];
                U->data[IDX(*U, 6, k, i, j)] = mat.data[IDX(mat,14, k, i, j)];
                U->data[IDX(*U, 7, k, i, j)] = mat.data[IDX(mat,16, k, i, j)];
                U->data[IDX(*U, 8, k, i, j)] = mat.data[IDX(mat,18, k, i, j)];
#undef IDX
            }
    LDU_separated = true;
}

template<typename idx_t, typename data_t>
void PointGS<idx_t, data_t>::ForwardPass_ZERO(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
{
    const seq_structVector<idx_t, data_t> & b_vec = *(b.local_vector);
          seq_structVector<idx_t, data_t> & x_vec = *(x.local_vector);
    CHECK_LOCAL_HALO(x_vec, b_vec);
    CHECK_LOCAL_HALO(x_vec, *U);
    CHECK_LOCAL_HALO(x_vec, *L);
    CHECK_LOCAL_HALO(x_vec, *D);

    const data_t * b_data = b_vec.data;
          data_t * x_data = x_vec.data;
    const data_t * L_data = L->data;
    const data_t * U_data = U->data;
    const data_t * D_data = D->data;      

    const idx_t ibeg = b_vec.halo_x, iend = ibeg + b_vec.local_x,
                jbeg = b_vec.halo_y, jend = jbeg + b_vec.local_y,
                kbeg = b_vec.halo_z, kend = kbeg + b_vec.local_z;
    const idx_t vec_k_size = x_vec.slice_k_size, vec_ki_size = x_vec.slice_ki_size;
    const idx_t num_diag = L->num_diag, slice_dk_size = L->slice_dk_size, slice_dki_size = L->slice_dki_size;
    const data_t one_minus_weight = 1.0 - weight;
    
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
            // D : 0
            // 依据更新中心点时，该点是否已被更新来分类L和U
            // L : 1, 3, 6, 7,  9, 10, 12, 15, 17 <=> 在L中顺序存储
            
            // bottom
            {
                idx_t k = kbeg;
                data_t diag_val = D_data[VECIDX(k, i, j)];
                data_t tmp = // L*x_{k+1}
                //   + diag_val * x_data[VECIDX(k  , i  , j  )]
                  + L_data[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  //+ U_data[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + L_data[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  //+ U_data[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  //+ U_data[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + L_data[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + L_data[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  //+ U_data[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                
                  //+ U_data[MATIDX( 6, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                  + L_data[MATIDX( 7, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  //+ U_data[MATIDX( 7, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + L_data[MATIDX( 8, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  //+ U_data[MATIDX( 8, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)]
                  ;
                tmp = b_data[VECIDX(k, i, j)] - tmp;
                x_data[VECIDX(k, i, j)] = weight * tmp / diag_val;
            }

            for (idx_t k = kbeg + 1; k < kend - 1; k++) {

                data_t diag_val = D_data[VECIDX(k, i, j)];
                data_t tmp = // L*x_{k+1}
                //   + diag_val * x_data[VECIDX(k  , i  , j  )]
                  + L_data[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  //+ U_data[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + L_data[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  //+ U_data[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  //+ U_data[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + L_data[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + L_data[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  //+ U_data[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                  + L_data[MATIDX( 4, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]
                  + L_data[MATIDX( 5, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  //+ U_data[MATIDX( 4, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + L_data[MATIDX( 6, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  //+ U_data[MATIDX( 5, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)]
                  //+ U_data[MATIDX( 6, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                  + L_data[MATIDX( 7, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  //+ U_data[MATIDX( 7, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + L_data[MATIDX( 8, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  //+ U_data[MATIDX( 8, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)]
                  ;
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - (D+U)*x_{k} - L*x_{k+1}
                x_data[VECIDX(k, i, j)] = weight * tmp / diag_val;
            }

            // top
            {
                idx_t k = kend - 1;
                data_t diag_val = D_data[VECIDX(k, i, j)];
                data_t tmp = // L*x_{k+1}
                //   + diag_val * x_data[VECIDX(k  , i  , j  )]
                  + L_data[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  //+ U_data[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + L_data[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  //+ U_data[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  //+ U_data[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + L_data[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + L_data[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  //+ U_data[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                  + L_data[MATIDX( 4, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]
                  + L_data[MATIDX( 5, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  //+ U_data[MATIDX( 4, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + L_data[MATIDX( 6, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  //+ U_data[MATIDX( 5, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)]
                  ;
                tmp = b_data[VECIDX(k, i, j)] - tmp;// b - (D+U)*x_{k} - L*x_{k+1}
                x_data[VECIDX(k, i, j)] = weight * tmp / diag_val;
            }
        }
    }
}

#endif

#undef VECIDX
#undef MATIDX

#endif