#pragma once

#include <atomic>
#include <memory>
#include <map>
#include <stdlib.h>
#include <sys/resource.h>
#include <string.h>

/* Memory Barrier */
/* https://support.huaweicloud.com/codeprtr-kunpenggrf/kunpengtaishanporting_12_0048.html */
#ifndef barrier
#if defined(__x86_64__) /* x64 */
#define barrier() __asm__ __volatile__("": : :"memory")
#define smp_mb() __asm__ __volatile__("lock; addl $0,-132(%%rsp)" ::: "memory", "cc")
#define smp_rmb() barrier()
#define smp_wmb() barrier()
#elif defined(__aarch64__) /* aarch64 */
#define barrier() __asm__ __volatile__("dmb" ::: "memory")
#define smp_mb()  __asm__ __volatile__("dmb ish" ::: "memory")
#define smp_rmb() __asm__ __volatile__("dmb ishld" ::: "memory")
#define smp_wmb() __asm__ __volatile__("dmb ishst" ::: "memory")
#else
#error No supprted CPU model
#endif
#endif


/* Cache clean */
static void cachebrush(std::size_t nbytes = 10 * 1024 * 1024)
{
    char * volatile arr = (char * volatile)malloc(nbytes * sizeof(char));
    memset(arr, 0, nbytes);
    for (std::size_t i = 0; i < nbytes; i++) {
        arr[i] = i;
    }
    // std::vector<char> arr(nbytes);
    // std::fill(arr.begin(), arr.end(), 0);
    // memset()
    // for (std::size_t i = 0; i < arr.size(); i++) {
    //     arr[i] = i;
    // }
}


/* Custom Allocator */
/* Work in progress */
/* https://howardhinnant.github.io/allocator_boilerplate.html */
template <class T>
class allocator
{
private:
    std::map<void *, std::size_t> p2b;
    std::atomic<std::size_t> msize {0};
    std::atomic<std::size_t> mpeak {0};
public:
    using value_type = T;

    allocator() noexcept {}
    template <class U> allocator(allocator<U> const&) noexcept {}

    value_type* allocate(std::size_t n) {
        std::size_t nbytes = n * sizeof(value_type);
        value_type *ptr = static_cast<value_type *>(::operator new(nbytes));
        /* If allocation success, add to size and return */
        this->msize += nbytes;
        std::size_t atom_peak = this->mpeak.load();
        while (atom_peak < this->msize.load() && !this->mpeak.compare_exchange_weak(atom_peak, this->msize.load(), std::memory_order_relaxed)) {}
        return ptr;
    }

    void deallocate(value_type* p, std::size_t n) noexcept {
        std::size_t nbytes = n * sizeof(value_type);
        ::operator delete(p);
        /* If deallocation success, sub from size and return */
        this->msize -= nbytes;
    }

    std::size_t size() { return this->msize.load(); }
    std::size_t peak() { return this->mpeak.load(); }
};
template <class T, class U>
bool
operator==(allocator<T> const&, allocator<U> const&) noexcept /* Allocator of different type T & U is not equal */
{
    return false;
}
template <class T, class U>
bool
operator!=(allocator<T> const& x, allocator<U> const& y) noexcept
{
    return !(x == y);
}


/* Memory usage */
static int proc_self_status(const char *name)
{
    FILE *statfp = fopen("/proc/self/status", "r");
    if (statfp == NULL)
        return -1;
    size_t namelen = strnlen(name, 1024);
    int val = 0;
    char *buffer = 0;
    size_t buflen = 0;
    int found = 0;
    while (!found && getline(&buffer, &buflen, statfp) != -1) {
        char *posn = buffer;
        char *epos;
        char *token;
        while ((token = strtok_r(posn, " \r\t,\n", &epos)) != 0) {
            size_t tokenlen = strnlen(token, 1024);
            if (found) {
                val = atoi(token);
                break;
            }
            size_t len = (tokenlen < namelen) ? tokenlen : namelen;
            if (strncmp(token, name, len) == 0)
                found = 1;
            posn = 0;
        }
    }
    free(buffer);
    fclose(statfp);
    if (!found)
        return -1;
    return val;
}
static inline std::string readable_size(std::size_t bytes, std::size_t precision = 2) {
    int i = 0;
    const char* units[] = {"Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    double size = bytes;
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    char buf[1024];
    sprintf(buf, "%.*f %s", precision, size, units[i]); /*%.*f*/
    return std::string(buf);
}
static inline std::size_t memusage() { return proc_self_status("VmRSS:") * 1024; }
static inline std::string memusage_str() { return readable_size(memusage() * 1024); }
static inline std::size_t mempeak() { return proc_self_status("VmHWM:") * 1024; }
static inline std::string mempeak_str() { return readable_size(mempeak() * 1024); }



#if 0
/* https://stackoverflow.com/a/59127415 */
#ifdef _MSC_VER 
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>

// The requirements for the allocator where taken from Howard Hinnant tutorial:
// https://howardhinnant.github.io/allocator_boilerplate.html

template <typename T>
struct MyAllocation
{
    size_t Size = 0;
    std::unique_ptr<T> Ptr;

    MyAllocation() { }
    MyAllocation(MyAllocation && other) noexcept
        : Ptr(std::move(other.Ptr)), Size(other.Size)
    {
        other.Size = 0;
    }
};

// This allocator keep ownership of the last allocate(n)
template <typename T>
class MyAllocator
{
public:
    using value_type = T;

private:
    // This is the actual allocator class that will be shared
    struct Allocator
    {
        [[nodiscard]] T* allocate(std::size_t n)
        {
            T *ret = new T[n];
            if (!(Current.Ptr == nullptr || CurrentDeallocated))
            {
                // Actually release the ownership of the Current unique pointer
                Current.Ptr.release();
            }

            Current.Ptr.reset(ret);
            Current.Size = n;
            CurrentDeallocated = false;
            return ret;
        }

        void deallocate(T* p, std::size_t n)
        {
            (void)n;
            if (Current.Ptr.get() == p)
            {
                CurrentDeallocated = true;
                return;
            }

            delete[] p;
        }

        MyAllocation<T> Current;
        bool CurrentDeallocated = false;
    };
public:
    MyAllocator()
        : m_allocator(std::make_shared<Allocator>())
    {
        std::cout << "MyAllocator()" << std::endl;
    }

    template<class U>
    MyAllocator(const MyAllocator<U> &rhs) noexcept
    {
        std::cout << "MyAllocator(const MyAllocator<U> &rhs)" << std::endl;
        // Just assume it's a allocator of the same type. This is needed in
        // MSVC STL library because of debug proxy allocators
        // https://github.com/microsoft/STL/blob/master/stl/inc/vector
        m_allocator = reinterpret_cast<const MyAllocator<T> &>(rhs).m_allocator;
    }

    MyAllocator(const MyAllocator &rhs) noexcept
        : m_allocator(rhs.m_allocator)
    {
        std::cout << "MyAllocator(const MyAllocator &rhs)" << std::endl;
    }

public:
    T* allocate(std::size_t n)
    {
        std::cout << "allocate(" << n << ")" << std::endl;
        return m_allocator->allocate(n);
    }

    void deallocate(T* p, std::size_t n)
    {
        std::cout << "deallocate(\"" << p << "\", " << n << ")" << std::endl;
        return m_allocator->deallocate(p, n);
    }

    MyAllocation<T> release()
    {
        if (!m_allocator->CurrentDeallocated)
            throw std::runtime_error("Can't release the ownership if the current pointer has not been deallocated by the container");

        return std::move(m_allocator->Current);
    }

public:
    // This is the instance of the allocator that will be shared
    std::shared_ptr<Allocator> m_allocator;
};

// We assume allocators of different types are never compatible
template <class T, class U>
bool operator==(const MyAllocator<T>&, const MyAllocator<U>&) { return false; }

// We assume allocators of different types are never compatible
template <class T, class U>
bool operator!=(const MyAllocator<T>&, const MyAllocator<U>&) { return true; }

int main()
{
    MyAllocator<char> allocator;
    {
        std::vector<char, MyAllocator<char>> test(allocator);
        test.resize(5);
        test.resize(std::strlen("Hello World") + 1);
        std::strcpy(test.data(), "Hello World");
        std::cout << "Current buffer: " << test.data() << std::endl;
        test.pop_back();
        test.push_back('!');
        test.push_back('\0');

        try
        {
            (void)allocator.release();
        }
        catch (...)
        {
            std::cout << "Expected throw on release() while the container has still ownership" << std::endl;
        }
    }

    auto allocation = allocator.release();
    std::cout << "Final buffer: " << allocation.Ptr.get() << std::endl;
    return 0;
}
#endif
