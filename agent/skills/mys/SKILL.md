---
name: mys
description: >-
  libmys (Mayeths' Library)  C/C++ 工具库的使用规范：集成方式、高频接口与模块地图
  （断言宏、高精度计时器、随机数发生器、系统接口工具函数、MPI通信库工具函数、Hash工具函数、
  调试工具函数、原子操作工具函数、简易内存分配器、内存复用对象池）。
  适用于在使用 `#include <mys.h>` 的 C/C++/MPI/HPC 项目中编写、编译、调试代码，
  或用户提到 libmys、mys.h、MYS_IMPL、ILOG/DLOG/ASSERT 等场景。
---

This is user skill "mys".

# libmys 使用 Practice

libmys（头文件 `libmys/include/mys`）是stb 类 single header 思路的 C/C++ 工具库，主要面向 POSIX（Linux/macOS）+ MPI/HPC
场景，提供日志、断言、计时、随机、内存、MPI 分组等模块。本文档面向**使用方**（在项目里 include / 编译 / 调用），不覆盖库自身的开发扩展。

本文档是地图与高频用法；**完整函数签名以头文件为准**——需要细节时直接读对应 `include/mys/<模块>.h`，不要凭记忆臆测签名。

## 集成与编译

只 include 一个总头，并在**整个程序里恰好一个** `.c`/`.cpp` 里定义 `MYS_IMPL` 来展开实现体：

```c
//// main.c —— 唯一定义实现的翻译单元
//#define MYS_NO_MPI
#define MYS_IMPL
#define MYS_ENABLE_DEBUG_TIMEOUT
#define MYS_ENABLE_NUMA
#include <mys.h>
//// other.c —— 其它文件只 include，不要再写 MYS_IMPL
#define MYS_ENABLE_DEBUG_TIMEOUT
#define MYS_ENABLE_NUMA
#include <mys.h>
```

编译时把 `include` 加入头文件搜索路径：

```bash
mpicc   -I${MYS_DIR}/include main.c other.c        # 默认按 MPI 环境编译
gcc     -I${MYS_DIR}/include -DMYS_NO_MPI main.c   # 非 MPI：必须 -DMYS_NO_MPI
```

关键编译宏（放在 `#include <mys.h>` 之前 `#define`，或用 `-D` 传入）：

| 宏 | 作用 |
|----|------|
| `MYS_IMPL` | 在本翻译单元展开 libmys 实现并导出符号（每个程序仅一处）。 |
| `MYS_IMPL_LOCAL` | 同上但隐藏可见性；动态库各自内置一份、互不干扰时使用。 |
| `MYS_NO_MPI` | 关闭 MPI，用 `gcc`/`g++` 等非 MPI 编译器时必须加。 |
| `MYS_DISABLE_DEBUG` | 关闭 `debug.h`（如 icc 等编译器报错时）。 |
| `MYS_ENABLE_DEBUG_TIMEOUT` | 启用 `mys_debug_set_timeout`（还需链接 `-lrt`）。 |
| `MYS_ENABLE_CJSON` / `MYS_ENABLE_MATRIXMARKET` / `MYS_ENABLE_STB` | 启用 `mys3` 第三方实现体。 |

要求 C++11+ 或 GNU C99+（用 `-std=gnu99`/`-std=c++11`，不要 `-ansi`）。仅支持 Linux/macOS/BSD。

## 高频 API（迷你示例）

### 断言（`assert.h`）

失败时打 FATAL 日志并 `MPI_Abort`。

```c
ASSERT(ptr != NULL, "alloc failed for %zu bytes", n);
AS_EQ_I32(got, want);                    // 类型化比较，失败时打印两侧实际值
AS_BETWEEN_IE_I32(0, idx, count);        // 断言 0 <= idx < count
FAILED("unreachable: mode=%d", mode);    // 无条件失败
STATIC_ASSERT(sizeof(int) == 4, "需要 32 位 int");
```

`AS_*` 按类型后缀分 `I8/I16/I32/I64`、`U8/U16/U32/U64`、`F32/F64`，操作 `EQ/NE/LT/LE/GT/GE` 与区间 `BETWEEN_{IE,II,EI,EE}`。

### 日志（`log.h`）

宏名 = 级别首字母 + `LOG`
`TLOG`/`DLOG`/`ILOG`/`WLOG`/`ELOG`/`FLOG`/`RLOG`
Trace/Debug/Info/Warn/Error/Fatal/Raw

```c
DLOG(0, "x=%d", 123);                   // 仅 rank 0 打印：[D::000 file.c:12] x=123
ELOG_SELF("local error");               // 本 rank 打印，无需 rank 参数
ILOG_WHEN(myrank == nranks / 2, "mid"); // 条件为真才打印
TLOG_ONCE("init done");                 // 同一 __FILE__:__LINE__ 只打印一次
WLOG_ORDERED("done");                   // 所有 rank 消息汇总到 rank 0 依次打印（集合调用）
RLOG(0, "plain line");                  // 原样输出，不带 LOG 前缀（结尾换行仍自动补）
```

支持使用`mys_log_create()`和`mys_log_destroy()`建立自定logger。
使用`mys_log_add_handler()`添加handler。
已提供通用handler`mys_log_stdio_handler1()`和`mys_log_stdio_handler2()`

### 每-rank 文件日志（`ranklog.h`）

各 rank 写到独立文件，集合调用。

```c
RANKLOG_OPEN("LOG.run");                 // 创建 LOG.run/000000.log ...
RANKLOG("LOG.run", "solve %f s", t);
RANKLOG_CLOSE("LOG.run");
```

### 高精度计时（`hrtime.h`）

```c
double t0 = mys_hrtime();                // 秒（double）
/* ... */
double dt = mys_hrtime() - t0;
mys_hrsync(MPI_COMM_WORLD);              // 跨进程对齐计时器（可选）
```

### 崩溃调试（`debug.h`）

安装信号处理器，崩溃时打印回溯；可选超时看门狗。

```c
mys_debug_init();
int timeout = mys_env_i32("DEBUG_TIMEOUT", 0);
if (timeout != 0) {
    mys_debug_set_timeout(timeout); // 需 -DMYS_ENABLE_DEBUG_TIMEOUT 且 -lrt
    ILOG(0, "Set debug timeout to %d seconds", timeout);
}
/* ... */
mys_debug_fini();
```

### 随机数（`rand.h`）

xoroshiro128\*\*，类型化闭区间 `[mi, ma]`。

```c
mys_rand_seed_time();
int    r = mys_rand_i32(0, 99);
double f = mys_rand_f64(0.0, 1.0);
(void)mys_rand_f64_array(arr, nelements, 0.0, 1.0);
(void)mys_rand_str(buffer/*size 8*/, 7, "abcdefghijklmnopqrstuvwxyz");
```

### 系统/环境（`os.h`）

环境变量、路径、亲和性、NUMA。

```c
int    n = mys_env_int("OMP_NUM_THREADS", 1);
double t = mys_env_f64("TOL", 1e-6);
mys_ensure_dir("out/sub", 0755);
const char *host = mys_hostname();
```

### 动态字符串（`string.h`）

自动扩容的字符串构造器，C 字符串在 `str->text`（长度 `str->size`）。

```c
mys_string_t *s = mys_string_create();   // 或 mys_string_create2("init str %d", 0)
mys_string_fmt(s, "iter %d, ", it);      // 追加格式化文本，按需自动扩容
mys_string_append(s, "done");            // 追加普通字符串
ILOG(0, "%s", s->text);                  // 访问底层 C 字符串
mys_string_destroy(&s);                  // 配对释放，置空指针
```

## 模块索引

细节见 [reference.md](reference.md)，或直接读 `include/mys/<模块>.h`。

| 头文件 | 用途 |
|--------|------|
| `log.h` | 分级日志宏 `[TDIWEFR]LOG` + `_SELF/_ONCE/_ORDERED/_WHEN`；多 logger 与 handler。 |
| `ranklog.h` | 每个 rank 写独立文件：`RANKLOG_OPEN/RANKLOG/RANKLOG_CLOSE`。 |
| `assert.h` | 运行时断言 `ASSERT/FAILED/THROW_NOT_IMPL`、类型化 `AS_*`、`STATIC_ASSERT`。 |
| `debug.h` | 信号回溯、崩溃信息、超时看门狗 `mys_debug_*`。 |
| `backtrace.h` | 取调用栈：`mys_backtrace` / `_source` / `_symbol`。 |
| `hrtime.h` | 高精度计时 `mys_hrtime/hrtick/hrfreq`、跨进程同步 `mys_hrsync`。 |
| `statistic.h` | 均值/标准差、聚合分析、boxplot 文本图。 |
| `checkpoint.h` | 轻量计时打点 `mys_checkpoint` + `mys_checkpoint_dump`。 |
| `trace.h` | 迭代/事件追踪 `mys_trace_*`。 |
| `rand.h` | 随机数 `mys_rand_*`（类型化区间）、多种 seed。 |
| `algorithm.h` | 多类型排序 `mys_sort_*`（含逆序 `_r`、带索引返回）。 |
| `hash.h` | SHA256：一次性 `mys_sha256_{bin,hex,base64}` 与流式 ctx。 |
| `base64.h` | base64 编解码与长度计算。 |
| `string.h` | 动态字符串 `mys_string_t`、可读字节数、`mys_str_to_*` 解析。 |
| `format.h` | 可扩展格式化器（自定义 pass）`mys_fmter_*`。 |
| `memory.h` | arena 分配器 + 泄漏追踪、共享内存、cache flush、位视图。 |
| `pool.h` | 定长对象池 `mys_pool_*`。 |
| `os.h` | 进程(`prun/popen`)、路径/目录、亲和性、NUMA、`mys_env_*`、sleep、主机名。 |
| `net.h` | TCP/UDP socket 辅助 `mys_tcp_*` / `mys_udp_*`。 |
| `pmparser.h` | 解析 `/proc/<pid>/maps`。 |
| `mpistubs.h` | MPI 包装/桩，使 `MYS_NO_MPI` 下也能编译。 |
| `commgroup.h` | MPI 通信分组（node/numa 分组与查询）`mys_commgroup_*`。 |
| `table.h` | 跨 rank 汇总并输出表格 `mys_table_*`。 |
| `thread.h` | 线程 id、互斥锁 `mys_mutex_*`。 |
| `atomic.h` | 原子操作。 |
| `guard.h` | 变量改写/越界守卫 `mys_guard_begin/end`。 |
| `errno.h` | 早退/跳转宏 `MYS_GOTOIF` / `MYS_RETIF`。 |
| `math.h` | 自带 libm 替代 `mys_math_*`（可控精度/可移植）。 |
| `linalg.h` / `linalg.hpp` | 区间划分 `mys_partition_naive/rcb`；C++ 线性代数。 |
| `complex.h` | 复数类型与运算。 |
| `macro.h` | 通用宏：索引 `RMIDX/CMIDX/IDX2/IDX3`、`REPn`、`MYS_ALIGN_*`/`MYS_ROUND_*`。 |
| `color.h` | ANSI 终端颜色。 |
| `type.h` | 公共类型定义。 |
| `misc.h` | 杂项，如 `mys_mpi_ensure_init`。 |
| `cuda.cuh` | CUDA 辅助（仅在定义 `CUDA_ARCH` 时引入）。 |

第三方子库 `mys3`（`cJSON` / `matrixmarket` mmio / `stb_image`）随 `mys.h` 一起声明，实现体需对应 `MYS_ENABLE_*` 才展开。

## 约定与陷阱

- `MYS_IMPL` 全程序只能有一处；多个文件都写会重复定义符号。其余文件只 `#include <mys.h>`。
- 非 MPI 编译必须 `-DMYS_NO_MPI`，否则会找 `mpi.h`。
- 公共 API 多为 `mys_` 前缀的函数 + 全大写宏（`ILOG`/`ASSERT`/`RANKLOG` 等）。
- `OPEN/CLOSE`、`_ORDERED`、`mys_hrsync`、`commgroup_create` 等是**集合调用**，所有 rank 都要执行。
- 形如 `mys_xxx_create` 的对象多有配套 `mys_xxx_destroy`，注意配对释放。
- 不确定签名就读头文件；很多函数有类型后缀变体（`_i32/_u64/_f64` 等），按数据类型选。
