This is user skill reference "clusters/kp". Last modified: 2026-11-11.

# KP 集群 Practice

本文档只描述 KP 集群的平台操作方式，不包含具体项目规则。KP 是基于华为鲲鹏 920（aarch64）的 openEuler 24.03 LTS-SP2 + Slurm 集群。

## 登录与连接

KP 集群默认通过本地 tmux 会话进入 `ssh kp`。几乎不要使用一次性 `ssh kp 'cmd'`，这类方式只用于只读、不加载环境、能马上结束的探测。远程侧只能有一条命令，引号里的命令本身不超过 20 个字符，例如 `date`、`hostname`。通常情况、或者更长命令的情况都放进 tmux。使用本地持久 tmux 会话 `AIKP` 可以保留工作路径、编译器环境、MPI 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIKP` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIKP 2>/dev/null || tmux new-session -d -s AIKP -n cmd1
tmux list-windows -t AIKP | grep -q 'cmd1' || tmux new-window -t AIKP -n cmd1
tmux capture-pane -t AIKP:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

若集群长时间无响应，通常说明网络或登录连接已中断，需要重新 `ssh kp`。

## 命令与脚本

为了支持并行任务，`AIKP` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1` 等命名。通常情况下，普通命令在默认的 `cmd1` 中运行。

如果还未进入 kp 集群，在对应 window 中发送，再用 `poll_tmux_change.sh` 等待：

```bash
tmux send-keys -t AIKP:cmd1 'ssh kp' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIKP:cmd1 60 5 3 'ssh'
```

不要在已经进入 kp 时重复执行 `ssh kp`，以免产生嵌套 SSH。

### 短时间命令

预计 10 秒内结束的命令追加 `echo "__DONE_$?__"` 以获取退出码，`send-keys`、`sleep 5` 和 `capture-pane` 放在同一次 Shell tool call 中。最后一个 prompt 上方的 DONE 即本次结果；5 秒后没看到，就用 `poll_tmux_done.sh` 接着等，TID 传 `-`（见下方长时间命令）。

```bash
tmux send-keys -t AIKP:cmd1 '<command>; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AIKP:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

### 长时间命令

更久的命令追加完成标记 `__TASK_${TID}_DONE_$?__`，标记用 `${TID}` 拼接，不要写死（`TID` 按 `T001`、`T002` 递增），再用 skill 目录下的 `tools/poll_tmux_done.sh` 等待 DONE，多步骤任务可在中间打印 `__TASK_${TID}_STEP<n>_$?__`。快的任务 deadline 60 秒、间隔 5 秒；更长的任务每轮 180 秒、间隔 15 秒。每轮 deadline 和后台等待时长（如 `AwaitShell` 的 `block_until_ms`）都最长 300 秒（5分钟），用一两句话向用户汇报进度（最新 STEP、日志尾部），让用户知道任务仍在正常运行，而不是某个环节已经出错还在空等。接 `| tee` 时用 `${PIPESTATUS[0]}` 代替 `$?`。

`ssh`、`salloc` 等切换 shell 的命令无法追加标记，先发送，再用 `tools/poll_tmux_change.sh` 等待：最后一行不再含关键词（命令走完），且 pane 连续几轮不变（输出刷完）。关键词用命令名，不要用会出现在 prompt 里的节点名。`salloc` 排队时最后一行仍含 `salloc`，脚本会等到新 prompt 出现或到达 deadline。申请资源时 deadline 用 120 秒；超时就再开一轮，并说明还在排队。

```bash
tmux send-keys -t AIKP:cmd1 'export TID=T001; bash long_task.sh; echo "__TASK_${TID}_DONE_$?__"' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIKP:cmd1 T001 60 5
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIKP:cmd1 - 60 5
#
tmux send-keys -t AIKP:salloc1 'salloc ...' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIKP:salloc1 120 5 3 'salloc'
```

工具支持后台等待（如 Cursor 的 `AwaitShell`）时，后台等待`poll_tmux_done.sh`等脚本结束。

## 工作环境

KP 使用个人账号 `huanghp`，家目录为：

```text
/A/hpcauser/huanghp
```

共享存储 `/A` 通过 NFSv4 挂载（家目录、project、module 均在其下）。与 YC 不同，KP 登录 `bash` 时 `.bashrc` 会自动 `source ~/project/libmys/etc/profile` 加载个人环境，无需额外的启动脚本。环境就绪后：

- 平台标识环境变量为 `HUANGHAOPENG_PLATFORM_ID=kp`。
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

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。MPI 程序通常先在 `salloc` window 中申请资源，再从运行命令的 window 使用 `mpirun` 启动。`srun` 和 `mpirun` 按普通命令追加 DONE 或 TASK 标记。若没有比较明确的指定 salloc 时间，默认按 30 分钟申请。

作业名一般使用 `<jobname>.huanghaopeng` 形式，例如 `test.huanghaopeng`。作业无特殊要求，一律使用 `--exclusive`。

可用 Slurm 分区：

1. `ALL`：默认分区，8 个计算节点（kp101–kp108），默认时限 30 分钟
2. `LONG`：同一批节点，无时限，用于长作业

示例：

```bash
tmux list-windows -t AIKP | grep -q 'salloc1' || tmux new-window -t AIKP -n salloc1
tmux send-keys -t AIKP:salloc1 'ssh kp' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIKP:salloc1 60 5 3 'ssh'
tmux send-keys -t AIKP:salloc1 'salloc -p ALL -N <nodes> --exclusive -J test.huanghaopeng --time=0:30:00' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIKP:salloc1 120 5 3 'salloc'
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
