# libmys 模块参考

`include/mys` 各模块的用途与招牌 API。这里只列代表性接口帮助定位；**完整、最新签名请直接读 `include/mys/<模块>.h`**。
所有公共函数用 `MYS_PUBLIC` 标记、`mys_` 前缀；宏多为全大写。

## 诊断与日志

### log.h — 分级日志
- 宏：`{T,D,I,W,E,F,R}LOG(rank, fmt, ...)`，级别 Trace/Debug/Info/Warn/Error/Fatal/Raw。`rank` 为打印者；`<0` 表示所有 rank。
- 变体（每级别都有）：`_SELF(fmt,...)` 本 rank、`_ONCE` 同点只一次、`_ORDERED` 按 rank 顺序（集合）、`_WHEN(cond,fmt,...)`。
- 默认 logger `MYS_LOGGER_G`。自定义：`mys_log_create/destroy`、`mys_log_set_level`、`mys_log_add_handler`、`mys_log_set_comm`。
- 内置 handler：`mys_log_stdio_handler1`（`[I::000 file:12] msg`）/`handler2`。

### ranklog.h — 每 rank 文件日志
- `RANKLOG_OPEN(folder)` / `RANKLOG(folder, fmt, ...)` / `RANKLOG_CLOSE(folder)`，均为集合调用，写出 `folder/000000.log` 等。

### assert.h — 断言
- `ASSERT(exp, fmt, ...)`、`FAILED(fmt, ...)`、`THROW_NOT_IMPL()`；失败时 FATAL 日志 + `mys_MPI_Abort`。
- `AS_TRUE/AS_FALSE`；类型化二元 `AS_{EQ,NE,LT,LE,GT,GE}_{I8..I64,U8..U64,F32,F64}(a,b)`，失败时打印两侧实际值。
- 区间 `AS_BETWEEN_{IE,II,EI,EE}_<type>(low, exp, high)`（I=闭/E=开）。
- `STATIC_ASSERT(expr, msg)` 编译期。可选 `-DMYS_ENABLE_ASSERT_BACKTRACE` 让断言失败附带调用栈。

### debug.h — 崩溃/超时调试
- `mys_debug_init()` / `mys_debug_fini()` 安装/卸载 SIGSEGV、SIGABRT 等信号回溯。
- `mys_debug_set_message(fmt,...)`、`mys_debug_set_max_frames(n)`、`mys_debug_add_stack_filter(str)`。
- 超时看门狗：`mys_debug_set_timeout(sec)` / `_env` / `_restart` / `mys_debug_clear_timeout()`（需 `-DMYS_ENABLE_DEBUG_TIMEOUT` + `-lrt`）。
- 整模块可用 `-DMYS_DISABLE_DEBUG` 关闭。

### backtrace.h — 调用栈
- `mys_backtrace(void **addrs, int max_depth)`、`mys_backtrace_source(addr, buf, n)`、`mys_backtrace_symbol(addr, buf, n)`。

## 计时与性能

### hrtime.h — 高精度计时
- `mys_hrtime()`→秒(double)、`mys_hrtick()`→tick、`mys_hrfreq()`→tick/秒、`mys_hrname()`、`mys_hrreset()`。
- `mys_hrsync(comm)` 跨 MPI 进程对齐计时器。平台后端：aarch64 / posix / mpi（`mys_hr*_posix` 等）。

### checkpoint.h — 计时打点
- `mys_checkpoint_init()` / `mys_checkpoint(name_fmt, ...)` / `mys_checkpoint_dump(file_fmt, ...)` / `mys_checkpoint_reset()`。

### trace.h — 迭代追踪
- `mys_trace_create/destroy`、`mys_trace_start_iter/next_iter/interrupt_iter`、`mys_trace_1{p,u,i,d}(trace, data)`。

### statistic.h — 统计
- `mys_arthimetic_mean/harmonic_mean/geometric_mean/standard_deviation(double*, n)`。
- `mys_aggregate_analysis(value)` / `_array(n, values, results)` 跨 rank 聚合。
- boxplot：`mys_boxplot(values, n)`（文本图）、`mys_boxplot_create/destroy/serialize[_pretty]`。

## 并行 / MPI

### mpistubs.h — MPI 包装/桩
- 提供 `mys_MPI_*` 包装（`mys_MPI_Comm`、`mys_MPI_COMM_WORLD`、`mys_MPI_Comm_rank`、`mys_MPI_Abort` 等），使代码在 `MYS_NO_MPI` 下也能编译。

### commgroup.h — 通信分组
- `mys_commgroup_create(comm, color, key)`、`mys_commgroup_create_node(comm)`、`mys_commgroup_create_numa(comm)`、`mys_commgroup_dup/release`。
- 查询：`mys_query_group_id/local_rank/brother/neighbor/group_size`。多为集合调用。

### thread.h — 线程
- `mys_thread_id()`；互斥锁 `mys_mutex_t` + `mys_mutex_init/destroy/lock/trylock/unlock`。

### atomic.h — 原子操作
- 原子读写/CAS/加减等（细节见头文件，按类型提供）。

### table.h — 跨 rank 汇总表
- `mys_table_create(comm, num_attrs, ...)`、`mys_table_append_cell(...)`、`mys_table_set_schema`、`mys_table_add_comment`、`mys_table_dump(file)`。

## 数据与算法

### rand.h — 随机数
- 种子：`mys_rand_seed(a)` / `_seed2(a0,a1)` / `_seed_time()` / `_seed_hardware()`；底层 `mys_rand_xoroshiro128ss()`。
- 闭区间 `[mi,ma]`：`mys_rand_{u8..u64,i8..i64,sizet,ssizet,f32,f64}(mi, ma)`；`mys_rand_str(buf, n, choices)`。

### algorithm.h — 排序
- `mys_sort_{int,sizet,i32,i64,u32,u64,f32,f64}(arr, n)`；逆序加 `_r`。
- 带索引：`mys_sort_f64_to_f64i`、`mys_sort_sizet_to_sizeti(values, n, sortctl)`。

### hash.h — SHA256
- 一次性：`mys_sha256_{bin,hex,base64}(text, size, output)`。
- 流式：`mys_sha256_init/update/...`（`update_{i8..i64,u8..u64,f32,f64,ptr,arr}`）、`mys_sha256_dump_{hex,base64,bin}`。

### base64.h — Base64
- `mys_base64_encode_len(n)` / `mys_base64_decode_len(str)` 及编解码函数。

### string.h — 动态字符串与解析
- `mys_string_t`：`mys_string_create[2]`、`mys_string_destroy`、`mys_string_fmt[_v]`、`mys_string_append[_n/2]`、`mys_string_dup`。
- 大小：`mys_parse_readable_size(text)`、`mys_to_readable_size(bytes, prec, buf, n)`。
- 解析（带默认值）：`mys_str_to_{int,long,sizet,double,float,u32,u64,i32,i64,f32,f64}(str, default)`。

### format.h — 可扩展格式化
- `mys_fmter_create/destroy`、`mys_fmter_register_pass(fmter, name, fn)`、`mys_fmter_compile(fmter, str)`→`mys_fmtex_apply(fmtex, ctx)`。

### memory.h — 分配与内存工具
- arena：`mys_arena_create/destroy`、`mys_arena_set_debug`、`mys_arena_print_leaked`、`mys_arena_next_leaked`。
- 分配：`mys_{malloc,calloc,aligned_alloc,realloc,free}2(arena, ...)`（带 arena 记账）；`mys_alloc_record/free_record`。
- 其它：`mys_cache_flush(nbytes)`、共享内存 `mys_alloc_shared_memory/free_shared_memory`、位视图 `mys_bits(data, size)`。

### pool.h — 对象池
- `mys_pool_create(obj_size)` / `_create2(obj_size, cap, strategy)`、`mys_pool_acquire/release`、`mys_pool_destroy`。

## 系统 / 平台

### os.h — 操作系统接口
- 子进程：`mys_prun_create[2]/destroy`（带输出缓冲）、`mys_popen_create/test/wait/kill`。
- 路径：`mys_path_is_{exists,file,symlink,dir}`、`mys_mkdir`、`mys_ensure_dir/ensure_parent`、`mys_readfd`。
- 亲和性/NUMA：`mys_get/print/stick_affinity`、`mys_current_cpu/numa`、`mys_cpu_num/numa_num/numa_size`、`mys_numa_query/move`。
- 环境变量（带默认值）：`mys_env_{str,int,long,sizet,u32,u64,i32,i64,f32,f64}(name, default)`。
- 杂项：`mys_sleep/busysleep(sec)`、`mys_procname()`、`mys_hostname()`、`mys_wait_flag(...)`。

### net.h — 网络
- `mys_tcp_server[2](addr, port[, opt])`、`mys_tcp_client(addr, port)`、`mys_udp_server/client(addr, port)`。

### pmparser.h — 进程内存映射
- `mys_pmparser_self()` / `mys_pmparser_parse(pid)`、遍历 `mys_pmparser_next`，`mys_pmparser_print`，`mys_pmparser_free`。

### guard.h — 变量守卫
- `mys_guard_begin(type_name, type_size, ptr, file, line)` / `mys_guard_end(...)` 检测变量被意外改写/越界。

## 数学

### math.h — 可移植 libm 替代
- `mys_math_{copysign,fabs,log,log10,sqrt,scalbn,pow,trunc}`（不依赖系统 libm，行为可控）。

### linalg.h / linalg.hpp — 划分与线性代数
- `mys_partition_naive(gbegin, gend, npart, ipart, *lbegin, *lend)` 等分区间。
- `mys_partition_rcb(..., weights, tol, num_tests, *lbegin, *lend)` 带权递归坐标二分。
- `linalg.hpp` 提供 C++ 线性代数（仅 C++）。

### complex.h — 复数
- 复数类型与基本运算（在 `mys.h` 末尾引入）。

## 通用宏与类型

### macro.h
- 索引：`RMIDX/CMIDX(row,col,nrow,ncol)`（行/列主序）、`IDX2(x,y,nx,ny)`、`IDX3(x,y,z,nx,ny,nz)`。
- 展开：`REP2..REP1024`、`REP5..REP1000`（重复语句，做微基准）。
- 对齐：`MYS_ALIGN_UP/DOWN(n, align)`、`MYS_ROUND_UP/DOWN(n, width)`；`MYS_MACRO2STR(x)`；`GRACEFUL_EXIT()`。

### errno.h
- `MYS_GOTOIF(errcond, errcode, label)`、`MYS_RETIF(errcond, errcode, ...)` 早退/跳转清理。

### color.h
- ANSI 终端颜色常量/宏，用于日志和输出着色。

### type.h
- 公共类型别名/定义。

### misc.h
- 杂项，如 `mys_mpi_ensure_init()`（按需初始化 MPI）。

## GPU

### cuda.cuh
- CUDA 辅助，仅当定义 `CUDA_ARCH` 时由 `mys.h` 引入。

## 第三方（mys3，随核心一起声明）
- `mys3/cJSON/cJSON.h`（JSON）、`mys3/matrixmarket/mmio.h`（Matrix Market I/O）、`mys3/stb/stb_image.h`（图像）。
- 实现体需分别 `-DMYS_ENABLE_CJSON` / `-DMYS_ENABLE_MATRIXMARKET` / `-DMYS_ENABLE_STB` 才会展开；各自保留原始 license。
