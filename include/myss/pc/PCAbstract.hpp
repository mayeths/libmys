#pragma once

template<typename matrix_t, typename vector_t, typename index_t, typename data_t>
class PCAbstract
{
public:
    using VType = vector_t;
    using AType = matrix_t; /*MAbstract<index_t, data_t>;*/
    AType *A = nullptr;

    index_t verbose = -1;
    data_t rtol = 1e-15, atol = 1e-6;

    index_t final_iter, converged;
    data_t final_norm;

    PCAbstract() : A(nullptr) { }
    PCAbstract(AType &A) { this->A = &A; }

    void SetRtol(data_t num) { this->rtol = num; }
    void SetAtol(data_t num) { this->atol = num; }
    void SetPrintLevel(index_t num) { this->verbose = num; }

    void SetOperator(const AType &op) { this->A = &op; }

    virtual void Apply(const VType &input, VType &output, bool xzero = false) const = 0;

    friend VType operator*(const PCAbstract &pc, const VType &b) {
        VType x(b);
        pc.Apply(b, x);
        return x;
    }
};
