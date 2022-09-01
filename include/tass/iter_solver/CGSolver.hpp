#ifndef GRAPES_CG_HPP
#define GRAPES_CG_HPP

#include "iter_solver.hpp"
#include "../utils/par_struct_mat.hpp"

template<typename idx_t, typename ksp_t, typename pc_t>
class CGSolver : public IterativeSolver<idx_t, ksp_t, pc_t> {
public:

    CGSolver() {  };

    virtual void Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const ;
};


template<typename idx_t, typename ksp_t, typename pc_t>
void CGSolver<idx_t, ksp_t, pc_t>::Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const 
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);

    MPI_Comm comm = x.comm_pkg->cart_comm;
    int my_pid; MPI_Comm_rank(comm, &my_pid);

    // 初始化辅助向量：r残差，p搜索方向，Ap为A乘以搜索方向
    par_structVector<idx_t, ksp_t> r(x), Ap(x), p(x);

    if (this->iterative_mode) {// 继续用上次的解进行refine，则要先算一遍残差
        this->oper->Mult(x, r);
        vec_add(b, -1.0, r, r);
    } else {// 初始解设为0，残差也就是右端向量b
        vec_copy(b, r);
        x.set_val(0.0);
    }

    double norm_b = this->Norm(b);
    double norm_r = this->Norm(r);
    double alpha_nom, alpha_denom, beta_nom;

    int & iter = this->final_iter;

    if (my_pid == 0) printf("iter %4d %21s ||r||/||b|| = %20.16e\n", iter, "", (double)(norm_r / norm_b));

    if (this->prec) {// 有预条件子，则以预条件后的残差M^{-1}*r作为搜索方向 p = M^{-1}*r
        this->prec->Mult(r, p);
    } else {// 没有预条件则直接以残差r作为搜索方向
        vec_copy(r, p);
    }
    iter++;// 执行一次预条件子就算一次迭代

    // Ap = A * p
    this->oper->Mult(p, Ap);

    alpha_nom   = this->Dot(r,  p);
    alpha_denom = this->Dot(p, Ap);

    for ( ; iter < this->max_iter; ) {
        double alpha = alpha_nom / alpha_denom;
        // 更新解和残差
        vec_add(x,  alpha,  p, x);
        vec_add(r, -alpha, Ap, r);

        // 判敛
        norm_r = this->Norm(r);
        if (my_pid == 0) printf("iter %4d   alpha %.10e   ||r||/||b|| = %.10e  ||r|| %.10e\n", 
                            iter, (double)alpha, (double)(norm_r / norm_b), (double) norm_r);
        if (norm_r / norm_b <= this->rel_tol || norm_r <= this->abs_tol) {
            this->converged = 1;
            break;
        }

        // 预条件子
        if (this->prec) {
            this->prec->Mult(r, Ap);
        } else {
            vec_copy(r, Ap);
        }
        iter++;// 执行一次预条件子就算一次迭代

        beta_nom = this->Dot(r, Ap);// beta的分子
        if (beta_nom < 0.0) {
            if (my_pid == 0) printf("WARNING: PCG: The preconditioner is not positive definite. (Br, r) = %.5e\n", (double)beta_nom);
            this->converged = 0;
            break;
        }

        // 计算β并更新搜索方向
        double beta = beta_nom / alpha_nom;
        // 更新搜索方向
        vec_add(Ap, beta, p, p);
        
        // Ap = A * p
        this->oper->Mult(p, Ap);

        alpha_denom = this->Dot(p, Ap);
        if (alpha_denom <= 0.0) {
            double dd = this->Dot(p, p);
            if (dd > 0.0)  printf("WARNING: PCG: The operator is not positive definite. (Ad, d) = %.5e\n", (double)alpha_denom);
            if (alpha_denom == 0.0) break;
        }
        alpha_nom = beta_nom;
    }
}


#if 0 // MFEM版本的CG，以预条件后的残差 (B r, r) ，不太准
template<typename idx_t, typename ksp_t, typename pc_t>
void CGSolver<idx_t, ksp_t, pc_t>::Mult(par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const 
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);

    MPI_Comm comm = x.comm_pkg->cart_comm;
    int my_pid; MPI_Comm_rank(comm, &my_pid);

    // 初始化辅助向量：r残差，p搜索方向，Ap为A乘以搜索方向
    par_structVector<idx_t, ksp_t> r(x), Ap(x), p(x);

    if (this->iterative_mode) {// 继续用上次的解进行refine，则要先算一遍残差
        this->oper->Mult(x, r);
        vec_add(b, -1.0, r, r);
    } else {// 初始解设为0，残差也就是右端向量b
        vec_copy(b, r);
        x.set_val(0.0);
    }

    double alpha_nom, alpha_denom, beta_nom, threshold, nom0;

    if (this->prec) {// 有预条件子，则以预条件后的残差M^{-1}*r作为搜索方向 p = M^{-1}*r
        this->prec->Mult(r, p);
    } else {// 没有预条件则直接以残差r作为搜索方向
        vec_copy(r, p);
    }

    alpha_nom   = nom0 = this->Dot(r, p);// alpha的分子
    if (alpha_nom < 0.0) {
        if (my_pid == 0)
            printf("Warning: PCG: The preconditioner is not positive definite. (Br, r) = %.16e\n", alpha_nom);
        this->converged = 0;
        this->final_iter = 0;
        this->final_norm = alpha_nom;
        return ;
    }
    threshold = std::max(alpha_nom * this->rel_tol * this->rel_tol, this->abs_tol * this->abs_tol);
    if (alpha_nom <= threshold) {
        this->converged = 1;
        this->final_iter = 0;
        this->final_norm = sqrt(alpha_nom);
        return ;
    }

    this->oper->Mult(p, Ap);

    alpha_denom = this->Dot(p, Ap);// alpha的分母
    if (alpha_denom <= 0.0) {
        double dd = this->Dot(p, p);
        if (dd > 0.0)
            if (my_pid == 0)
                printf("Warning: PCG: The operator is not positive definite. (Ad, d) = %.16e\n", alpha_denom);
        if (alpha_denom == 0.0) {
            this->converged = 0;
            this->final_iter = 0;
            this->final_norm = sqrt(alpha_nom);
            return ;
        }
    }

    // start iteration
    this->converged = 0;
    int & iter = this->final_iter;
    iter = 1;
    for ( ; true ; ) {
        double alpha = alpha_nom / alpha_denom;
        // 更新解
        vec_add(x,  alpha,  p, x);
        vec_add(r, -alpha, Ap, r);

        if (this->prec) {
            this->prec->Mult(r, Ap);
            beta_nom = this->Dot(r, Ap);
        } else {
            beta_nom = this->Dot(r, r);
        }

        if (beta_nom < 0.0) {
            if (my_pid == 0)
                printf("WARNING: PCG: The preconditioner is not positive definite. (Br, r) = %.16e\n", (double)beta_nom);
            this->converged = 0;
            break;
        }

        if (my_pid == 0)
            printf("   Iteration : %3d  (B r, r) = %.16e\n", iter, beta_nom);
        // 判敛
        if (beta_nom <= threshold) {// 要么残差到位了
            if (my_pid == 0) 
                printf("   Iteration : %3d  (B r, r) = %.16e\n", iter, beta_nom);
            this->converged = 1;
            break ;
        }
        if (++iter > this->max_iter)// 要么已到最大次数
            break;;

        double beta = beta_nom / alpha_nom;
        // 更新搜索方向
        if (this->prec)
            vec_add(Ap, beta, p, p);
        else
            vec_add( r, beta, p, p);

        this->oper->Mult(p, Ap);

        alpha_denom = this->Dot(p, Ap);
        if (alpha_denom <= 0.0) {
            double dd = this->Dot(p, p);
            if (dd > 0.0) {
                if (my_pid == 0) printf("WARNING: PCG: The operator is not positive definite. (Ad, d) = %.16e\n", (double)alpha_denom);
            }
            if (alpha_denom == 0.0) break;
        }
        alpha_nom = beta_nom;// beta的分母也就是alpha的分子
    }

    if (!this->converged) {
        if (my_pid == 0) {
            printf("   Iteration : %3d  (B r, r) = %.16e\n",         0       , nom0);
            printf("   Iteration : %3d  (B r, r) = %.16e\n", this->final_iter, beta_nom);
            printf("PCG: No convergence!\n");
        }
    }
    if (my_pid == 0)
        printf("Average reduction factor = %.8e\n", pow(beta_nom/nom0, 0.5/this->final_iter));
    this->final_norm = sqrt(beta_nom);
}
#endif

#endif