#ifndef GRAPES_POINT_JACOBI_HPP
#define GRAPES_POINT_JACOBI_HPP

#include "precond.hpp"
#include "../utils/par_struct_mat.hpp"

template<typename idx_t, typename data_t>
class PointJacobi : public Solver<idx_t, data_t> {
public:
    // when used as precondition, just do once, and zero-val optimization
    // could be exploited 
    bool used_as_precond = true;

    // weighted Jacobi
    mutable data_t weight = 1.0;

    // operator (often as matrix-A)
    const Operator<idx_t, data_t> * oper = nullptr;

    // separate diagonal values if for efficiency concern is needed
    // should only be used when separation is cheap
    mutable bool D_separated = false;
    mutable par_structVector<idx_t, data_t> * invD = nullptr;

    PointJacobi() : Solver<idx_t, data_t>(0, true) {  }

    // 按照x的规格进行构造结构化向量，并从oper中提取对角元
    void separate_D(const par_structVector<idx_t, data_t> & x) const ;

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

    // 近似求解一个（残差）方程，以b为右端向量，返回x为近似解
    // 原方程 A*x = b，分解系数矩阵 A := D + L + U
    // Richardson迭代法：x_{k+1} = x_{k} + w*M^{-1}*(b - A*x_{k})
    // 按Jacobi迭代法：取M = D，则有
    // x_{k+1} = x_{k} + w*D^{-1}*(b - (D+L+U)*x_{k})
    // D*x_{k+1} = D*x_{k} + w*(b - (D+L+U)*x_{k}) = w*b + (1-w)*D*x_{k} - w*(L+U)*x_{k}
    // 当x_{k}为0初值时，可优化为D*x_{k+1} = w*b，即x_{k+1} = w*D^{-1}*b
    virtual void Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;

    virtual ~PointJacobi();
};

template<typename idx_t, typename data_t>
void PointJacobi<idx_t, data_t>::separate_D(const par_structVector<idx_t, data_t> & x) const {
    assert(this->oper != nullptr);

    invD = new par_structVector<idx_t, data_t>(x);

    // 提取矩阵对角元到向量
    assert(this->oper->input_dim[0] == this->oper->output_dim[0] && this->oper->input_dim[0] == x.global_size_x &&
           this->oper->input_dim[1] == this->oper->output_dim[1] && this->oper->input_dim[1] == x.global_size_y &&
           this->oper->input_dim[2] == this->oper->output_dim[2] && this->oper->input_dim[2] == x.global_size_z);

    const seq_structMatrix<idx_t, data_t> & mat = *(((par_structMatrix<idx_t, data_t> *) oper)->local_matrix);
    const seq_structVector<idx_t, data_t> & vec = *(invD->local_vector);
    CHECK_LOCAL_HALO(mat, vec);

    const idx_t jbeg = 0, jend = mat.local_x + 2 * mat.halo_x,
                ibeg = 0, iend = mat.local_y + 2 * mat.halo_y,
                kbeg = 0, kend = mat.local_z + 2 * mat.halo_z;
    const idx_t num_diag = mat.num_diag, slice_dk_size = mat.slice_dk_size, slice_dki_size = mat.slice_dki_size;
    const idx_t vec_k_size = vec.slice_k_size, vec_ki_size = vec.slice_ki_size;

    for (idx_t j = jbeg; j < jend; j++)
        for (idx_t i = ibeg; i < iend; i++)
            for (idx_t k = kbeg; k < kend; k++) {
#define MATIDX(k, i, j) (k) * num_diag + (i) * slice_dk_size + (j) * slice_dki_size // 直接取第0个数
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
                vec.data[VECIDX(k, i, j)] = 1.0 / mat.data[MATIDX(k, i, j)];
#undef MATIDX
#undef VECIDX
            }

    D_separated = true;
}

template<typename idx_t, typename data_t>
PointJacobi<idx_t, data_t>::~PointJacobi() {
    if (invD != nullptr) delete invD;
}

template<typename idx_t, typename data_t>
void PointJacobi<idx_t, data_t>::Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);

    if (used_as_precond) {// 用作预条件时，默认只做一次，且可以做0-初值优化
        if (!D_separated) separate_D(x);

        // 不需要通信
        const seq_structVector<idx_t, data_t> & b_vec = *(b.local_vector);
              seq_structVector<idx_t, data_t> & x_vec = *(x.local_vector);

        // Jacobi一定是对称操作，检查b_vec和x_vec的规格相容性
        CHECK_LOCAL_HALO(x_vec, b_vec);

        const data_t * b_data = b_vec.data;
              data_t * x_data = x_vec.data;
        const data_t * invD_val  = invD->local_vector->data;
        const idx_t ibeg = b_vec.halo_x, iend = ibeg + b_vec.local_x,
                    jbeg = b_vec.halo_y, jend = jbeg + b_vec.local_y,
                    kbeg = b_vec.halo_z, kend = kbeg + b_vec.local_z;
        const idx_t vec_k_size = x_vec.slice_k_size, vec_ki_size = x_vec.slice_ki_size;
        
        for (idx_t j = jbeg; j < jend; j++) {
            for (idx_t i = ibeg; i < iend; i++) {
                for (idx_t k = kbeg; k < kend; k++) {
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
                    x_data[VECIDX(k, i, j)] = weight * invD_val[VECIDX(k, i, j)] * b_data[VECIDX(k, i, j)];
#undef VECIDX
                }
            }
        }
    }
    else {// 用作迭代算子
        MPI_Comm comm = b.comm_pkg->cart_comm;
        int my_pid;
        MPI_Comm_rank(comm, &my_pid);

        if (my_pid == 0)
            printf("Error: PointJacobi: usage of non-preconditioner not implemented\n");

        MPI_Abort(comm, -3000);
    }
}

#endif