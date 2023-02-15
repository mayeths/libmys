#pragma once

// https://codereview.stackexchange.com/a/247491
// https://codereview.stackexchange.com/a/248060

#include <string>
#include <stdexcept>

struct guard_t
{
protected:
    bool valid = false;
public:
    void set() { this->valid = true; }
    void reset() { this->valid = false; }
    bool get() const { return this->valid; }
    void ensure(const std::string &error = "Invalid operation on guard_t object") const {
        if (!this->valid)
            throw std::runtime_error(error);
    }
};

template<typename DERIVED, typename T>
class raii
{
protected:
    T res = DERIVED::invalid_res;
    guard_t guard;
    void set(T new_res) {
        this->res = new_res;
        this->guard.set();
    }
    void reset() {
        this->res = DERIVED::invalid_res;
        this->guard.reset();
    }
public:
    T& get() const {
        this->ensure();
        return this->res;
    }
    void ensure() const {
        this->guard.ensure();
    }
};

//////////
// EXAMPLE
//////////
// class VAO_raii : public raii<VAO_raii, unsigned int>
// {
// public:
//     static const unsigned int invalid_res = 0;
//     int num;
//     VAO_raii() {}
//     ~VAO_raii() { this->destroy(); }

//     void create(int num) {
//         unsigned int id;
//         glGenVertexArrays(num, &id);
//         this->set(id);
//         this->num = num;
//     }

//     void destroy() {
//         if (this->valid) {
//             glDeleteVertexArrays(this->num, &this->res);
//         }
//         this->unset();
//     }

// };
