#pragma once

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

