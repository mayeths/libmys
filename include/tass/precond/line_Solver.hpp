#ifndef GRAPES_LINE_SOLVER_HPP
#define GRAPES_LINE_SOLVER_HPP

#include "precond.hpp"
#include "../utils/par_struct_mat.hpp"
#include "../utils/tridiag.hpp"

template<typename idx_t, typename data_t>
class LineSolver : public Solver<idx_t, data_t> {
public:
    // when used as precondition, just do once, and zero-val optimization
    // could be exploited 
    bool used_as_precond = true;
    // direction to update vals simultaneously (default as 0)
    // 0 : inner-most (vertical), 1: middle, 2: outer-most
    char line_dir = 'z';

    mutable data_t weight = 1.0;

    // operator (often as matrix-A)
    const Operator<idx_t, data_t> * oper = nullptr;
    TridiagSolver<idx_t, data_t> * tri_solver = nullptr;

    LineSolver(char line_dir = 'z') : Solver<idx_t, data_t>(0, true), line_dir(line_dir) {  }

    virtual void SetRelaxWeight(data_t wt) {weight = wt; } 

    virtual void SetOperator(const Operator<idx_t, data_t> & op) {
        oper = & op;

        this->input_dim[0] = op.input_dim[0];
        this->input_dim[1] = op.input_dim[1];
        this->input_dim[2] = op.input_dim[2];

        this->output_dim[0] = op.output_dim[0];
        this->output_dim[1] = op.output_dim[1];
        this->output_dim[2] = op.output_dim[2];

        // 根据松弛方向进行setup
        Setup();
    }

    virtual void Setup();

    virtual ~LineSolver();
};

template<typename idx_t, typename data_t>
LineSolver<idx_t, data_t>::~LineSolver() {
    if (tri_solver != nullptr) delete[] tri_solver;
}

template<typename idx_t, typename data_t>
void LineSolver<idx_t, data_t>::Setup() 
{
    assert(this->oper != nullptr);

    const seq_structMatrix<idx_t, data_t> * mat = ((par_structMatrix<idx_t, data_t>*)(this->oper))->local_matrix;
    idx_t tot_solvers, neqn;
    switch (line_dir)
    {
    case 'z':
        tot_solvers = mat->local_x * mat->local_y;
        neqn = mat->local_z;
        break;
    case 'x':
        tot_solvers = mat->local_z * mat->local_y;
        neqn = mat->local_x;
        break;
    case 'y':
    default:
        printf("INVALID line_dir of %c to setup TridiagSolver\n", line_dir);
        MPI_Abort(MPI_COMM_WORLD, -4000);
        break;
    }

    tri_solver = new TridiagSolver<idx_t, data_t> [tot_solvers];
    for (idx_t i = 0; i < tot_solvers; i++) {
        tri_solver[i].periodic = (line_dir == 'z') ? false : true;
        tri_solver[i].alloc(neqn);
    }

    extract_vals_from_mat(*mat, line_dir, tri_solver);

    for (idx_t i = 0; i < tot_solvers; i++) {
        tri_solver[i].Setup();
    }
}

#endif