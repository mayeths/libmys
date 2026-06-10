---
name: kp-cluster
description: >-
  KP 鲲鹏 Slurm 集群操作规范。适用于 ssh host `kp`，以及用户要求在 kp 上运行、构建、测试、查看内容的场景。
---

# KP 集群 Practice

本文档只描述 KP 集群的平台操作方式，不包含具体项目规则。KP 是基于华为鲲鹏 920（aarch64）的 openEuler + Slurm 集群。

## 登录与连接

KP 集群默认通过本地 tmux 会话进入 `ssh kp`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh kp 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIKP` 可以保留工作路径、编译器环境、MPI 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIKP` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIKP 2>/dev/null || tmux new-session -d -s AIKP -n cmd1
tmux list-windows -t AIKP | grep -q 'cmd1' || tmux new-window -t AIKP -n cmd1
tmux capture-pane -t AIKP:cmd1 -p | tail -n 30
```

若集群长时间无响应，通常说明网络或登录连接已中断，需要重新 `ssh kp`。

## 命令与脚本

为了支持并行任务，`AIKP` 中的 tmux window 按 `cmd1`、`cmd2` 等命名。通常情况下，普通命令在默认的 `cmd1` 中运行。

如果还未进入 kp 集群，在对应 window 中发送：

```bash
tmux send-keys -t AIKP:cmd1 'ssh kp' Enter
```

不要在已经进入 kp 时重复执行 `ssh kp`，以免产生嵌套 SSH。

运行命令时，应在命令后加上完成标记，方便判断命令是否结束以及退出码：

```bash
tmux send-keys -t AIKP:cmd1 '<command>; echo __AIKP_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AIKP:cmd1 -p -S -120 | tail -n 80
```

如果没有看到 `__AIKP_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。

## 工作环境

KP 使用个人账号 `huanghp`，家目录为：

```text
/A/hpcauser/huanghp
```

共享存储统一挂在 `/A`（家目录、project、module 均在其下）。与 yc 不同，KP 登录 `bash` 时 `.bashrc` 会自动 `source ~/project/libmys/etc/profile` 加载个人环境，无需额外的启动脚本。环境就绪后：

- 模块系统为 Lmod（`module avail` / `module load`），个人模块树在 `~/module/CONFIG`。
- 默认已加载 `mpi/hpcx/2.21.3`（HPC-X OpenMPI），`mpicc` / `mpirun` 即来自 HPC-X；另有 mpich、mvapich、多版本 openmpi 可按需 `module load`。
- 系统自带 gcc / gfortran / cmake / make；可 `module load compiler/gcc/13.2.0` 等切换编译器。

始终使用登录默认的 `bash`，不要切换到 zsh。预期 bash prompt 类似：

```text
[huanghp@kunpeng-master ~]$
```

一般使用`rsync`传输文件（除非明确指示，否则不使用`--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。

## 作业提交与调度系统

本集群使用 Slurm 管理。登录节点 `kunpeng-master` 只用于轻量操作，不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。MPI 程序通常先在 `salloc` window 中申请资源，再从运行命令的 window 使用 `mpirun` 启动。若没有比较明确的指定 salloc 时间，默认按 30 分钟申请。

可用 Slurm 分区：

1. `ALL`：默认分区，8 个计算节点（kp101–kp108），默认时限 30 分钟
2. `LONG`：同一批节点，无时限，用于长作业

示例：

```bash
tmux list-windows -t AIKP | grep -q 'salloc1' || tmux new-window -t AIKP -n salloc1
tmux send-keys -t AIKP:salloc1 'salloc -p ALL -N <nodes> --exclusive --time=0:30:00' Enter
```

## 鲲鹏节点硬件

计算节点 `kp101`–`kp108`（共 8 台）大致配置如下：

```text
CPU: 华为 Kunpeng-920 (aarch64), 4 sockets × 32 cores = 128 cores/node
线程: 1 thread/core
NUMA: 4 个 NUMA domain，每路一域：0-31 / 32-63 / 64-95 / 96-127
内存: 约 244 GiB/node（RealMemory 250000 MB），无 swap
网络: Mellanox ConnectX-5, 100 Gb/s EDR InfiniBand（设备 ibp1s0）
指令集: ARMv8 NEON/asimd（含 asimddp 点积），无 SVE，无 AVX
```

登录节点 `kunpeng-master` 为 2 路 × 48 核、4 NUMA 的鲲鹏 920，配置与计算节点不同，仅作登录用途。

## 互联网网络

KP 计算与登录节点可**直接访问互联网**，无需配置 HTTP proxy（`git` / `curl` / `pip` 等可直连）。
