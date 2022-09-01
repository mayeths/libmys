#pragma once

#include "ISSAbstract.hpp"
#include "../blas/BLAS1.hpp"
#include "../blas/BLAS2.hpp"

template<
    typename matrix_t,
    typename vector_t,
    typename index_t = int,
    typename data_t = double,
    typename pcdata_t = data_t,       /* preconditioner data_t */
    typename intermediate_t = data_t> /* intermediate data_t */
class CGSolver : public ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
public:
    using BASE = ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    CGSolver() : BASE() { }
    CGSolver(AType &A) : BASE(A) { }
    CGSolver(AType &A, BType &B) : BASE(A, B) { }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        ASSERT_NE(this->A, nullptr);
        const AType &A = *(this->A);
        const BType &B = *(this->B);
        VType r(x), p(x), d(x), Ad(x);

        if (xzero) {
            THROW_NOT_IMPL();
        } else {
            BLAS2::mult(A, x, d);
            BLAS1::waxpy(r, -1, d, b);
        }

        intermediate_t norm_b = BLAS1::norm(b);
        intermediate_t norm_r = BLAS1::norm(r);
        intermediate_t old_r_dot_p, d_dot_Ad, r_dot_p;

        index_t &iter = this->iter;
        iter = 0;

        if (MYRANK() == 0) printf("iter %4d %21s ||r||/||b|| = %20.16e\n", iter, "", (double)(norm_r / norm_b));

        if (this->B) {
            // DEBUG(0, "CG 1.0"); BARRIER();
            this->B->Apply(r, p);
            // DEBUG(0, "CG 2.0"); BARRIER();
            BLAS1::copy(p, d);
            // DEBUG(0, "CG 3.0"); BARRIER();
        } else {
            THROW_NOT_IMPL();
        }
        iter++;

        // DEBUG(0, "CG 4.0"); BARRIER();
        old_r_dot_p   = BLAS1::dot(r, p);

        for (; iter < this->maxiter; iter++) {
            // DEBUG(0, "CG 5.0"); BARRIER();
            BLAS2::mult(A, d, Ad);
            // DEBUG(0, "CG 6.0"); BARRIER();
            d_dot_Ad = BLAS1::dot(Ad, d);

            // DEBUG(0, "CG 7.0"); BARRIER();
            double alpha = old_r_dot_p / d_dot_Ad;
            BLAS1::axpy(x, alpha, d);
            // DEBUG(0, "CG 8.0"); BARRIER();
            BLAS1::axpy(r, -alpha, Ad);

            // DEBUG(0, "CG 9.0"); BARRIER();
            norm_r = BLAS1::norm(r);
            if (MYRANK() == 0) printf("iter %4d  ||r||/||b|| = %.17e  ||r|| %.17e\n", 
                                iter, (double)(norm_r / norm_b), (double) norm_r);
            if (norm_r / norm_b <= this->rtol || norm_r <= this->atol) {
                this->converged = 1;
                break;
            }

        // DEBUG(0, "CG 10.0"); BARRIER();
        if (this->B) {
            this->B->Apply(r, p);
        } else {
            THROW_NOT_IMPL();
        }
        // DEBUG(0, "CG 11.0"); BARRIER();
            r_dot_p = BLAS1::dot(r, p);
        // DEBUG(0, "CG 12.0"); BARRIER();
            intermediate_t beta = r_dot_p / old_r_dot_p;
        // DEBUG(0, "CG 13.0"); BARRIER();
            BLAS1::aypx(d, beta, p);
            // BLAS2::mult(A, d, p);

            // d_dot_Ad = BLAS1::dot(d, p);
            // if (d_dot_Ad <= 0.0) {
            //     double dd = BLAS1::dot(d, d);
            //     if (dd > 0.0)  printf("WARNING: PCG: The operator is not positive definite. (Ad, d) = %.5e\n", (double)d_dot_Ad);
            //     if (d_dot_Ad == 0.0) break;
            // }
            old_r_dot_p = r_dot_p;
        }
    }

};
