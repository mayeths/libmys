#pragma once

template<typename data_t>
class async
{
public:
    using AwaitFunction = data_t(*)(const async *);
private:
    mutable data_t val;
    mutable AwaitFunction waitfn = nullptr;
public:
    const void *context1 = nullptr;
    const void *context2 = nullptr;

    async(data_t val = 0, const void *ctx1 = nullptr, const void *ctx2 = nullptr, AwaitFunction waitfn = nullptr) {
        this->val = val;
        this->context1 = ctx1;
        this->context2 = ctx2;
        this->waitfn = waitfn;
    }

    data_t await() const {
        if (this->waitfn != nullptr) {
            this->val = this->waitfn(this);
            this->waitfn = nullptr;
        }
        return this->val;
    }

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
