This is user skill reference "clusters/kp". Last modified: 2026-10-09.

# KP 集群 Practice

本文档只描述 KP 集群的平台操作方式，不包含具体项目规则。KP 是基于华为鲲鹏 920（aarch64）的 openEuler 24.03 LTS-SP2 + Slurm 集群。

## 登录与连接

KP 集群默认通过本地 tmux 会话进入 `ssh kp`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh kp 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIKP` 可以保留工作路径、编译器环境、MPI 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIKP` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIKP 2>/dev/null || tmux new-session -d -s AIKP -n cmd1
tmux list-windows -t AIKP | grep -q 'cmd1' || tmux new-window -t AIKP -n cmd1
tmux capture-pane -t AIKP:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20
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
tmux capture-pane -t AIKP:cmd1 -p -J -S -200 | grep -v '^$' | tail -n 80
```

如果没有看到 `__AIKP_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。
如果使用类似 `tmux capture-pane -t AIKP:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20` 的 capture 功能发现命令未结束，则后面不应给用户 `sleep 20; tmux capture-pane -t AIKP:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20` 这样的命令。
应去掉其中的 `sleep 20`，由用户确认运行结束后手动点击确认，直接执行 `tmux capture-pane -t AIKP:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20`。
如果需要 sleep，则一般 sleep 5 或 10 秒；如果 10 秒后还未结束，则再 sleep 10 秒。如果能确认会等得久一些，可以停止自我 sleep 等待，让用户来确认结束后唤醒。

## 工作环境

KP 使用个人账号 `huanghp`，家目录为：

```text
/A/hpcauser/huanghp
```

共享存储 `/A` 通过 NFSv4 挂载（家目录、project、module 均在其下）。与 YC 不同，KP 登录 `bash` 时 `.bashrc` 会自动 `source ~/project/libmys/etc/profile` 加载个人环境，无需额外的启动脚本。环境就绪后：

- 模块系统为 Lmod（`module avail` / `module load`），个人模块树在 `~/module/CONFIG`。
- 默认已加载 `mpi/hpcx/2.21.3`（HPC-X OpenMPI），`mpicc` / `mpirun` 即来自 HPC-X；另有 mpich、mvapich、多版本 openmpi 可按需 `module load`。
- 系统自带 gcc / gfortran / cmake / make；可 `module load compiler/gcc/13.2.0` 等切换编译器。

始终使用登录默认的 `bash`，不要切换到 zsh。预期 bash prompt 类似：

```text
[huanghp@kunpeng-master ~]$
```

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。

## 作业提交与调度系统

本集群使用 Slurm 管理。登录节点 `kunpeng-master` 只用于轻量操作，不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。MPI 程序通常先在 `salloc` window 中申请资源，再从运行命令的 window 使用 `mpirun` 启动。若没有比较明确的指定 salloc 时间，默认按 30 分钟申请。

作业名一般使用 `<jobname>.huanghaopeng` 形式，例如 `test.huanghaopeng`。作业无特殊要求，一律使用 `--exclusive`。

可用 Slurm 分区：

1. `ALL`：默认分区，8 个计算节点（kp101–kp108），默认时限 30 分钟
2. `LONG`：同一批节点，无时限，用于长作业

示例：

```bash
tmux list-windows -t AIKP | grep -q 'salloc1' || tmux new-window -t AIKP -n salloc1
tmux send-keys -t AIKP:salloc1 'salloc -p ALL -N <nodes> --exclusive -J test.huanghaopeng --time=0:30:00' Enter
```

## 鲲鹏节点硬件

计算节点 `kp101`–`kp108`（共 8 台）实测配置如下：

```text
OS: openEuler 24.03 LTS-SP2，Linux 6.6，aarch64
CPU: 华为 Kunpeng-920，128 cores/node，1 thread/core
拓扑: lscpu 报告 2 sockets × 64 cores；Slurm 报告 4 sockets × 32 cores
NUMA: 4 个 domain，每域 32 核：0-31 / 32-63 / 64-95 / 96-127
Cache/core: 64 KiB L1d + 64 KiB L1i + 512 KiB L2
L3: 4 × 64 MiB = 256 MiB，每个 NUMA domain 一个实例
内存: 约 244 GiB/node（Slurm RealMemory 250000 MB），4 GiB swap
GPU/加速器: 无
网络: 2 × Mellanox ConnectX-6 InfiniBand，每端口 100 Gb/s
网卡亲和性: ibp4s0 位于 NUMA 0，ibp129s0 位于 NUMA 2
指令集: ARMv8 NEON/asimd（含 asimddp 点积），无 SVE，无 AVX
本地盘: 2 × 279.4 GiB SAS
共享盘: /A，NFSv4，约 17 TiB
```

登录节点 `kunpeng-master` 也是 openEuler 24.03 LTS-SP2，但硬件与计算节点不同：

```text
CPU: 华为 Kunpeng-920，2 sockets × 48 cores = 96 cores，1 thread/core
NUMA: 4 个 domain，每域 24 核
Cache/core: 64 KiB L1d + 64 KiB L1i + 512 KiB L2
L3: 4 × 48 MiB = 192 MiB
内存: 约 250 GiB，4 GiB swap
网络: 1 × 100 Gb/s InfiniBand（ibp1s0）及管理网络
```

登录节点仅用于轻量操作，不能用其硬件数据代替计算节点结果。

## 互联网网络

KP 计算与登录节点可直接访问互联网，无需配置 HTTP proxy（`git` / `curl` / `pip` 等可直连）。
