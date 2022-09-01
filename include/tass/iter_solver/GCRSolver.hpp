#ifndef GRAPES_GCR_HPP
#define GRAPES_GCR_HPP

#include "iter_solver.hpp"
#include "../utils/par_struct_mat.hpp"

template<typename idx_t, typename ksp_t, typename pc_t>
class GCRSolver : public IterativeSolver<idx_t, ksp_t, pc_t> {
public:
    // iter_max+1就是重启长度，闭区间[0, iter_max]
    int inner_iter_max = 10;

    GCRSolver() {  }
    virtual void SetInnerIterMax(int num) { inner_iter_max = num; }

    // 求解以b为右端向量的方程组，x为返回的近似解
    virtual void Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const ;
};

template<typename idx_t, typename ksp_t, typename pc_t>
void GCRSolver<idx_t, ksp_t, pc_t>::Mult(const par_structVector<idx_t, ksp_t> & b, par_structVector<idx_t, ksp_t> & x) const 
{
    assert(this->oper != nullptr);
    assert(this->input_dim[0] == x.global_size_x && this->input_dim[1] == x.global_size_y && this->input_dim[2] == x.global_size_z);
    assert(this->output_dim[0]== b.global_size_x && this->output_dim[1]== b.global_size_y && this->output_dim[2]== b.global_size_z);
    assert(this->inner_iter_max > 0);
    assert(b.comm_pkg->cart_comm == x.comm_pkg->cart_comm);// 如果不满足，可能会有问题？从之前的向量构造的规范性上应该是满足的

    MPI_Comm comm = x.comm_pkg->cart_comm;
    int my_pid; MPI_Comm_rank(comm, &my_pid);

    // 初始化辅助向量
    std::allocator<par_structVector<idx_t, ksp_t> > alloc;
    par_structVector<idx_t, ksp_t> * p = alloc.allocate(inner_iter_max + 1), 
                                   *ap = alloc.allocate(inner_iter_max + 1);

    for (int i = 0; i <= inner_iter_max; i++) {
        alloc.construct( p + i, x); // vec_set_val( p[i], 0.0);// 不需要清零
        alloc.construct(ap + i, x); // vec_set_val(ap[i], 0.0);
    }

    par_structVector<idx_t, ksp_t> r(x), ar(x), gcr(x); //vec_set_val( r, 0.0); vec_set_val(ar, 0.0); // 不需要清零

    // 初始化辅助标量
    double res = 0.0;
    double aps[inner_iter_max], 
            c1[inner_iter_max], c2[inner_iter_max], beta[inner_iter_max];

    // 开始计算
    this->oper->Mult(x, r);

    res = this->Dot(r, r);
    if (my_pid == 0) printf("(A*x, A*x): %20.16e\n", res);

    vec_add(b, -1.0, r, r);

    res = this->Dot(r, r);
    if (my_pid == 0) printf("begin of gcr %20.16e\n", res);

    ksp_t weight = 0.8;
    // 在这里做预条件，r => M^{-1}*r，然后用M^{-1}*r去跟矩阵A乘
    if (this->prec) {
        this->prec->Mult(r, gcr);
    } else {
        vec_copy(r, gcr);
    }
    this->oper->Mult(gcr, ar);// ar = A*r

    // p[0] = r, ap[0] = ar
    vec_copy(gcr,  p[0]);
    vec_copy( ar, ap[0]);

    // 主循环
    for (int mm = 0; mm < this->max_iter; mm++) {
        // m: The index of current iteration 注意这版代码里的inner_iter_max要设置得比Fortran代码的少1才能相互匹配
        int m = mm % inner_iter_max;
        if (m == 0 && mm > 0) {// 内迭代归零时，开始下一轮内迭代，拷贝上一轮的最后一个值
            // if (my_pid == 0) printf("\n");
            vec_copy(  p[inner_iter_max],  p[0] );
            vec_copy( ap[inner_iter_max], ap[0] );
        }

        seq_vec_dot( *(    r.local_vector), *(ap[m].local_vector), &c1[0], sizeof(c1[0]));
        seq_vec_dot( *(ap[m].local_vector), *(ap[m].local_vector), &c1[1], sizeof(c1[1]));
        MPI_Allreduce(c1, c2, 2, MPI_dtype<ksp_t>(), MPI_SUM, comm);
        double ac = c2[0] / c2[1];
        aps[m]    = c2[1];

        vec_add(x,  ac,  p[m], x);// 更新解向量
        vec_add(r, -ac, ap[m], r);// 更新残差向量
        double loc_err, glb_err;
        seq_vec_dot( *(r.local_vector), *(r.local_vector), &loc_err, sizeof(loc_err));// 判敛的全局通信可以放到后面的和c1、c2一起通

        // 这里做预条件，r => M^{-1}*r，然后用M^{-1}*r去跟矩阵A乘，得到ar
        if (this->prec) {
            this->prec->Mult(r, gcr);
        } else {
            vec_copy(r, gcr);
        }
        this->oper->Mult(gcr, ar);

        for (int l = 0; l <= m; l++)
            seq_vec_dot(*(ar.local_vector), *((ap[l]).local_vector) , &c1[l], sizeof(c1[l]));

        c1[m + 1] = loc_err;
        MPI_Allreduce(c1, c2, m + 2, MPI_dtype<ksp_t>(), MPI_SUM, comm);// 通信更新[0,1,...,m]共计m+1个数

        glb_err = c2[m + 1];
        if (my_pid == 0) printf("  res of gcr %20.16e at %3d iter\n", glb_err, mm);
        if (glb_err <= this->abs_tol || mm == this->max_iter) goto finish;

        // 计算beta并更新下一次内迭代的向量
        for (int l = 0; l <= m; l++)
            beta[l] = - c2[l] / aps[l];
        vec_copy(gcr,  p[m+1]);
        vec_copy( ar, ap[m+1]);
        for (int l = 0; l <= m; l++) {// 根据[0,1,...,m]共计m+1个向量更新下一次内迭代要使用的序号为m+1的
            vec_add(  p[m+1], beta[l],  p[l],  p[m+1]);
            vec_add( ap[m+1], beta[l], ap[l], ap[m+1]);
        }
    }

finish:
    // 清理数据
    for (int i = 0; i <= inner_iter_max; i++) {
        alloc.destroy( p + i);
        alloc.destroy(ap + i);
    }
    alloc.deallocate( p, inner_iter_max + 1);
    alloc.deallocate(ap, inner_iter_max + 1);
}


#endif