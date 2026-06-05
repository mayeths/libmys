---
name: qh-cluster
description: >-
  QH Slurm 集群操作规范。适用于 ssh host `qh`，以及用户要求在 qh 上运行、构建、测试、查看内容的场景。
---

# QH 集群 Practice

本文档只描述 QH 集群的平台操作方式，不包含具体项目规则。QH 是 Rocky Linux 8.9（x86_64）+ Slurm 的异构集群，含 Intel Xeon Max（HBM）与 AMD EPYC 两类节点。

## 登录与连接

QH 集群默认通过本地 tmux 会话进入 `ssh qh`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh qh 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIQH` 可以保留工作路径、编译器环境、MPI 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIQH` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIQH 2>/dev/null || tmux new-session -d -s AIQH -n cmd1
tmux list-windows -t AIQH | grep -q 'cmd1' || tmux new-window -t AIQH -n cmd1
tmux capture-pane -t AIQH:cmd1 -p | tail -n 30
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
tmux capture-pane -t AIQH:cmd1 -p -S -120 | tail -n 80
```

如果没有看到 `__AIQH_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。

## 加载环境

QH 使用共享账号 `xuewei`（组 `xuewei_group`）。始终使用登录默认的 `bash`。常用目录：

```text
个人目录: /online1/xuewei_group/xuewei/huanghp   # 即 ~/data/huanghp，工作与数据都放这里
并行盘:   /online1                                # Lustre（over IB），上面的个人目录就在此盘
账号家目录: /home/xuewei_group/xuewei             # /home，共享账号家目录，不要把个人产物堆这里
软件栈:   /apps                                   # 模块与应用
```

工作和算例一律放在个人目录 `/online1/xuewei_group/xuewei/huanghp`（`~/data/huanghp`），与共享账号其他用户隔离。

进入 qh 后，加载环境**统一 source 个人 `set_env`**（必须 source，不能直接执行，因为它会重设 `HOME`）：

```bash
source /online1/xuewei_group/xuewei/huanghp/set_env
```

`set_env` 会：把 `HOME` 切到个人目录并 `cd` 过去、`source` 个人 `libmys/etc/profile`、`module purge` 后加载 `intel/gcc/13.2.0`、`intel/openmpi/4.1.6/gcc13.2.0`、`intel/cmake/3.26.3`、`intel/python/3.9.19`。这是默认编译/运行环境，无需再手动逐个 `module load`。

如确需其他软件，再用 `module load` 追加（Lmod，模块树 `/apps/support/modulefiles`，Intel 节点用 `intel/*`、AMD 节点用 `amd/*`，`module avail` 查看完整列表）。使用 `scp` / `rsync` 时不要依赖 `~`，应使用绝对路径。

## 作业提交与调度系统

本集群使用 Slurm 23.02 管理。登录节点 `login01` 只用于轻量操作，不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。MPI 程序通常先在 `salloc` window 中申请资源，再从运行命令的 window 使用 `mpirun` 启动。若没有比较明确的指定 salloc 时间，默认按 30 分钟申请。

可用 Slurm 分区：

1. `intel`：默认分区，Intel Xeon Max 节点（`qhcn###`），数百节点，无时限
2. `amd`：AMD EPYC 大内存节点（`qhdn###`），3 节点，无时限
3. `intel_expr`：Intel 调试/试验分区，4 节点，时限 2 小时

示例：

```bash
tmux list-windows -t AIQH | grep -q 'salloc1' || tmux new-window -t AIQH -n salloc1
tmux send-keys -t AIQH:salloc1 'salloc -p intel -N <nodes> --exclusive --time=0:30:00' Enter
```

## 节点硬件

Intel 节点 `qhcn###`（`intel` / `intel_expr` 分区）：

```text
CPU: Intel Xeon CPU Max 9462 (Sapphire Rapids + HBM2e), 2 sockets × 32 cores = 64 cores/node
线程: 1 thread/core
NUMA: 8 个 NUMA domain（每路 SNC4），每域 8 核：0-7 / 8-15 / ... / 56-63
内存: 约 502 GiB/node（RealMemory 514000 MB），含片上 HBM
指令集: AVX-512（含 AVX512-FP16/BF16）、AMX、AVX-VNNI
```

AMD 节点 `qhdn###`（`amd` 分区）：

```text
CPU: AMD EPYC (Genoa 级), 2 sockets × 96 cores = 192 cores/node
内存: 约 1.5 TiB/node（RealMemory 1546000 MB）
```

互连：Mellanox ConnectX HDR InfiniBand，100 Gb/s（2X HDR，设备 mlx5_0）；`/online1` 为 Lustre over IB（o2ib）。

## 互联网网络

QH **默认不能直接访问互联网**（无 HTTP proxy，外部域名无法解析）。不要假设 `git clone` / `pip install` / `conda` 能联网，应优先使用 `/apps` 下已有的 module 软件与本地资源；确需联网时先与用户确认可用的代理或内网镜像。
