#ifndef GRAPES_GMRES_HPP
#define GRAPES_GMRES_HPP

#include "iter_solver.hpp"
#include "../utils/par_struct_mat.hpp"

template<typename idx_t, typename ksp_t, typename pc_t>
class GMRESSolver : public IterativeSolver<idx_t, ksp_t, pc_t> {
public:
    int restart_len = 10;

    GMRESSolver() {  }
    virtual void SetRestartlen(int num) { restart_len = num; }
    virtual void Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const ;
};

// 注意这里是引用，原位修改值了
void GeneratePlaneRotation(double &dx, double &dy, double &cs, double &sn)
{
   if (dy == 0.0)
   {
      cs = 1.0;
      sn = 0.0;
   }
   else if (fabs(dy) > fabs(dx))
   {
      double temp = dx / dy;
      sn = 1.0 / sqrt( 1.0 + temp*temp );
      cs = temp * sn;
   }
   else
   {
      double temp = dy / dx;
      cs = 1.0 / sqrt( 1.0 + temp*temp );
      sn = temp * cs;
   }
}

void ApplyPlaneRotation(double &dx, double &dy, double &cs, double &sn)
{
   double temp = cs * dx + sn * dy;
   dy = -sn * dx + cs * dy;
   dx = temp;
}

#define arrId(i,j) ((i)*restart_len + j)

template<typename idx_t, typename data_t>
void Update(par_structVector<idx_t, data_t> & x, int k, double* h, double * s, 
        par_structVector<idx_t, data_t> * v, int restart_len, double * aux_y)
{
    for (int i = 0; i <= restart_len; i++)// 对应 Vector y(s);
        aux_y[i] = s[i];

    // Backsolve:
    for (int i = k; i >= 0; i--) {
        aux_y[i] /= h[arrId(i, i)];
        for (int j = i - 1; j >= 0; j--)
            aux_y[j] -= h[arrId(j, i)] * aux_y[i];
    }

    for (int j = 0; j <= k; j++) {
        vec_add(x, aux_y[j], v[j], x);// x.Add(y(j), *v[j]);
    }
}

// 抄的hypre的GMRES，比较麻烦，精简了一点
template<typename idx_t, typename ksp_t, typename pc_t>
void GMRESSolver<idx_t, ksp_t, pc_t>::Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const 
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(this->restart_len > 0);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);// 如果不满足，可能会有问题？从之前的向量构造的规范性上应该是满足的

    MPI_Comm comm = x.comm_pkg->cart_comm;
    int my_pid; MPI_Comm_rank(comm, &my_pid);

    double * rs = new double [restart_len + 1];
    double *  c = new double [restart_len];
    double *  s = new double [restart_len];
    double * rs2= new double [restart_len + 1];
    double ** hh = new double * [restart_len + 1];
    for (int i = 0; i <= restart_len; i++)
        hh[i] = new double [restart_len];

    // 初始化辅助向量
    par_structVector<idx_t, ksp_t> r(x), w(x);
    std::allocator<par_structVector<idx_t, ksp_t> > alloc;
    par_structVector<idx_t, ksp_t> * p = alloc.allocate(restart_len + 1);
    for (int i = 0; i <= restart_len; i++)
        alloc.construct(p + i, x);

    // compute initial residual
    this->oper->Mult(x, p[0]);
    vec_add(b, -1.0, p[0], p[0]);

    double b_norm = this->Norm(b);
    double real_r_norm_old = b_norm, real_r_norm_new;
    double r_norm = this->Norm(p[0]);
    double r_norm_0 = r_norm;

    if (my_pid == 0) {
        printf("L2 norm of b: %20.16e\n", b_norm);
        printf("Initial L2 norm of residual: %20.16e\n", r_norm);
    }

    int & iter = this->final_iter = 0;
    double den_norm;
    if (b_norm > 0.0)   den_norm = b_norm;// convergence criterion |r_i|/|b| <= accuracy if |b| > 0
    else                den_norm = r_norm;// convergence criterion |r_i|/|r0| <= accuracy if |b| = 0

    /*  convergence criteria: |r_i| <= max( a_tol, r_tol * den_norm)
        den_norm = |r_0| or |b|
        note: default for a_tol is 0.0, so relative residual criteria is used unless
        user specifies a_tol, or sets r_tol = 0.0, which means absolute
        tol only is checked  */
    double epsilon;
    epsilon = std::max(this->abs_tol, this->rel_tol * den_norm);

    // so now our stop criteria is |r_i| <= epsilon
    if (my_pid == 0) {
        if (b_norm > 0.0) {
            printf("=========================================================\n\n");
            printf("Iters             resid.norm               rel.res.norm  \n");
            printf("------       -------------------      --------------------\n");
        } else {
            printf("=============================================\n\n");
            printf("Iters     resid.norm     \n");
            printf("------    ------------    \n");
        }
    }

    /* once the rel. change check has passed, we do not want to check it again */
    bool rel_change_passed = false;
    while (iter < this->max_iter) {
        /* initialize first term of hessenberg system */
        rs[0] = r_norm;
        if (r_norm == 0.0) {
            this->converged = 1;
            goto finish;
        }

        /* see if we are already converged and 
           should print the final norm and exit */
        if (r_norm <= epsilon) {
            this->oper->Mult(x, r);
            vec_add(b, -1.0, r, r);
            r_norm = this->Norm(r);
            if (r_norm <= epsilon) {
                if (my_pid == 0)
                    printf("\nFinal L2 norm of residual: %20.16e\n\n", r_norm);
                break;
            } else {
                if (my_pid == 0) 
                    printf("false convergence\n");
            }
        }

        vec_scale(1.0 / r_norm, p[0]);
        int i = 0;
        while (i < restart_len && iter < this->max_iter) {
            i++;
            iter++;

            if (this->prec) this->prec->Mult(p[i-1], r);
            else                    vec_copy(p[i-1], r);

            this->oper->Mult(r, p[i]);

            /* modified Gram_Schmidt */
            for (int j = 0; j < i; j++) {
                hh[j][i-1] = this->Dot(p[j], p[i]);
                vec_add(p[i], -hh[j][i-1], p[j], p[i]);
            }

            double t = this->Norm(p[i]);
            hh[i][i-1] = t;	
            if (t != 0.0) vec_scale(1.0 / t, p[i]);

            /* done with modified Gram_schmidt and Arnoldi step.
                update factorization of hh */
            for (int j = 1; j < i; j++) {
                double t = hh[j-1][i-1];
                hh[j-1][i-1] = s[j-1]*hh[j][i-1] + c[j-1] * t;
                hh[j][i-1]   = -s[j-1]*t + c[j-1]*hh[j][i-1];
            }
            t = hh[i][i-1] * hh[i][i-1];
            t += hh[i-1][i-1] * hh[i-1][i-1];
            double gamma = sqrt(t);
            if (gamma == 0.0) gamma = 1.0e-16;
            c[i-1] = hh[i-1][i-1]/gamma;
            s[i-1] = hh[i][i-1]/gamma;
            rs[i] = -hh[i][i-1]*rs[i-1];
            rs[i]/=  gamma;
            rs[i-1] = c[i-1]*rs[i-1];
            /* determine residual norm */
            hh[i-1][i-1] = s[i-1]*hh[i][i-1] + c[i-1]*hh[i-1][i-1];
            r_norm = fabs(rs[i]);

            if (my_pid == 0) {
                if (b_norm > 0.0)
                    printf("%5d    %.16e   %.16e\n", iter, r_norm, r_norm/b_norm);
                else
                    printf("%5d    %.16e\n", iter, r_norm);
            }

            if (r_norm <= epsilon) break;
        }/*** end of restart cycle ***/

        /* now compute solution, first solve upper triangular system */
        rs[i-1] = rs[i-1]/hh[i-1][i-1];
        for (int k = i-2; k >= 0; k--)
        {
            double t = 0.0;
            for (int j = k+1; j < i; j++) {
                t -= hh[k][j]*rs[j];
            }
            t += rs[k];
            rs[k] = t/hh[k][k];
        }

        vec_mul_by_scalar(rs[i-1], p[i-1], w);
        for (int j = i-2; j >= 0; j--)
            vec_add(w, rs[j], p[j], w);

        if (this->prec) this->prec->Mult(w, r);
        else                    vec_copy(w, r);

        /* update current solution x (in x) */
        vec_add(x, 1.0, r, x);

        /* check for convergence by evaluating the actual residual */
        if (r_norm <= epsilon) {
            this->oper->Mult(x, r);
            vec_add(b, -1.0, r, r);
            real_r_norm_new = r_norm = this->Norm(r);

            if (r_norm <= epsilon) {
                if (my_pid == 0)
                    printf("\nFinal L2 norm of residual: %20.16e\n\n", r_norm);
                goto finish;
            } else {/* conv. has not occurred, according to true residual */
                /* exit if the real residual norm has not decreased */
                if (real_r_norm_new >= real_r_norm_old) {
                    if (my_pid == 0)
                        printf("\nFinal L2 norm of residual: %20.16e\n\n", r_norm);
                    this->converged = 1;
                    break;
                }

                /* report discrepancy between real/GMRES residuals and restart */
                if (my_pid == 0)
                    printf("false convergence 2, L2 norm of residual: %20.16e\n", r_norm);

                vec_copy(r, p[0]);
                i = 0;
                real_r_norm_old = real_r_norm_new;
            }
        }/* end of convergence check */

        /* compute residual vector and continue loop */
	    for (int j=i ; j > 0; j--) {
            rs[j-1] = -s[j-1]*rs[j];
            rs[j] = c[j-1]*rs[j];
    	}
        
        if (i) vec_add(p[i], rs[i] - 1.0, p[i], p[i]);

        for (int j=i-1 ; j > 0; j--)
            vec_add(p[i], rs[j], p[j], p[i]);
        
        if (i) {
            vec_add(p[0], rs[0] - 1.0, p[0], p[0]);
            vec_add(p[0], 1.0, p[i], p[0]);
        }

        if (my_pid == 0) printf("Restarting...\n");
    }/* END of iteration while loop */

finish:
    delete c; delete s; delete rs; delete rs2;
    for (int i = 0; i <= restart_len; i++) delete hh[i];
    delete hh;
    for (int i = 0; i <= restart_len; i++) alloc.destroy(p + i);
    alloc.deallocate(p, restart_len + 1);
}

#if 0 // MFEM版本的GMRES，以预条件后的残差 (B r, r) ，不太准
template<typename idx_t, typename ksp_t, typename pc_t>
void GMRESSolver<idx_t, ksp_t, pc_t>::Mult(par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const 
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(this->restart_len > 0);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);// 如果不满足，可能会有问题？从之前的向量构造的规范性上应该是满足的

    MPI_Comm comm = x.comm_pkg->cart_comm;
    int my_pid; MPI_Comm_rank(comm, &my_pid);

    // 初始化辅助向量
    par_structVector<idx_t, ksp_t> r(x), w(x);
    std::allocator<par_structVector<idx_t, ksp_t> > alloc;
    par_structVector<idx_t, ksp_t> * v = alloc.allocate(restart_len + 1);
    for (int i = 0; i <= restart_len; i++)
        alloc.construct(v + i, x);
    // GMRES 相关的数据结构
    double * H       = (double *) malloc (sizeof(double) * (restart_len + 1) * restart_len);
    double * s       = (double *) malloc (sizeof(double) * (restart_len + 1) );
    double * cs      = (double *) malloc (sizeof(double) * (restart_len + 1) );
    double * sn      = (double *) malloc (sizeof(double) * (restart_len + 1) );
    double * aux_y   = (double *) malloc (sizeof(double) * (restart_len + 1) );

    if (this->iterative_mode) {
        this->oper->Mult(x, r);// r = A * x
    } else {
        x.set_val(0.0);
    }

    if (this->prec) {
        if (this->iterative_mode) {
            vec_add(b, -1.0, r, w);// w = b - r 即为 b - A x 残差
            this->prec->Mult(w, r);// r = M (b - A x) 预条件子，即为误差e
        } else {
            this->prec->Mult(b, r);
        }
    } else {
        if (this->iterative_mode) {// 刚才上面已经算了r = A*x
            vec_add(b, -1.0, r, r);// 此时得到 r = b - A*x
        } else {
            vec_copy(b, r);
        }
    }

    double beta = this->Norm(r);// beta = ||r|| 
    // MFEM_ASSERT(IsFinite(beta), "beta = " << beta);

    this->final_norm = std::max(this->rel_tol * beta, this->abs_tol);

    if (beta <= this->final_norm) {
        this->final_norm = beta;
        this->final_iter = 0;
        this->converged = 1;
        goto finish;
    }
    if (my_pid == 0) printf("   Pass : %3d   Iteration : %3d  ||B r|| = %.16e\n", 1, 0, (double) beta);

    for (int j = 1; j <= this->max_iter; ) {
        vec_mul_by_scalar(1.0/beta, r, v[0]); 
        for (int kk = 0; kk <= restart_len; kk++) s[kk] = 0.0;
        s[0] = beta;

        int i;
        for (i = 0; i < restart_len && j <= this->max_iter; i++, j++) {
            if (this->prec) {
                this->oper->Mult(v[i], r);// r := A*v[i]
                this->prec->Mult(r, w);
            } else {
                this->oper->Mult(v[i], w);
            }

            for (int k = 0; k <= i; k++) {
                H[arrId(k,i)] = this->Dot(w, v[k]);// 可以像GCR一样，多次allreduce合并到一起通信
                vec_add(w, -H[arrId(k,i)], v[k], w);// vec_add(w_high, -H[arrId(k,i)], v[k], w_high, n, NULL);
            }

            H[arrId(i+1, i)] = this->Norm(w);
            // MFEM_ASSERT(IsFinite(H(i+1,i)), "Norm(w) = " << H(i+1,i));

            vec_mul_by_scalar(1.0/H[arrId(i+1, i)], w, v[i+1]);//  vec_mul_by_scalar(w_high, 1.0/H[arrId(i+1, i)], v[i+1], n);

            for (int k = 0; k < i; k++)
                ApplyPlaneRotation(H[arrId(k,i)], H[arrId(k+1,i)], cs[k], sn[k]);

            GeneratePlaneRotation (H[arrId(i,i)], H[arrId(i+1,i)], cs[i], sn[i]);
            ApplyPlaneRotation    (H[arrId(i,i)], H[arrId(i+1,i)], cs[i], sn[i]);
            ApplyPlaneRotation    (s[i], s[i+1], cs[i], sn[i]);

            double resid = fabs(s[i+1]);// 这个resid能跟||r||直接划等号吗？

            // MFEM_ASSERT(IsFinite(resid), "resid = " << resid);
            if (resid <= this->final_norm) {
                Update(x, i, H, s, v, restart_len, aux_y);// 更新解？
                this->final_norm = resid;
                this->final_iter = j;
                this->converged = 1;
                goto finish;
            }

            if (my_pid == 0)
                printf("   Pass : %3d   Iteration : %3d  ||B r|| = %.16e\n", (j-1)/restart_len + 1, j, (double) resid);
        }// i loop

        if (j <= this->max_iter) if (my_pid == 0) printf("Restarting....\n");

        Update(x, i-1, H, s, v, restart_len, aux_y);// 更新解

        this->oper->Mult(x, r);// 更新解之后马上算一遍残差 // 对应 oper->Mult(x, r);
        if (this->prec) {
            vec_add(b, -1.0, r, w);
            this->prec->Mult(w, r);
        } else {
            vec_add(b, -1.0, r, r);
        }

        beta = this->Norm(r);// 判断残差是否到收敛
        // MFEM_ASSERT(IsFinite(beta), "beta = " << beta);
        if (beta <= this->final_norm) {
            this->final_norm = beta;
            this->final_iter = j;
            this->converged = 1;
            goto finish;
        }
    }// j loop

    this->final_norm = beta;
    this->final_iter = this->max_iter;
    this->converged = 0;

finish:
    if (my_pid == 0)
        printf("   Pass : %3d   Iteration : %3d  ||B r|| = %.16e\n", 
            (this->final_iter-1)/restart_len + 1, this->final_iter, (double) this->final_norm);
    
    // check 一下：真实残差||b-A*x||确实是跟resid差距不太大的
    // this->oper->Mult(x, r);
    // vec_add(b, -1.0, r, r);
    // beta = this->Norm(r);
    // if (my_pid == 0) printf("CHECK: final ||r|| = %.16e\n", beta);

    free(H); free(s); free(cs); free(sn); free(aux_y);
    for (int i = 0; i <= restart_len; i++)
        alloc.destroy( v + i);
    alloc.deallocate( v, restart_len + 1);
}
#endif

#undef arrId

#endif