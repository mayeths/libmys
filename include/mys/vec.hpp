#pragma once

#include "mys/headers.hpp"

/* Work in progress */
template <typename index_t = int, typename data_t = double>
struct DV {
private:
    bool valid = true;
public:
    index_t size = 0;
    data_t *data = nullptr;

    DV(): valid(false) {}

    DV(index_t size, data_t *data = nullptr)
        : size(size), data(data), valid(data != nullptr)
    {
    }

    data_t& operator[](index_t i)
    {
        return this->data[i];
    }

    ~DV()
    {
        if (this->data != nullptr) free(this->data);
    }

};
