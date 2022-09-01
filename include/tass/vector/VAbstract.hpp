#pragma once

template<typename data_t>
class async
{
public:
    using AwaitFunction = data_t(*)(const async *);
private:
    mutable data_t val;
    mutable AwaitFunction waitfn = nullptr;
    mutable const void *ctx = nullptr;
public:

    async(data_t val = 0, const void *ctx = nullptr, AwaitFunction waitfn = nullptr) {
        this->val = val;
        this->ctx = ctx;
        this->waitfn = waitfn;
    }

    data_t await() const {
        if (this->waitfn != nullptr) {
            this->val = this->waitfn(this);
            this->waitfn = nullptr;
            this->ctx = nullptr;
        }
        return this->val;
    }

    const void *context() const { return this->ctx; }

    operator data_t() const {
        return this->await();
    }
};

template<typename index_t, typename data_t>
class VAbstract
{
public:

    explicit VAbstract() { }
    explicit VAbstract(const VAbstract&) { } /* Copy */
    explicit VAbstract(VAbstract&&) { } /* Move */

};
