#ifndef GRAPES_ITER_SOLVER_HPP
#define GRAPES_ITER_SOLVER_HPP

#include "../utils/operator.hpp"
#include <memory>

// 虚基类IterativeSolver，支持两种精度
// 继承自虚基类->Solver->Operator，重写了SetOperator()，还需下一层子类重写Mult()
template<typename idx_t, typename ksp_t, typename pc_t>
class IterativeSolver : public Solver<idx_t, ksp_t> {
public:
    // oper(外迭代的算子/矩阵)，可以和prec(预条件子)采用不一样的精度
    const Operator<idx_t, ksp_t> *oper = nullptr;
    Solver<idx_t, pc_t> *prec = nullptr;

    int max_iter = 10, print_level = -1;
    double rel_tol = 0.0, abs_tol = 0.0;// 用高精度的收敛判断

    // stats
    mutable int final_iter, converged;
    mutable double final_norm;

    IterativeSolver() : Solver<idx_t, ksp_t>(0, true) {   }

    void SetRelTol(double rtol) { rel_tol = rtol; }
    void SetAbsTol(double atol) { abs_tol = atol; }
    void SetMaxIter(int max_it) { max_iter = max_it; }
    void SetPrintLevel(int print_lvl) { print_level = print_level; }

    int GetNumIterations() const { return final_iter; }
    int GetConverged() const { return converged; }
    double GetFinalNorm() const { return final_norm; }

    /// This should be called before SetOperator
    virtual void SetPreconditioner(Solver<idx_t, pc_t> & pr) {
        prec = & pr;
        prec->iterative_mode = false;
    }

    /// Also calls SetOperator for the preconditioner if there is one
    virtual void SetOperator(const Operator<idx_t, ksp_t> & op) {
        oper = & op;
        this->input_dim[0] = op.input_dim[0];
        this->input_dim[1] = op.input_dim[1];
        this->input_dim[2] = op.input_dim[2];

        this->output_dim[0] = op.output_dim[0];
        this->output_dim[1] = op.output_dim[1];
        this->output_dim[2] = op.output_dim[2];

        if (prec)
            prec->SetOperator(*oper);
    }

    // 迭代法里的点积（默认采用双精度）
    double Dot(const par_structVector<idx_t, ksp_t> & x, const par_structVector<idx_t, ksp_t> & y) const {
        return vec_dot<idx_t, ksp_t, double>(x, y);
    }
    // 迭代法里的范数（默认采用双精度）
    double Norm(const par_structVector<idx_t, ksp_t> & x) const {
        return sqrt(Dot(x, x));
    }

};

#endif