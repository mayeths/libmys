---
name: yc-cluster
description: >-
  YC 盐城 Slurm 集群操作规范。适用于 ssh host `yc`，以及用户要求在 yc 上运行、构建、测试、查看内容的场景。
---

# YC 集群 Practice

本文档只描述 YC 集群的平台操作方式，不包含具体项目规则。

## 登录与连接

YC 集群默认通过本地 tmux 会话进入 `ssh yc`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh yc 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIYC` 可以保留工作路径、编译器环境、MPI 环境、CUDA 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIYC` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIYC 2>/dev/null || tmux new-session -d -s AIYC -n cmd1
tmux list-windows -t AIYC | grep -q 'cmd1' || tmux new-window -t AIYC -n cmd1
tmux capture-pane -t AIYC:cmd1 -p | tail -n 30
```

集群 VPN 与登录会话约 30 分钟空闲后可能断开。若集群长时间无响应，通常说明网络或登录连接已中断，需要重新 `ssh yc`。当前 SSH 已使用 `~/.ssh/control-XXX` 做连接持久化。

## 命令与脚本

为了支持并行任务，`AIYC` 中的 tmux window 按 `cmd1`、`cmd2` 等命名。通常情况下，普通命令在默认的 `cmd1` 中运行。

如果还未进入 yc 集群，在对应 window 中发送：

```bash
tmux send-keys -t AIYC:cmd1 'ssh yc' Enter
```

不要在已经进入 yc 时重复执行 `ssh yc`，以免产生嵌套 SSH。

运行命令时，应在命令后加上完成标记，方便判断命令是否结束以及退出码：

```bash
tmux send-keys -t AIYC:cmd1 '<command>; echo __AIYC_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AIYC:cmd1 -p -S -120 | tail -n 80
```

如果没有看到 `__AIYC_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。

## 工作环境

共享账号为 `xuew`，个人环境家目录是：

```text
/online1/ycsc_xuew/xuew/huanghp-home
```

进入盐城集群后，应运行 `huanghp-zsh` 启动个人 zsh，以使用个人编译环境、Python 环境等配置。预期 prompt 类似：

```text
xuew@psn002 ~ ❯
```

一般使用`rsync`传输文件（除非明确指示，否则不使用`--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。

## 作业提交与调度系统

本集群使用 Slurm 管理。登录节点 `psn002` 等只用于轻量操作，不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。MPI 程序通常先在 `salloc` window 中申请资源，再从运行命令的 window 使用 `mpirun` 启动。若没有比较明确的指定 salloc 时间，默认按 30 分钟申请。

常用 Slurm 分区：

1. `q_amd_share`：默认 AMD EPYC 分区，bn 节点
2. `q_intel_share`：Intel 分区，dn 节点

示例：

```bash
tmux list-windows -t AIYC | grep -q 'salloc1' || tmux new-window -t AIYC -n salloc1
tmux send-keys -t AIYC:salloc1 'salloc -p q_amd_share -N <nodes> --exclusive --time=0:30:00' Enter
```

## AMD 节点硬件

`q_amd_share` / `bn` 节点大致配置如下：

```text
CPU: AMD EPYC 7H12, 2 sockets × 64 cores = 128 cores/node
线程: 1 thread/core
NUMA: 2 个 NUMA domain，0-63 与 64-127
内存: 约 188 GiB/node，无 swap
网络: Mellanox ConnectX-5, 100 Gb/s EDR InfiniBand
指令集: AVX2，无 AVX-512
```

## 互联网网络

yc 访问互联网需要 HTTP proxy，默认 `.bashrc` 已设置：

```bash
export http_proxy=http://174.0.250.13:3128
export https_proxy=http://174.0.250.13:3128
```
