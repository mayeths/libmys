#ifndef GRAPES_OPERATOR_HPP
#define GRAPES_OPERATOR_HPP

// 虚基类Operator，需要子类重写Mult()，如有必要还需重写析构函数
template<typename idx_t, typename data_t>
class Operator {
public:
    idx_t  input_dim[3];
    idx_t output_dim[3];

    explicit Operator(idx_t s = 0) : input_dim{s, s, s}, output_dim{s, s, s} {  }
    Operator(idx_t in0, idx_t in1, idx_t in2, idx_t out0, idx_t out1, idx_t out2) 
        : input_dim{in0, in1, in2}, output_dim{out0, out1, out2} {  }
    virtual void Mult(const par_structVector<idx_t, data_t> & input, par_structVector<idx_t, data_t> & output) const = 0;
    virtual ~Operator() {}
};

// 虚基类Solver，继承自虚基类Operator
// 需要子类重写SetOperator()和Mult()，如有必要还需重写析构函数
template<typename idx_t, typename data_t>
class Solver : public Operator<idx_t, data_t> {
public:
    bool iterative_mode;

    explicit Solver(idx_t s = 0, bool iter_mode = false) : Operator<idx_t, data_t>(s) {iterative_mode = iter_mode;} 
    Solver(idx_t in0, idx_t in1, idx_t in2, idx_t out0, idx_t out1, idx_t out2, bool iter_mode = false) :
        Operator<idx_t, data_t>(in0, in1, in2, out0, out1, out2) {iterative_mode = iter_mode; }
    /// Set/update the solver for the given operator.
    virtual void SetOperator(const Operator<idx_t, data_t> & op) = 0;
};

#endif