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
class PIPECGSolver : public ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>
{
public:
    using BASE = ISS<matrix_t, vector_t, index_t, data_t, pcdata_t>;
    using VType = typename BASE::VType;
    using AType = typename BASE::AType;
    using BType = typename BASE::BType;
    PIPECGSolver() : BASE() { }
    PIPECGSolver(AType &A) : BASE(A) { }
    PIPECGSolver(AType &A, BType &B) : BASE(A, B) { }

    void Apply(const VType &b, VType &x, bool xzero = false) const
    {
        DEBUG(0, "PIPECGSolver Apply");
        ASSERT_NE(this->A, nullptr);
        const AType &A = *(this->A);
        const BType &B = *(this->B);
        VType r(x), z(x), p(x), n(x), w(x), q(x), u(x), m(x), s(x);

        index_t &iter = this->final_iter;

        BLAS2::mult(A, x, u); // u <- Ax
        BLAS1::waxpy(r, -1, u, b); // r <- b - u
        intermediate_t bnorm = BLAS1::norm(b);
        intermediate_t rnorm = BLAS1::norm(r);

        DEBUG(0, "Initial ||r|| %.17e ||r||/||b|| %.17e", iter, rnorm, rnorm / bnorm);

        this->B->Apply(r, u); // u <- Br

        intermediate_t gamma = BLAS1::dot(r, u); // gamma <- u*r
        BLAS2::mult(A, u, w); // w <- Au

        iter = 0;

        intermediate_t gammaold = gamma;
        do {
            if (iter > 0) {
                rnorm = BLAS1::norm(r);
            }
            if (!(iter == 0)) {
                gamma = BLAS1::dot(r, u);
            }
            intermediate_t delta = BLAS1::dot(w, u);
            this->B->Apply(w, m); // m <- Bw
            BLAS2::mult(A, m, n); // n <- Am

            DEBUG(0, "iter %4d ||r|| %.17e ||r||/||b|| %.17e", iter, rnorm, rnorm / bnorm);
            if (rnorm < this->atol) {
                break;
            }

            intermediate_t alpha, beta;
            if (iter == 0) {
                alpha = gamma / delta;
                BLAS1::copy(n, z);
                BLAS1::copy(m, q);
                BLAS1::copy(u, p);
                BLAS1::copy(w, s);
            } else {
                beta = gamma / gammaold;
                alpha = gamma / (delta - beta / alpha * gamma);
                BLAS1::aypx(z, beta, n);
                BLAS1::aypx(q, beta, m);
                BLAS1::aypx(p, beta, u);
                BLAS1::aypx(s, beta, w);
            }
            BLAS1::axpy(x,  alpha, p);
            BLAS1::axpy(u, -alpha, q);
            BLAS1::axpy(w, -alpha, z);
            BLAS1::axpy(r, -alpha, s);
            gammaold = gamma;
            iter += 1;
        } while (iter < this->maxiter);




        // for (; iter < this->maxiter; iter++) {
        //     // DEBUG(0, "CG 5.0"); BARRIER();
        //     BLAS2::mult(A, d, Ad);
        //     // DEBUG(0, "CG 6.0"); BARRIER();
        //     d_dot_Ad = BLAS1::dot(Ad, d);

        //     // DEBUG(0, "CG 7.0"); BARRIER();
        //     double alpha = old_r_dot_p / d_dot_Ad;
        //     BLAS1::axpy(x, alpha, d);
        //     // DEBUG(0, "CG 8.0"); BARRIER();
        //     BLAS1::axpy(r, -alpha, Ad);

        //     // DEBUG(0, "CG 9.0"); BARRIER();
        //     norm_r = BLAS1::norm(r);
        //     if (MYRANK() == 0) printf("iter %4d  ||r||/||b|| = %.17e  ||r|| %.17e\n", 
        //                         iter, (double)(norm_r / norm_b), (double) norm_r);
        //     if (norm_r / norm_b <= this->rtol || norm_r <= this->atol) {
        //         this->converged = 1;
        //         break;
        //     }

        // // DEBUG(0, "CG 10.0"); BARRIER();
        // if (this->B) {
        //     this->B->Apply(r, p);
        // } else {
        //     THROW_NOT_IMPL();
        // }
        // // DEBUG(0, "CG 11.0"); BARRIER();
        //     r_dot_p = BLAS1::dot(r, p);
        // // DEBUG(0, "CG 12.0"); BARRIER();
        //     intermediate_t beta = r_dot_p / old_r_dot_p;
        // // DEBUG(0, "CG 13.0"); BARRIER();
        //     BLAS1::aypx(d, beta, p);
        //     // BLAS2::mult(A, d, p);

        //     // d_dot_Ad = BLAS1::dot(d, p);
        //     // if (d_dot_Ad <= 0.0) {
        //     //     double dd = BLAS1::dot(d, d);
        //     //     if (dd > 0.0)  printf("WARNING: PCG: The operator is not positive definite. (Ad, d) = %.5e\n", (double)d_dot_Ad);
        //     //     if (d_dot_Ad == 0.0) break;
        //     // }
        //     old_r_dot_p = r_dot_p;
        // }
    }

};
