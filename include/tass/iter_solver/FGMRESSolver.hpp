#ifndef GRAPES_FGMRES_HPP
#define GRAPES_FGMRES_HPP

#include "iter_solver.hpp"
#include "../utils/par_struct_mat.hpp"

template<typename idx_t, typename ksp_t, typename pc_t>
class FGMRESSolver : public IterativeSolver<idx_t, ksp_t, pc_t> {
public:
    int restart_len = 10;

    FGMRESSolver() {  }

    virtual void SetRestartlen(int num) { restart_len = num; }
    virtual void Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const ;
};

#include "GMRESSolver.hpp" // for GeneratePlaneRotation(), ApplyPlaneRotation() Update()

#define arrId(i,j) ((i)*restart_len + j)

template<typename idx_t, typename ksp_t, typename pc_t>
void FGMRESSolver<idx_t, ksp_t, pc_t>::Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const 
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(this->restart_len > 0);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);// 如果不满足，可能会有问题？从之前的向量构造的规范性上应该是满足的

    MPI_Comm comm = x.comm_pkg->cart_comm;
    int my_pid; MPI_Comm_rank(comm, &my_pid);

    par_structVector<idx_t, ksp_t> r(x);// 残差向量

    if (this->iterative_mode) {
        this->oper->Mult(x, r);
        vec_add(b, -1.0, r, r);
    } else {
        x.set_val(0.0);
        vec_copy(b, r);
    }

    double beta = this->Norm(r);

    this->final_norm = std::max(this->rel_tol * beta, this->abs_tol);
    if (beta <= this->final_norm) {
        this->final_norm = beta;
        this->final_iter = 0;
        this->converged = 1;
        return ;
    }

    if (my_pid == 0) printf("   Pass : %2d   Iteration : %3d  || r || = %20.16e\n", 1, 0, beta);

    // 初始化辅助向量
    std::allocator<par_structVector<idx_t, ksp_t> > alloc;
    par_structVector<idx_t, ksp_t> * v = alloc.allocate(restart_len + 1),
                                   * z = alloc.allocate(restart_len + 1);
    for (int i = 0; i <= restart_len; i++) {
        alloc.construct(v + i, x);
        alloc.construct(z + i, x);
    } 
    // GMRES 相关的数据结构
    double * H       = (double *) malloc (sizeof(double) * (restart_len + 1) * restart_len);
    double * s       = (double *) malloc (sizeof(double) * (restart_len + 1) );
    double * cs      = (double *) malloc (sizeof(double) * (restart_len + 1) );
    double * sn      = (double *) malloc (sizeof(double) * (restart_len + 1) );
    double * aux_y   = (double *) malloc (sizeof(double) * (restart_len + 1) );

    int j = 1;
    while (j <= this->max_iter) {
        vec_mul_by_scalar(1.0/beta, r, v[0]);
        for (int kk = 0; kk <= restart_len; kk++) s[kk] = 0.0;
        s[0] = beta;

        int i;
        for (i = 0; i < restart_len && j <= this->max_iter; i++, j++) {
            z[i].set_val(0.0);

            if (this->prec)
                this->prec->Mult(v[i], z[i]);
            else
                vec_copy(v[i], z[i]);
            
            this->oper->Mult(z[i], r);

            for (int k = 0; k <= i; k++) {
                H[arrId(k, i)] = this->Dot(r, v[k]);// H(k,i) = r * v[k]
                vec_add(r, -H[arrId(k,i)], v[k], r);// r -= H(k,i) * v[k]
            }

            H[arrId(i+1, i)] = this->Norm(r);// H(i+1,i) = ||r||
            
            vec_mul_by_scalar(1.0/H[arrId(i+1, i)], r, v[i+1]);

            for (int k = 0; k < i; k++) 
                ApplyPlaneRotation(H[arrId(k,i)], H[arrId(k+1,i)], cs[k], sn[k]);
            
            GeneratePlaneRotation (H[arrId(i,i)], H[arrId(i+1,i)], cs[i], sn[i]);
            ApplyPlaneRotation    (H[arrId(i,i)], H[arrId(i+1,i)], cs[i], sn[i]);
            ApplyPlaneRotation    (s[i], s[i+1], cs[i], sn[i]);

            double resid = fabs(s[i+1]);
            if (my_pid == 0) printf("   Pass : %2d   Iteration : %3d  || r || = %20.16e\n", (j-1)/restart_len + 1, j, resid);

            if (resid <= this->final_norm) {
                Update(x, i, H, s, z, restart_len, aux_y);
                this->final_norm = resid;
                this->final_iter = j;
                this->converged = 1;
                goto finish;
            }
        }// i loop

        if (my_pid == 0) printf("Restarting...\n");

        Update(x, i-1, H, s, z, restart_len, aux_y);// 更新解

        this->oper->Mult(x, r);
        vec_add(b, -1.0, r, r);
        beta = this->Norm(r);

        if (beta <= this->final_norm) {
            this->final_norm = beta;
            this->final_iter = j;
            this->converged = 1;
            goto finish;
        }
    }// j loop

    this->converged = 0;

finish:
    free(H); free(s); free(cs); free(sn); free(aux_y);
    for (int i = 0; i <= restart_len; i++) {
        alloc.destroy( v + i);
        alloc.destroy( z + i);
    }
    alloc.deallocate( v, restart_len + 1);
    alloc.deallocate( z, restart_len + 1);
}


#endif