#ifndef GRAPES_LINE_JACOBI_HPP
#define GRAPES_LINE_JACOBI_HPP

#include "line_Solver.hpp"

template<typename idx_t, typename data_t>
class LineJacobi : public LineSolver<idx_t, data_t> {
public:
    LineJacobi(char line_dir = 'z') : LineSolver<idx_t, data_t>(line_dir) {  }
    virtual void Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const;
};

template<typename idx_t, typename data_t>
void LineJacobi<idx_t, data_t>::Mult(const par_structVector<idx_t, data_t> & b, par_structVector<idx_t, data_t> & x) const
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);

    if (this->used_as_precond) {// 用作预条件时，默认只做一次，且可以做0-初值优化
        // 不需要通信
        const seq_structMatrix<idx_t, data_t> & A_mat = *(((par_structMatrix<idx_t, data_t>*)(this->oper))->local_matrix);
        const seq_structVector<idx_t, data_t> & b_vec = *(b.local_vector);
              seq_structVector<idx_t, data_t> & x_vec = *(x.local_vector);
        // Jacobi一定是对称操作，检查b_vec和x_vec的规格相容性
        CHECK_LOCAL_HALO(x_vec, b_vec);

        const data_t * A_data = A_mat.data;
        const data_t * b_data = b_vec.data;
              data_t * x_data = x_vec.data;
        const idx_t ibeg = b_vec.halo_x, iend = ibeg + b_vec.local_x,
                    jbeg = b_vec.halo_y, jend = jbeg + b_vec.local_y,
                    kbeg = b_vec.halo_z, kend = kbeg + b_vec.local_z;
        const idx_t vec_k_size = x_vec.slice_k_size, vec_ki_size = x_vec.slice_ki_size;
        const idx_t num_diag = A_mat.num_diag, slice_dk_size = A_mat.slice_dk_size, slice_dki_size = A_mat.slice_dki_size;
        
        assert(this->line_dir == 'z');
        
        idx_t tot_lev = x_vec.halo_z * 2 + x_vec.local_z;
        assert(tot_lev == kend - kbeg);

        data_t * rhs = new data_t [tot_lev], * sol = new data_t [tot_lev];

        for (idx_t j = jbeg; j < jend; j++) {
            for (idx_t i = ibeg; i < iend; i++) {
                for (idx_t k = kbeg; k < kend; k++) {
#define VECIDX(k, i, j) (k) + (i) * vec_k_size + (j) * vec_ki_size
                    // 因为是0初值，所以直接拷贝右端向量的值
                    rhs[k] = b_data[VECIDX(k, i, j)];
                }

                this->tri_solver[A_mat.local_x * (j - jbeg) + i - ibeg].Solve(rhs, sol);

                for (idx_t k = kbeg; k < kend; k++)
                    x_data[VECIDX(k, i, j)] = this->weight * sol[k];
#undef VECIDX
            }
        }

        delete rhs; delete sol;
    }
    else {// 用作迭代算子
        MPI_Comm comm = b.comm_pkg->cart_comm;
        int my_pid;
        MPI_Comm_rank(comm, &my_pid);

        if (my_pid == 0)
            printf("Error: LineJacobi: usage of non-preconditioner not implemented\n");

        MPI_Abort(comm, -3000);
    }
}

#endif