#ifndef GRAPES_TRIDIAG_HPP
#define GRAPES_TRIDIAG_HPP

#include "seq_struct_mv.hpp"

/* Fortran style (1-based):
       _                                                _   _      _     _      _
      |  b(1)  c(1)                                      | | x(1  ) |   | d(1  ) |
      |  a(2)  b(2)  c(2)                                | | x(2  ) |   | d(2  ) |
      |        a(3)  b(3)  c(3)                          | | x(3  ) |   | d(3  ) |
      |           ...  ...  ...                          | | ...    | = | ...    |
      |                ...  ...  ...                     | | ...    |   | ...    |
      |                          a(n-1)  b(n-1)  c(n-1)  | | x(n-1) |   | d(n-1) |
      |_                                 a(n  )  b(n  ) _| |_x(n  )_|   |_d(n  )_|
    */

template<typename idx_t, typename data_t>
class TridiagSolver {
private:
    idx_t neqn;
    data_t * a = nullptr, * b = nullptr, * c = nullptr;
public:
    bool periodic = false;
    TridiagSolver(bool is_periodic = false) : periodic(is_periodic) { }
    void alloc(idx_t neqn);
    void Setup();
    void Solve(data_t * rhs, data_t * sol);
    data_t * Get_a() {return a;}
    data_t * Get_b() {return b;}
    data_t * Get_c() {return c;}
    idx_t Get_neqn() {return neqn;}
    ~TridiagSolver();
};

template<typename idx_t, typename data_t>
TridiagSolver<idx_t, data_t>::~TridiagSolver() {
    if (a) delete a;
    if (b) delete b;
    if (c) delete c;
}

template<typename idx_t, typename data_t>
void TridiagSolver<idx_t, data_t>::alloc(idx_t neqn) {
    this->neqn = neqn;

    a = new data_t [neqn];
    b = new data_t [neqn];
    c = new data_t [neqn];
}

template<typename idx_t, typename data_t>
void TridiagSolver<idx_t, data_t>::Setup()
{
    if (!periodic) {
        assert(a[   0    ] == 0.0);
        assert(c[neqn - 1] == 0.0);
        // 系数分解
        c[0] /= b[0];
        for (idx_t i = 1; i < neqn; i++) {
            b[i] = b[i] - a[i] * c[i - 1];
            c[i] = c[i] / b[i];
        }
    }
    else {
        printf("NOT implemented periodic TridiagSolver\n");
        MPI_Abort(MPI_COMM_WORLD, -4000);
    }
}

template<typename idx_t, typename data_t>
void TridiagSolver<idx_t, data_t>::Solve(data_t * rhs, data_t * sol) {
    
    // 前代
    rhs[0] /= b[0];
    for (idx_t i = 1; i < neqn; i++) 
        rhs[i] = (rhs[i] - a[i] * rhs[i - 1]) / b[i];
    
    
    // 回代
    sol[neqn - 1] = rhs[neqn - 1];
    for (idx_t i = neqn - 2; i >= 0; i--)
        sol[i] = rhs[i] - c[i] * sol[i + 1];
}

template<typename idx_t, typename data_t>
void extract_vals_from_mat(const seq_structMatrix<idx_t, data_t> & mat, const char dir, TridiagSolver<idx_t, data_t> * solvers) 
{
#define MATIDX(d, k, i, j) (d) + (k) * mat.num_diag + (i) * mat.slice_dk_size + (j) * mat.slice_dki_size
    switch (dir)
    {
    case 'z':
        assert(solvers[0].Get_neqn() == mat.local_z + 2 * mat.halo_z);
        for (idx_t j = 0; j < mat.local_y; j++) {
            idx_t real_j = j + mat.halo_y;
            for (idx_t i = 0; i < mat.local_x; i++) {
                idx_t real_i = i + mat.halo_x;
                data_t * a = solvers[j * mat.local_x + i].Get_a();
                data_t * b = solvers[j * mat.local_x + i].Get_b();
                data_t * c = solvers[j * mat.local_x + i].Get_c();

                const idx_t kbeg = mat.halo_z, kend = kbeg + mat.local_z;
                assert(solvers[0].Get_neqn() == kend - kbeg);
                for (idx_t k = kbeg; k < kend; k++) {
                    c[k] = mat.data[MATIDX(14, k, real_i, real_j)];
                    b[k] = mat.data[MATIDX( 0, k, real_i, real_j)];
                    a[k] = mat.data[MATIDX( 9, k, real_i, real_j)];
                }
            }
        }
        break;
    case 'x':
        assert(solvers[0].Get_neqn() == mat.local_x + 2 * mat.halo_x);
        for (idx_t j = 0; j < mat.local_y; j++) {
            idx_t real_j = j + mat.halo_y;
            for (idx_t k = 0; k < mat.local_z; k++) {
                idx_t real_k = k + mat.halo_z;

                data_t * a = solvers[j * mat.local_x + k].Get_a();
                data_t * b = solvers[j * mat.local_x + k].Get_b();
                data_t * c = solvers[j * mat.local_x + k].Get_c();

                const idx_t ibeg = mat.halo_x, iend = ibeg + mat.local_x;
                assert(solvers[0].Get_neqn() == iend - ibeg);
                for (idx_t i = ibeg; i < iend; i++) {
                    c[i] = mat.data[MATIDX( 2, real_k, i, real_j)];
                    b[i] = mat.data[MATIDX( 0, real_k, i, real_j)];
                    a[i] = mat.data[MATIDX( 1, real_k, i, real_j)];
                }
            }
        }
        break;
    case 'y':
    default:
        printf("INVALID dir of %c to extract vals from matrix\n", dir);
        MPI_Abort(MPI_COMM_WORLD, -4000);
        break;
    }
#undef MATIDX
}


template<typename idx_t, typename data_t>
void tridiag_thomas(data_t * a, data_t * b, data_t * c, data_t * d, data_t * sol, idx_t neqn)
{
    /* Fortran style (1-based):
       _                                                _   _      _     _      _
      |  b(1)  c(1)                                      | | x(1  ) |   | d(1  ) |
      |  a(2)  b(2)  c(2)                                | | x(2  ) |   | d(2  ) |
      |        a(3)  b(3)  c(3)                          | | x(3  ) |   | d(3  ) |
      |           ...  ...  ...                          | | ...    | = | ...    |
      |                ...  ...  ...                     | | ...    |   | ...    |
      |                          a(n-1)  b(n-1)  c(n-1)  | | x(n-1) |   | d(n-1) |
      |_                                 a(n  )  b(n  ) _| |_x(n  )_|   |_d(n  )_|
    */

    c[0] /= b[0];
    d[0] /= b[0];
    for (idx_t i = 1; i < neqn; i++) {
        data_t denom = b[i] - a[i] * c[i - 1];
        c[i] /= denom;
        d[i] = (d[i] - a[i] * d[i - 1]) / denom;
    }

    sol[neqn - 1] = d[neqn - 1];
    for (idx_t i = neqn - 2; i >= 0; i--)
        sol[i] = d[i] - c[i] * sol[i + 1];
}

#endif