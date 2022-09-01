#ifndef GRAPES_LINE_GS_HPP
#define GRAPES_LINE_GS_HPP

#include "line_Solver.hpp"

template<typename idx_t, typename data_t>
class LineGS : public LineSolver<idx_t, data_t> {
public:
    // 对称GS：0 for sym, 1 for forward, -1 backward
    SCAN_TYPE scan_type = SYMMETRIC;

    LineGS(SCAN_TYPE type = SYMMETRIC, int line_dir = 'z') : LineSolver<idx_t, data_t>(line_dir), scan_type(type) {  }
    virtual void Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;

    void ForwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;
    void BackwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;
};

template<typename idx_t, typename data_t>
void LineGS<idx_t, data_t>::Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);

    if (this->used_as_precond) {
        switch (scan_type)
        {
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
void LineGS<idx_t, data_t>::ForwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
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

    assert(this->line_dir == 'z');
    idx_t tot_lev = x_vec.halo_z * 2 + x_vec.local_z;
    assert(tot_lev == kend - kbeg);
    data_t * rhs = new data_t [tot_lev], * sol = new data_t [tot_lev];

    const data_t one_minus_weight = 1.0 - this->weight;
    for (idx_t j = jbeg; j < jend; j++) {
        for (idx_t i = ibeg; i < iend; i++) {
            // bottom
            {
                idx_t k = kbeg;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]// 去掉0的位置，因为联立求解
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
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]// 去掉14的位置，因为联立求解
                  + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// U*x_{k} + L*x_{k+1}
                rhs[k] = b_data[VECIDX(k, i, j)] - tmp;// b - ∑a_{ngb}*x_{ngb}
            }

            for (idx_t k = kbeg + 1; k < kend - 1; k++) {
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]// 去掉0的位置，因为联立求解
                  + A[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  + A[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + A[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  + A[MATIDX( 4, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  + A[MATIDX( 5, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + A[MATIDX( 6, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + A[MATIDX( 7, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  + A[MATIDX( 8, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                //   + A[MATIDX( 9, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]// 去掉9的位置，因为联立求解
                  + A[MATIDX(10, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  + A[MATIDX(11, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + A[MATIDX(12, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)]
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]// 去掉14的位置，因为联立求解
                  + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// U*x_{k} + L*x_{k+1}
                rhs[k] = b_data[VECIDX(k, i, j)] - tmp;// b - ∑a_{ngb}*x_{ngb}
            }

            // top
            {
                idx_t k = kend - 1;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]// 去掉0的位置，因为联立求解
                  + A[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  + A[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + A[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  + A[MATIDX( 4, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  + A[MATIDX( 5, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + A[MATIDX( 6, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + A[MATIDX( 7, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  + A[MATIDX( 8, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                //   + A[MATIDX( 9, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]// 去掉9的位置，因为联立求解
                  + A[MATIDX(10, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  + A[MATIDX(11, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + A[MATIDX(12, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)];// (D+U)*x_{k} + L*x_{k+1}
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                //   + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                //   + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                //   + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                //   + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];
                rhs[k] = b_data[VECIDX(k, i, j)] - tmp;// b - ∑a_{ngb}*x_{ngb}
            }

            this->tri_solver[A_mat.local_x * (j - jbeg) + i - ibeg].Solve(rhs, sol);
            
            /* 或者可以直接调用tridiag_thomas
             * 
             * coeff_c[k] = A[MATIDX(14, k, i, j)];
               coeff_b[k] = A[MATIDX( 0, k, i, j)];
               coeff_a[k] = A[MATIDX( 9, k, i, j)];
               assert(coeff_a[0] == 0.0 && coeff_c[tot_lev - 1] == 0.0);
               tridiag_thomas(coeff_a, coeff_b, coeff_c, rhs, sol, tot_lev);
             */

            for (idx_t k = kbeg; k < kend; k++)
                x_data[VECIDX(k, i, j)] = this->weight * sol[k];
        }
    }

    delete rhs; delete sol;
}

template<typename idx_t, typename data_t>
void LineGS<idx_t, data_t>::BackwardPass(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
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

    assert(this->line_dir == 'z');
    idx_t tot_lev = x_vec.halo_z * 2 + x_vec.local_z;
    assert(tot_lev == kend - kbeg);
    data_t * rhs = new data_t [tot_lev], * sol = new data_t [tot_lev];
    
    const data_t one_minus_weight = 1.0 - this->weight;
    for (idx_t j = jend - 1; j >= jbeg; j--) {
        for (idx_t i = iend - 1; i >= ibeg; i--) {
            // top
            {
                idx_t k = kend - 1;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]// 去掉0的位置，因为联立求解
                  + A[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  + A[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + A[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  + A[MATIDX( 4, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  + A[MATIDX( 5, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + A[MATIDX( 6, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + A[MATIDX( 7, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  + A[MATIDX( 8, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                //   + A[MATIDX( 9, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]// 去掉9的位置，因为联立求解
                  + A[MATIDX(10, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  + A[MATIDX(11, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + A[MATIDX(12, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)];// (D+U)*x_{k} + L*x_{k+1}
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]
                //   + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                //   + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                //   + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                //   + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];
                rhs[k] = b_data[VECIDX(k, i, j)] - tmp;// b - ∑a_{ngb}*x_{ngb}
            }

            for (idx_t k = kend - 2; k >= kbeg + 1; k--) {
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]// 去掉0的位置，因为联立求解
                  + A[MATIDX( 1, k, i, j)] * x_data[VECIDX(k  , i-1, j  )]
                  + A[MATIDX( 2, k, i, j)] * x_data[VECIDX(k  , i+1, j  )]
                  + A[MATIDX( 3, k, i, j)] * x_data[VECIDX(k  , i  , j-1)]
                  + A[MATIDX( 4, k, i, j)] * x_data[VECIDX(k  , i  , j+1)]
                  + A[MATIDX( 5, k, i, j)] * x_data[VECIDX(k  , i+1, j+1)]
                  + A[MATIDX( 6, k, i, j)] * x_data[VECIDX(k  , i+1, j-1)]
                  + A[MATIDX( 7, k, i, j)] * x_data[VECIDX(k  , i-1, j-1)]
                  + A[MATIDX( 8, k, i, j)] * x_data[VECIDX(k  , i-1, j+1)]
                //   + A[MATIDX( 9, k, i, j)] * x_data[VECIDX(k-1, i  , j  )]// 去掉9的位置，因为联立求解
                  + A[MATIDX(10, k, i, j)] * x_data[VECIDX(k-1, i-1, j  )]
                  + A[MATIDX(11, k, i, j)] * x_data[VECIDX(k-1, i+1, j  )]
                  + A[MATIDX(12, k, i, j)] * x_data[VECIDX(k-1, i  , j-1)]
                  + A[MATIDX(13, k, i, j)] * x_data[VECIDX(k-1, i  , j+1)]
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]// 去掉14的位置，因为联立求解
                  + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// U*x_{k} + L*x_{k+1}
                rhs[k] = b_data[VECIDX(k, i, j)] - tmp;// b - ∑a_{ngb}*x_{ngb}
            }

            // bottom
            {
                idx_t k = kbeg;
                data_t diag_val = A[MATIDX( 0, k, i, j)];
                data_t tmp = 
                //   + A[MATIDX( 0, k, i, j)] * x_data[VECIDX(k  , i  , j  )]// 去掉0的位置，因为联立求解
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
                //   + A[MATIDX(14, k, i, j)] * x_data[VECIDX(k+1, i  , j  )]// 去掉14的位置，因为联立求解
                  + A[MATIDX(15, k, i, j)] * x_data[VECIDX(k+1, i-1, j  )]
                  + A[MATIDX(16, k, i, j)] * x_data[VECIDX(k+1, i+1, j  )]
                  + A[MATIDX(17, k, i, j)] * x_data[VECIDX(k+1, i  , j-1)]
                  + A[MATIDX(18, k, i, j)] * x_data[VECIDX(k+1, i  , j+1)];// U*x_{k} + L*x_{k+1}
                rhs[k] = b_data[VECIDX(k, i, j)] - tmp;// b - ∑a_{ngb}*x_{ngb}
            }

            this->tri_solver[A_mat.local_x * (j - jbeg) + i - ibeg].Solve(rhs, sol);
            

            for (idx_t k = kbeg; k < kend; k++)
                x_data[VECIDX(k, i, j)] = this->weight * sol[k];
        }
    }

    delete rhs; delete sol;
}

#undef VECIDX
#undef MATIDX

#endif