This is user skill reference "clusters/qh". Last modified: 2026-10-09.

# QH 集群 Practice

本文档只描述 QH 集群的平台操作方式，不包含具体项目规则。QH 是 Rocky Linux 8.9（x86_64）+ Slurm 的异构集群，含 Intel Xeon Max（HBM）与 AMD EPYC 两类节点。

## 登录与连接

QH 集群默认通过本地 tmux 会话进入 `ssh qh`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh qh 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIQH` 可以保留工作路径、编译器环境、MPI 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIQH` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIQH 2>/dev/null || tmux new-session -d -s AIQH -n cmd1
tmux list-windows -t AIQH | grep -q 'cmd1' || tmux new-window -t AIQH -n cmd1
tmux capture-pane -t AIQH:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20
```

若集群长时间无响应，通常说明网络或登录连接已中断，需要重新 `ssh qh`。

## 命令与脚本

为了支持并行任务，`AIQH` 中的 tmux window 按 `cmd1`、`cmd2` 等命名。通常情况下，普通命令在默认的 `cmd1` 中运行。

如果还未进入 qh 集群，在对应 window 中发送：

```bash
tmux send-keys -t AIQH:cmd1 'ssh qh' Enter
```

不要在已经进入 qh 时重复执行 `ssh qh`，以免产生嵌套 SSH。

运行命令时，应在命令后加上完成标记，方便判断命令是否结束以及退出码：

```bash
tmux send-keys -t AIQH:cmd1 '<command>; echo __AIQH_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AIQH:cmd1 -p -J -S -200 | grep -v '^$' | tail -n 80
```

如果没有看到 `__AIQH_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。
如果使用类似 `tmux capture-pane -t AIQH:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20` 的 capture 功能发现命令未结束，则后面不应给用户 `sleep 20; tmux capture-pane -t AIQH:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20` 这样的命令。
应去掉其中的 `sleep 20`，由用户确认运行结束后手动点击确认，直接执行 `tmux capture-pane -t AIQH:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20`。
如果需要 sleep，则一般 sleep 5 或 10 秒；如果 10 秒后还未结束，则再 sleep 10 秒。如果能确认会等得久一些，可以停止自我 sleep 等待，让用户来确认结束后唤醒。

## 工作环境

QH 使用共享账号 `xuewei`（组 `xuewei_group`）。始终使用登录默认的 `bash`。常用目录：

```text
个人目录: /online1/xuewei_group/xuewei/huanghp
并行盘: /online1
账号家目录: /home/xuewei_group/xuewei
软件栈: /apps
```

工作和算例一律放在个人目录 `/online1/xuewei_group/xuewei/huanghp`（`~/data/huanghp`），与共享账号其他用户隔离。

进入 qh 后，加载环境统一 source 个人 `set_env`（必须 source，不能直接执行，因为它会重设 `HOME`）：

```bash
source /online1/xuewei_group/xuewei/huanghp/set_env
```

`set_env` 会把 `HOME` 切到个人目录并 `cd` 过去、source 个人 libmys 环境、清理 module 后加载个人默认软件栈。如确需其他软件，再用 `module load` 追加。

个人环境中的平台标识为 `HUANGHAOPENG_PLATFORM_ID=qh`。

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。

## 作业提交与调度系统

本集群使用 Slurm 23.02 管理。登录节点 `login01` 只用于轻量操作，不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。若没有明确指定时间，默认按 30 分钟申请。

作业名一般使用 `<jobname>.huanghaopeng`，无特殊要求一律使用 `--exclusive`。

可用 Slurm 分区：

1. `intel`：默认分区，Intel Xeon Max 节点（`qhcn###`），数百节点，无时限
2. `amd`：AMD EPYC 大内存节点（`qhdn###`），3 节点，无时限
3. `intel_expr`：Intel 调试/试验分区，4 节点，时限 2 小时

```bash
tmux list-windows -t AIQH | grep -q 'salloc1' || tmux new-window -t AIQH -n salloc1
tmux send-keys -t AIQH:salloc1 'salloc -p intel -N <nodes> --exclusive -J test.huanghaopeng --time=0:30:00' Enter
```

## 节点硬件

Intel 节点 `qhcn###`：

```text
OS: Rocky Linux 8.9，Linux 4.18
CPU: Intel Xeon CPU Max 9462, 2 sockets × 32 cores = 64 cores/node
线程: 1 thread/core
NUMA: 8 个 domain，每域 8 核
内存: 约 503 GiB/node（RealMemory 514000 MB），无 swap，含片上 HBM
Cache/core: 48 KiB L1d + 32 KiB L1i + 2 MiB L2；lscpu 报告 75 MiB L3
指令集: AVX-512、AVX512-FP16/BF16、AMX、AVX-VNNI
网络: 1 × Mellanox ConnectX-6，100 Gb/s InfiniBand
```

AMD 节点 `qhdn###`：

```text
OS: Rocky Linux 8.9，Linux 4.18
CPU: AMD EPYC 9654，2 sockets × 96 cores = 192 cores/node
线程: 1 thread/core
NUMA: 8 个 domain，每域 24 核
内存: 约 1.5 TiB/node（RealMemory 1546000 MB），无 swap
Cache/core: 32 KiB L1d + 32 KiB L1i + 1 MiB L2；lscpu 报告 32 MiB L3
指令集: AVX-512、AVX512-BF16、AVX-VNNI
```

`/online1` 为约 17 PiB Lustre over InfiniBand。

## 互联网网络

QH 默认不能直接访问互联网。不要假设 `git clone` / `pip install` / `conda` 能联网，应优先使用已有 module 和本地资源；确需联网时先与用户确认代理或内网镜像。
