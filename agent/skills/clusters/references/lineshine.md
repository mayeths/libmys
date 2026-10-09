This is user skill reference "clusters/lineshine". Last modified: 2026-10-09.

# LineShine 集群 Practice

本文档只描述 LineShine 集群，不包含具体项目规则。LineShine 是大规模鲲鹏 ARM 集群。LineShine 也称 Shenchao、深超、国家超级计算深圳中心二期等。

## 登录与连接

LineShine 默认通过本地 tmux 会话进入 `ssh lineshine`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh lineshine 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AILS` 可以保留登录、工作路径、环境等，用户也可以随时 attach 查看或协助。

查看或创建会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AILS 2>/dev/null || tmux new-session -d -s AILS -n cmd1
tmux list-windows -t AILS | grep -q 'cmd1' || tmux new-window -t AILS -n cmd1
tmux capture-pane -t AILS:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20
```

如果尚未进入 LineShine，在对应 window 中发送：

```bash
tmux send-keys -t AILS:cmd1 'ssh lineshine' Enter
```

不要在已经进入 LineShine 时重复执行 SSH，以免产生嵌套连接。若长时间无响应，通常说明本地内网、VPN、跳板或 SSH 连接已经中断，需要重新连接。当前本机 SSH 使用 ControlMaster 复用连接；首次连接可能需要认证，后续连接可能复用已有 control socket。

```
ssh lineshine # 新账号
ssh lineshine_old # DIDA 项目中本人使用的账号
ssh lineshine_zyx # DIDA 项目中周永潇使用的账号
```

## 命令与脚本

为支持并行任务，tmux window 按 `cmd1`、`cmd2` 等命名。普通命令默认在 `cmd1` 中运行。

发送命令时追加完成标记：

```bash
tmux send-keys -t AILS:cmd1 '<command>; echo __AILS_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AILS:cmd1 -p -J -S -200 | grep -v '^$' | tail -n 80
```

如果没有看到 `__AILS_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。
如果使用类似 `tmux capture-pane -t AILS:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20` 的 capture 功能发现命令未结束，后面不应给用户 `sleep 20; tmux capture-pane -t AILS:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20` 这样的命令。
应去掉其中的 `sleep 20`，由用户确认运行结束后手动点击确认，直接执行 `tmux capture-pane -t AILS:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20`。
如果需要 sleep，则一般 sleep 5 或 10 秒；如果 10 秒后还未结束，则再 sleep 10 秒。如果能确认会等得久一些，可以停止自我 sleep 等待，让用户确认结束后唤醒。

## 工作环境与文件同步

当前个人家目录为：

```text
/home/share/nsls_huanghaopeng
```

项目应放在该目录下的个人子目录中。第一次同步某个项目时，先检查项目根目录 `.vscode/sftp.json`；若其中没有 LineShine 目标路径，应向用户确认，不要默认写入旧账号目录。

一般使用 `rsync` 传输文件，除非用户明确要求，否则不使用 `--delete`。使用 `rsync` 或 `scp` 时使用绝对路径，不依赖 `~`。实验输出通常较大，向集群同步源码时应明确排除本地结果目录；从集群回收结果时只同步需要的输出。

## Module 与编译环境

登录后默认没有加载 module。2026 年 9 月实验使用过以下环境：

```bash
module purge
module unuse /work_ssd/software/HPCKit/latest/modulefiles
module use /work_ssd/software/modulefile
module use /work_ssd/software/HPCKit/25.2.1/modulefiles
module load tool/cmake/3.28.2
module load gcc/compiler12.3.1/gccmodule gcc/hmpi25.2.1/release
export OMPI_CC=gcc
export OMPI_CXX=g++
```

## 作业提交与调度系统

本集群使用 Donau Scheduler，不是 Slurm。登录节点只用于轻量操作、编译、编辑、查询调度系统、提交作业和管理文件。不要在登录节点运行 benchmark、多节点 MPI/OpenMP、训练、推理或长时间重负载任务；登录节点没有计算节点的 RDMA 网络环境。

常用命令：

```bash
dinfo -q # 查看当前可用的队列的，类似sinfo
djob # 查看已提交的任务，类似squeue
djob -l <job_id> # 查看任务的具体细节
dkill <job_id> # 杀死任务
```

当前队列为 `q_grapes`。节点状态和队列随时变化，提交前重新执行 `dinfo -q`，只把 `OK` 节点视为当前可用；不把旧实验的坏节点排除列表永久写进统一 Skill。

典型 `dsub` 参数：

```bash
dsub \
  -q q_grapes \
  -nn <nodes> \
  --replica <nodes> \
  -n <job_name> \
  -x job \
  --mpi hmpi \
  -R "cpu=<cpus_per_replica>" \
  -T <timeout_seconds> \
  -oo <stdout_path> \
  -eo <stderr_path> \
  <executable_job_script>
```

注意：

1. 作业脚本必须有执行权限，否则会以 exit code 126 失败。
2. `-oo` / `-eo` 使用覆盖模式，适合避免重试作业把日志混在一起；如需追加必须明确选择追加参数。
3. 使用 `dsub -s <script>` 时才依赖脚本中的 `#DSUB` 指令。直接把脚本作为可执行文件提交时，关键资源和 MPI 参数应显式写在 `dsub` 命令行中。
4. stdout 和 stderr 分开检查；MPI、UCX 和 launcher 错误通常出现在 stderr。
5. 作业没有生成 rankfile 或 stdout 时，可能在调度器、容器或 launcher 阶段就已经失败，不能直接归因于应用算法。
6. `dkill` 会终止作业，只有用户要求取消或当前任务明确需要时才能使用。

## CCS_ALLOC_FILE、rankfile 与 MPI

调度器通过 `$CCS_ALLOC_FILE` 提供分配节点。`mpirun` 不应被假定会自动按项目要求读取和绑定这些资源；大规模作业通常需要从该文件生成 hostfile 或 rankfile。

2026 年 9 月实验采用每节点 16 个 MPI rank、每个 rank 对应一个 NUMA domain 的布局；每个 NUMA 有 38 个 CPU slot，OpenMP 通常使用其中 36 线程。该配置适合当时的鲲鹏计算节点，但新项目使用前仍应在计算节点核对 CPU、NUMA 和调度器分配。

跨节点所需环境变量应使用 `mpirun -x` 显式传播，包括 `PATH`、`LD_LIBRARY_PATH`、OpenMP、UCX 和项目参数。不要假设在作业脚本中 `export` 后所有远端 MPI 进程都能自动获得相同环境。

## 计算节点与网络

登录节点 `login01` 实测配置：

```text
OS: Kylin Linux Advanced Server V10，Linux 5.10，aarch64
CPU: Kunpeng-920，2 sockets × 64 cores = 128 cores，1 thread/core
NUMA: 4 个 domain，每域 32 核
Cache/core: 64 KiB L1d + 64 KiB L1i + 512 KiB L2
L3: 4 × 32 MiB = 128 MiB
内存: 约 502 GiB，无 swap
网络: 2 × 100 Gb/s RoCE
```

`q_grapes` 计算节点 `cn*` 实测配置：

```text
OS: Kylin Linux Advanced Server V10，Linux 5.10，aarch64
CPU: 608 cores/node，2 sockets × 304 cores，1 thread/core
CPU NUMA: 16 个 domain，每域 38 核
扩展 NUMA: 另有 16 个无 CPU 的 4 GiB memory-only domain，共 32 个 NUMA node
Cache/core: 32 KiB L1d + 32 KiB L1i + 768 KiB L2；未报告 L3
内存: 约 565 GiB/node，无 swap
GPU/加速器: 无
RoCE: 8 × 200 Gb/s，设备 roceroh0–roceroh7，netdev roh0–roh7
共享存储: /home/share 为约 2.5 PiB DTFS；/work_ssd/software 为约 10 TiB DTFS
历史布局: 16 MPI ranks/node，每个 rank 绑定一个含 CPU 的 NUMA domain
```

当时 HMPI/UCX 作业常用：

```bash
export UCX_TLS=rc
export UCX_NET_DEVICES=roceroh0:1,roceroh1:1,roceroh2:1,roceroh3:1,roceroh4:1,roceroh5:1,roceroh6:1,roceroh7:1
export UCX_RC_VERBS_ROCE_LOCAL_SUBNET=y
export UCX_UD_VERBS_ROCE_LOCAL_SUBNET=y
export UCX_RNDV_THRESH=512K
export UCX_UD_VERBS_TX_QUEUE_LEN=1024
export UCX_UD_VERBS_RX_QUEUE_LEN=32768
export UCX_UD_VERBS_TIMEOUT=720s
export UCX_RC_TIMEOUT=720s
export UCX_RC_RETRY_COUNT=6
```

这些是历史上验证过的实验配置，不是所有项目的强制默认值。UCX transport、RNDV threshold、多 rail 和网卡绑定会影响性能，只有项目需要或用户要求时才固定；否则先沿用项目脚本并做小规模验证。

## 互联网与本地网络

本机能否连接 LineShine 取决于当前内网、VPN或跳板状态。不要把 2026 年 9 月现场使用过的临时 Wi-Fi、SOCKS 代理、共享账号或口令写入脚本和 Skill。远端是否能访问互联网尚未作为稳定能力确认；执行 `git`、`curl`、`pip` 等外部访问前先做轻量探测或向用户确认代理与镜像。
