/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once
/*
 [Atomic Operations and Memory Orderings]
 1. Atomic operations are used to ensure that shared data or resources are treated as
    a single, indivisible, and uninterrupted unit of memory operation.
 2. Memory orderings are used to control how memory operations are observed and ordered.
 3. These concepts are commonly associated with multi-threaded environments where threads
    act as producers and consumers of specific resources. Atomic flags are used to signal
    the status of these resources. When resources are modified after flag changes take
    effect, a write-after-read (WAR) hazard may occurs that other threads may read memory
    before this thread write. To mitigate such issue, memory fences with specific ordering
    constraints are utilized, preventing the compiler and CPU from reordering resources and
    flag updates. This's why all atomic operations accept a memory order argument because
    we always want to ensure modification on resources actually happened before accessing
    atomic flags. To use atomic operations alone, simply use "relaxed" ordering to provide
    the compiler and CPU the most flexibility possible when optimizing your code.
 4. Atomic can used with any integral scalar or pointer type that is 1, 2, 4, or 8 bytes.
    16-byte integral types are also allowed if ‘__int128’ is supported by the architecture.
 5. Mutex can be implemented with atomic flag. To atomic update other type, use mutex instead.
 6. Concepts
    - atomic store / atomic load / atomic inc,dec intrinsics
    - memory barrier, fence intrinsics
    - acquire(ACQ), release(REL), sequentially consistent(SEQ_CST) primitives
    - weak memory order ISA (ARM, RISC-V default), total store order ISA (x86, RISC-V Ztso)
    - read after write(RAW), write after read(WAR) hazard
 7. References
    - Illustrate ACQ, REL and SEQ_CST usages
      https://gcc.gnu.org/wiki/Atomic/GCCMM/AtomicSync
    - Understanding atomics and memory ordering
      https://dev.to/kprotty/understanding-atomics-and-memory-ordering-2mom
    - List of memory ordering on modern SMP microprocessor systems
      https://en.wikipedia.org/wiki/Memory_ordering#Runtime_memory_ordering
    - Making producer & consumer semantic
      https://stackoverflow.com/a/13633355/11702338
      ---------- (Producer)
      (1) atomic write flag with release primitive
          $ memcpy(shmem, src, size);
          $ mys_atomic_store_n(&flag, true, MYS_ATOMIC_RELEASE);
      (2) use fence with release primitive then write flag with relaxed primitive
          $ memcpy(shmem, src, size);
          $ mys_atomic_fence(MYS_ATOMIC_RELEASE);
          $ flag = true; // or mys_atomic_store_n(&flag, true, MYS_ATOMIC_RELAXED);
      ---------- (Consumer)
      (1) read flag with relaxed primitive then use fence with acquire primitive
          $ while (!flag); // or while (!mys_atomic_load_n(&flag, MYS_ATOMIC_RELAXED));
          $ mys_atomic_fence(MYS_ATOMIC_ACQUIRE);
          $ memcpy(dest, shmem, size);
      ----------
*/

/**************************************************/
/* memory orders (ascending order of strength)    */
/* all memory ordering is that it guarantees what */
/* "HAS HAPPENED", not what is going to happen.   */
/**************************************************/

/**
 * @brief Don't prevent any reorder optimization across this instruction
 * @note Most aggresive reorder optimizations are allowed
 */
#define MYS_ATOMIC_RELAXED __ATOMIC_RELAXED
/**
 * @brief Ensure all memory operations declared after this
 * operation actually happen after it (commonly used with `mys_atomic_load()`)
 * @note Prevent hoisting of code to before the operation,
 * so guarantee operations happened before this operation on this thread
 * can be observed by operations after it on other threads
 */
#define MYS_ATOMIC_ACQUIRE __ATOMIC_ACQUIRE
/**
 * @brief Ensure all memory operations declared before this
 * operation actually happen before it (commonly used with `mys_atomic_store()`)
 * @note Prevent sinking of code to after the operation,
 * so guarantee operations after this operation on other threads
 * can observe operations happened before it on this thread
 */
#define MYS_ATOMIC_RELEASE __ATOMIC_RELEASE
/**
 * @brief Ensure all memory operations declared before/after
 * this operation actually happen before/after it.
 * @note Combines the effects of both `MYS_ATOMIC_ACQUIRE` and `MYS_ATOMIC_RELEASE`
 */
#define MYS_ATOMIC_ACQ_REL __ATOMIC_ACQ_REL
/**
 * @brief
 * Ensure all memory operations declared before/after
 * this operation actually happen before/after it (same like `MYS_ATOMIC_ACQ_REL`),
 * and ensure all cores can observe identical sequence of operations that use `MYS_ATOMIC_SEQ_CST`
 * @note
 * Enforce sequential consistency (TOTAL ORDERING) with all other `MYS_ATOMIC_SEQ_CST` operations.
 * The most expensive order.
 */
#define MYS_ATOMIC_SEQ_CST __ATOMIC_SEQ_CST
/* This semantic is least used and commonly implemented by `MYS_ATOMIC_ACQUIRE` */
// #define MYS_ATOMIC_CONSUME __ATOMIC_CONSUME


/**************************************************/
/* fence                                          */
/**************************************************/

/*
  Synchronize between threads based on the specified memory order. (All memory orders are valid)
 */
#define mys_atomic_fence(memorder) __atomic_thread_fence (memorder)
#define mys_atomic_signal_fence(memorder) __atomic_signal_fence (memorder)

#define mys_atomic_wait_eq(ptr, val, memorder) do {              \
    while (mys_atomic_load_n(ptr, MYS_ATOMIC_RELAXED) != val) {} \
    mys_atomic_fence(memorder);                                  \
} while (0)

/**************************************************/
/* atomic load, store, exchange, compare_exchange */
/**************************************************/

/*
  Atomic load content from `*ptr` to `*outptr`
  Valid memory orders:
  - `MYS_ATOMIC_RELAXED`
  - `MYS_ATOMIC_ACQUIRE` (commonly used)
  - `MYS_ATOMIC_SEQ_CST`
 */
#define mys_atomic_load(ptr, outptr, memorder) __atomic_load   (ptr, outptr, memorder)
#define mys_atomic_load_n(ptr, memorder)         __atomic_load_n (ptr, memorder)
/*
  Atomic store content from `*inptr` to `*ptr`
  Valid memory orders:
  - `MYS_ATOMIC_RELAXED`
  - `MYS_ATOMIC_RELEASE` (commonly used)
  - `MYS_ATOMIC_SEQ_CST`
 */
#define mys_atomic_store(ptr, inptr, memorder) __atomic_store   (ptr, inptr, memorder)
#define mys_atomic_store_n(ptr, val, memorder)   __atomic_store_n (ptr, val, memorder)
/*
  Atomic store content val into *ptr, and copy the previous content of `*ptr` to `*outptr`. (All memory orders are valid)
 */
#define mys_atomic_exchange(ptr, val, outptr, memorder) __atomic_exchange (ptr, val, outptr, memorder)
#define mys_atomic_exchange_n(ptr, val, memorder)         __atomic_exchange_n (ptr, val, memorder)
/*
  #### if (*ptr == *ptr_old)
  `{ *ptr = *ptr_new; return true; }`
  - memory is affected according to success_memorder.
  - when in doubt, use `MYS_ATOMIC_ACQ_REL`.
  #### else
  `{ *ptr_old = *ptr; return false; }`
  - memory is affected according to failure_memorder.
  - failure_memorder must be relax, acquire, or seq_cst, cannot stronger than success_memorder.
  - when in doubt, use `MYS_ATOMIC_RELAXED`.
*/
#define mys_atomic_compare_exchange(ptr, ptr_old, ptr_new, success_memorder, failure_memorder) \
     __atomic_compare_exchange (ptr, ptr_old, ptr_new, 0, success_memorder, failure_memorder)
#define mys_atomic_compare_exchange_n(ptr, ptr_old, val_new, success_memorder, failure_memorder) \
    __atomic_compare_exchange_n (ptr, ptr_old, val_new, 0, success_memorder, failure_memorder)


/**************************************************/
/* atomic arithmetic operations (fetch old value) */
/**************************************************/

#define mys_atomic_fetch_add(ptr, val, memorder) __atomic_fetch_add (ptr, val, memorder) // `t=(*ptr); (*ptr)+=val; return t;` All memory orders are valid
#define mys_atomic_fetch_sub(ptr, val, memorder) __atomic_fetch_sub (ptr, val, memorder) // `t=(*ptr); (*ptr)-=val; return t;` All memory orders are valid
#define mys_atomic_fetch_and(ptr, val, memorder) __atomic_fetch_and (ptr, val, memorder) // `t=(*ptr); (*ptr)&=val; return t;` All memory orders are valid
#define mys_atomic_fetch_xor(ptr, val, memorder) __atomic_fetch_xor (ptr, val, memorder) // `t=(*ptr); (*ptr)^=val; return t;` All memory orders are valid
#define mys_atomic_fetch_or(ptr, val, memorder) __atomic_fetch_or  (ptr, val, memorder) // `t=(*ptr); (*ptr)|=val; return t;` All memory orders are valid
#define mys_atomic_fetch_nand(ptr, val, memorder) __atomic_fetch_nand(ptr, val, memorder) // `t=(*ptr); (*ptr)=~(*ptr&val); return t;` All memory orders are valid

/**************************************************/
/* atomic arithmetic operations (fetch new value) */
/**************************************************/

#define mys_atomic_add_fetch(ptr, val, memorder) __atomic_add_fetch (ptr, val, memorder) // `(*ptr)+=val; return *ptr;` All memory orders are valid
#define mys_atomic_sub_fetch(ptr, val, memorder) __atomic_sub_fetch (ptr, val, memorder) // `(*ptr)-=val; return *ptr;` All memory orders are valid
#define mys_atomic_and_fetch(ptr, val, memorder) __atomic_and_fetch (ptr, val, memorder) // `(*ptr)&=val; return *ptr;` All memory orders are valid
#define mys_atomic_xor_fetch(ptr, val, memorder) __atomic_xor_fetch (ptr, val, memorder) // `(*ptr)^=val; return *ptr;` All memory orders are valid
#define mys_atomic_or_fetch(ptr, val, memorder) __atomic_or_fetch  (ptr, val, memorder) // `(*ptr)|=val; return *ptr;` All memory orders are valid
#define mys_atomic_nand_fetch(ptr, val, memorder) __atomic_nand_fetch(ptr, val, memorder) // `(*ptr)=~(*ptr&val); return *ptr;` All memory orders are valid



/* https://groups.google.com/g/lock-free/c/Nescdq-8qVM/m/0LfqxmG27sUJ
----------- ACQUIRE & RELEASE
double data = 0;
bool flag = false;
// thread 1
data = produce_data();
mys_atomic_store_n(&flag, true, MYS_ATOMIC_RELEASE);
// thread 2
if (mys_atomic_load_n(&flag, MYS_ATOMIC_ACQUIRE))
    consume_data(data);

----------- ACQ_REL
double *data = malloc(size);
int ref = 1;
// ... run code
memset(data, 'A', size);
if (1 == mys_atomic_fetch_sub(&ref, 1, MYS_ATOMIC_ACQ_REL))
    free(data);

----------- SEQ_CST
int x = 0; // atomic no.1
int y = 0; // atomic no.2
int rx, ry;
// thread 1
mys_atomic_store_n(&x, 1, MYS_ATOMIC_ACQ_REL);
ry = mys_atomic_load_n(&y, MYS_ATOMIC_ACQ_REL);
// thread 2
mys_atomic_store_n(&y, 1, MYS_ATOMIC_ACQ_REL);
rx = mys_atomic_load_n(&x, MYS_ATOMIC_ACQ_REL);
// all threads can observe (rx == 0 && ry == 0)
// but with MYS_ATOMIC_SEQ_CST this will be
// impossible because the sequence of SEQ_CST
// operations is enforced to be identical among threads.
*/



/* gcc -g a.c && objdump -S -d ./a.out | less
----------- Code
void relaxed() { __asm__ volatile("nop"); __atomic_thread_fence(__ATOMIC_RELAXED); __asm__ volatile("nop"); }
void acquire() { __asm__ volatile("nop"); __atomic_thread_fence(__ATOMIC_ACQUIRE); __asm__ volatile("nop"); }
void release() { __asm__ volatile("nop"); __atomic_thread_fence(__ATOMIC_RELEASE); __asm__ volatile("nop"); }
void acq_rel() { __asm__ volatile("nop"); __atomic_thread_fence(__ATOMIC_ACQ_REL); __asm__ volatile("nop"); }
void seq_cst() { __asm__ volatile("nop"); __atomic_thread_fence(__ATOMIC_SEQ_CST); __asm__ volatile("nop"); }
int main() {
    relaxed();
    acquire();
    release();
    acq_rel();
    seq_cst();
}
----------- Intel-x64 Results (total store order ISA)
- relaxed: nop
- acquire: nop
- release: nop
- acq_rel: nop
- seq_cst: mfence
----------- ARMv8-AArch64 Results (weak memory order ISA)
- relaxed: nop
- acquire: dmb ishld
- release: dmb ish
- acq_rel: dmb ish
- seq_cst: dmb ish
----------- ARM Cacheable and shareable memory attributes
https://developer.arm.com/documentation/dui0489/c/arm-and-thumb-instructions/miscellaneous-instructions/dmb--dsb--and-isb
https://developer.arm.com/documentation/den0024/a/Memory-Ordering/Memory-attributes/Cacheable-and-shareable-memory-attributes
    +----------------------------------------------------------------------------------+
    | Hyprevisor (Outer Shareable Domain)                                              |
    |                                                                                  |
    |      +--------------------------------------------------------------------+      |
    |      | Operating System 1 (Inner Shareable Domain)                        |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |    | Socket 0                 |    | Socket 1                 |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |  | Core 0 |  | Core 1 |  |    |  | Core 2 |  | Core 3 |  |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |                          |    |                          |    |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |                                                                    |      |
    |      +--------------------------------------------------------------------+      |
    |                                                                                  |
    |                                                                                  |
    |      +--------------------------------------------------------------------+      |
    |      | Operating System 2 (Inner Shareable Domain)                        |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |    | Socket 2                 |    | Socket 3                 |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |  | Core 4 |  | Core 5 |  |    |  | Core 6 |  | Core 7 |  |    |      |
    |      |    |  +--------+  +--------+  |    |  +--------+  +--------+  |    |      |
    |      |    |                          |    |                          |    |      |
    |      |    +--------------------------+    +--------------------------+    |      |
    |      |                                                                    |      |
    |      +--------------------------------------------------------------------+      |
    |                                                                                  |
    |                                                                                  |
    |           +--------------------------+    +--------------------------+           |
    |           | GPU Device               |    | FPGA Device              |           |
    |           |                          |    |                          |           |
    |           +--------------------------+    +--------------------------+           |
    |                                                                                  |
    +----------------------------------------------------------------------------------+
*/
