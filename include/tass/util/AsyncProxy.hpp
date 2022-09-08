#pragma once

template<typename data_t>
class AsyncProxy
{
public:
    using AwaitFunction = data_t(*)(const AsyncProxy *);
private:
    mutable data_t val;
    mutable AwaitFunction waitfn = nullptr;
    mutable const void *ctx = nullptr;
public:

    AsyncProxy(data_t val = 0, const void *ctx = nullptr, AwaitFunction waitfn = nullptr) {
        this->val = val;
        this->ctx = ctx;
        this->waitfn = waitfn;
    }

    ~AsyncProxy() {
        this->await();
    }

    static void swap(AsyncProxy &src, AsyncProxy &dst) {
        if (&src != &dst) dst.~AsyncProxy(); else return;
        std::swap(src.val, dst.val);
        std::swap(src.waitfn, dst.waitfn);
        std::swap(src.ctx, dst.ctx);
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

    /* No copy rules */
    AsyncProxy(const AsyncProxy &src) = delete;
    AsyncProxy& operator=(const AsyncProxy &src) = delete;
    /* Only move rules */
    AsyncProxy(AsyncProxy&& src) noexcept { AsyncProxy::swap(src, *this); }
    AsyncProxy& operator=(AsyncProxy&& src) noexcept { AsyncProxy::swap(src, *this); return *this; }
};


template<typename data_t>
class SyncProxy : public AsyncProxy<data_t>
{
public:
    using BASE = AsyncProxy<data_t>;
    using AwaitFunction = typename BASE::AwaitFunction;
    SyncProxy(data_t val = 0, const void *ctx = nullptr, AwaitFunction waitfn = nullptr) : AsyncProxy<data_t>(val, ctx, waitfn) {
        this->await();
    }

    /* No copy rules */
    SyncProxy(const SyncProxy &src) = delete;
    SyncProxy(const BASE &src) = delete;
    SyncProxy& operator=(const SyncProxy &src) = delete;
    SyncProxy& operator=(const BASE &src) = delete;
    /* Only move rules */
    SyncProxy(SyncProxy&& src) noexcept { BASE::swap(src, *this); this->await(); }
    SyncProxy(BASE&& src) noexcept { BASE::swap(src, *this); this->await(); }
    SyncProxy& operator=(SyncProxy&& src) noexcept { BASE::swap(src, *this); this->await(); return *this; }
    SyncProxy& operator=(BASE&& src) noexcept { BASE::swap(src, *this); this->await(); return *this; }
};
