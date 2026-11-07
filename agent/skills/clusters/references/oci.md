This is user skill reference "clusters/oci". Last modified: 2026-11-05.

# OCI 平台 Practice

本文档只描述 OCI 平台的操作方式，不包含具体项目规则。OCI（`oci-hsg-cs-001`）是 NVIDIA MARS 集群，位于印尼 Batam，Ubuntu 24.04 + Slurm 25.11，计算节点为 GB200 NVL72 的 compute tray `nvl72xxx-Txx`（aarch64）。

## 登录与连接

OCI 默认通过本地 tmux 会话进入 `ssh oci`。几乎不要使用一次性 `ssh oci 'cmd'`，这类方式只用于只读、不加载环境、能马上结束的探测。远程侧只能有一条命令，引号里的命令本身不超过 20 个字符，例如 `date`、`hostname`。通常情况、或者更长命令的情况都放进 tmux。使用本地持久 tmux 会话 `AIOCI` 可以保留登录会话、Slurm allocation、计算节点工作路径和环境状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIOCI` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIOCI 2>/dev/null || tmux new-session -d -s AIOCI -n cmd1
tmux list-windows -t AIOCI | grep -q 'cmd1' || tmux new-window -d -t AIOCI -n cmd1
tmux capture-pane -t AIOCI:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

如果还未进入 OCI 登录节点，在对应 window 中发送：

```bash
tmux send-keys -t AIOCI:cmd1 'ssh oci' Enter
sleep 5
tmux send-keys -t AIOCI:cmd1 'bash' Enter
```

OCI 默认登录 shell 是 `/bin/csh`（prompt 为 `oci-hsg-cs-001-login-02%`）。登录后需先执行 `bash`（prompt 变为 `haopengh@oci-hsg-cs-001-login-02:~$`），再发送其他命令。`salloc` 返回的 shell 也是 csh。

不要在已经进入 OCI 时重复执行 `ssh oci`，以免产生嵌套 SSH。本机 SSH 配置中 `oci` 指向 `oci-hsg-cs-001-login-02.nvidia.com`，通过 `eos` 跳板进入。登录节点还有 `login-01`、`login-03`；数据传输用 Data Copier 节点 `oci-hsg-cs-001-dc-0[1-3].nvidia.com`，VS Code 只能用 `oci-hsg-cs-001-vscode-0[1-3].nvidia.com`。

## 命令与脚本

为了支持并行任务，`AIOCI` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1` 等命名。

登录节点只用于轻量操作、查询 Slurm、申请资源和管理文件，每个用户限 1 CPU、300 个进程、30 GB 内存。不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理、编译或长时间重负载任务。

### 短时间命令

预计 10 秒内结束的命令追加 `echo "__DONE_$?__"` 以获取退出码，`send-keys`、`sleep 5` 和 `capture-pane` 放在同一次 Shell tool call 中。最后一个 prompt 上方的 DONE 即本次结果；5 秒后没看到，就用 `poll_tmux_done.sh` 接着等，TID 传 `-`（见下方长时间命令）。标记依赖 bash，在 csh 中不可用。

```bash
tmux send-keys -t AIOCI:cmd1 '<command>; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AIOCI:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

### 长时间命令

更久的命令追加完成标记 `__TASK_${TID}_DONE_$?__`，标记用 `${TID}` 拼接，不要写死（`TID` 按 `T001`、`T002` 递增），再用 skill 目录下的 `tools/poll_tmux_done.sh` 等待 DONE，多步骤任务可在中间打印 `__TASK_${TID}_STEP<n>_$?__`。快的任务 deadline 60 秒、间隔 5 秒；更长的任务每轮 180 秒、间隔 15 秒。每轮 deadline 和后台等待时长（如 `AwaitShell` 的 `block_until_ms`）都最长 300 秒（5分钟），用一两句话向用户汇报进度（最新 STEP、日志尾部），让用户知道任务仍在正常运行，而不是某个环节已经出错还在空等。接 `| tee` 时用 `${PIPESTATUS[0]}` 代替 `$?`。

`ssh`、`salloc`、`srun --pty` 等切换 shell 的命令无法追加标记，先发送，再用 `tools/poll_tmux_change.sh` 等待：最后一行不再含关键词（命令走完），且 pane 连续几轮不变（输出刷完）。关键词用命令名，不要用会出现在 prompt 里的节点名。

```bash
tmux send-keys -t AIOCI:cmd1 'export TID=T001; bash long_task.sh; echo "__TASK_${TID}_DONE_$?__"' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIOCI:cmd1 T001 60 5
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIOCI:cmd1 - 60 5
#
tmux send-keys -t AIOCI:salloc1 'salloc ...' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIOCI:salloc1 120 5 3 'salloc'
```

工具支持后台等待（如 Cursor 的 `AwaitShell`）时，后台等待`poll_tmux_done.sh`等脚本结束。

## 工作环境与计算节点

默认工作流是：

1. 在登录节点（bash）申请一个计算节点。
2. `salloc` 成功后，shell 仍在登录节点，且为 csh；保持该 window 不退出，以维持 allocation。
3. 在该 window 中 `srun -N 1 --pty bash` 进入计算节点。
4. 在计算节点上进行项目构建、测试、运行和文件查看。

示例：

```bash
tmux list-windows -t AIOCI | grep -q 'salloc1' || tmux new-window -d -t AIOCI -n salloc1
tmux send-keys -t AIOCI:salloc1 'ssh oci' Enter
sleep 5
tmux send-keys -t AIOCI:salloc1 'bash' Enter
sleep 2
tmux capture-pane -t AIOCI:salloc1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

确认 `salloc1` 已在 OCI 登录节点的 bash prompt 后，申请节点：

```bash
tmux send-keys -t AIOCI:salloc1 'salloc -A hw_nresearch_snoise -p batch -q interactive -N 1 --gres=gpu:4 --exclusive -J hw_nresearch_snoise-hhp.test --time=0:30:00' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIOCI:salloc1 120 5 3 'salloc'
```

看到 `Nodes ... are ready for job` 后，在同一 window 进入计算节点：

```bash
tmux send-keys -t AIOCI:salloc1 'srun -N 1 --pty bash' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIOCI:salloc1 90 3 2 'srun'
```

预期 prompt 类似 `haopengh@nvl72173-T02:~$`。退出时先 `exit` 离开计算节点，再 `exit` 释放 allocation。如需确认当前 allocation，可在登录节点运行 `squeue -u $(whoami)`。

注意：在 `srun --pty bash` 进去的 shell 里再起多节点 step 时，会继承 `SLURM_NTASKS=1`，`srun -N 4 --ntasks-per-node=1` 只会落在 1 个节点上，多节点程序一直等不到其他 rank。要显式写 `-n <节点数>`（如 `srun -N 4 -n 4 --ntasks-per-node=1 ...`），或者从登录节点的 salloc shell 里起 step（261107 W050-012 踩到）。

## 工作环境

OCI 使用个人账号 `haopengh`，默认 shell 为 `csh`。

```text
登录节点: oci-hsg-cs-001-login-02.nvidia.com
家目录: /home/haopengh（NFS，配额 10 GB，有每日快照）
Lustre: /lustre/fsw（符号链接集合）、/lustre/fs1，各约 20 PB
项目目录: /lustre/fs1/portfolios/hw/projects/hw_nresearch_snoise（/lustre/fsw/portfolios/hw/projects/... 是同一块存储）
个人目录: /lustre/fs1/portfolios/hw/projects/hw_nresearch_snoise/users/haopengh（已建，家目录下有软链 ~/oci-fs 指向它）
本地盘: 计算节点 /raid（含 /raid/scratch），约 21 TB，作业之间清空；/ 约 6.5 TB
平台标识: HUANGHAOPENG_PLATFORM_ID 未设置
```

大文件放个人目录（`~/oci-fs`），不要放只有 10 GB 的家目录。配额（2026-11-07 实测）：用户在 fs1 和 fsw 上各 100 TB / 50M inodes；项目 `hw_nresearch_snoise`（组共享）25 TB / 50M inodes，会先撞到项目配额。Lustre 整体 20 PB，已用约 95%，写大数据时留意变慢或失败。已实测在个人目录 `fallocate -l 25G` 成功（登录节点上耗时约 2.5 分钟，配额按实占计费），测试文件已删除。

存储与 inode 用量用 `/cm/shared/apps/scripts/fs-quota-status` 查看（运行较慢，约 1 分钟；登录节点没有 `quota` 命令，家目录里已有一份拷贝 `~/fs-quota-status`）。Lustre 有 stale data 策略：个人 stale 数据超过 15 TiB 会被禁止提交作业。

登录节点没有 `module` 和 `enroot`。计算节点自带软件栈：

- CUDA：驱动 580.126.20，CUDA 13.0；宿主机没有 `nvcc`。
- MPI：系统 MLNX OFED 自带 `/usr/mpi/gcc/openmpi-4.1.9a1`。
- 容器：`enroot`，`srun` 支持 Pyxis 的 `--container-image`。节点为 aarch64，只能使用 ARM 容器，x86 容器不可用。

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。集群在印尼，到美国的传输较慢，大量数据应走 Data Copier 节点或 Data Mover，不要在登录节点上传输。

## 作业提交与调度系统

本平台使用 Slurm 25.11.6，按 QOS 调度。提交作业必须同时指定 `-A hw_nresearch_snoise`、`-p` 和 `-q`；GPU 分区必须申请 `--gres=gpu:4`，否则报错 `Cannot find GPU specification`。不允许部分节点，节点总是独占（4 块 GPU）。`salloc` 返回的 shell 留在登录节点，真正运行需要 `srun`。若没有比较明确的指定时间，默认按 30 分钟申请。

作业名一般使用 `hw_nresearch_snoise-hhp.<jobname>` 形式，例如 `hw_nresearch_snoise-hhp.test`。

分区：

1. `batch`：默认，最长 4 小时，默认 31 分钟
2. `batch_long`：4 小时到 7 天
3. `cpu`：纯 CPU 作业（`gres/gpu=0`），最长 7 天，55 台 AmpereOne 96 核节点
4. `cpu_datamover`：跨集群传数据，不限时

账号 `hw_nresearch_snoise` 可用的主要 QOS：

1. `interactive`：交互调试，每人最多 4 个节点，最多提交 8 个作业
2. `short`：最长 2 小时、每人 64 个节点、同时运行 2 个，2 小时内不会被抢占
3. `normal`：常规批处理，每人最多 1512 个节点
4. `free`：不计 fairshare，但随时可能被高优先级作业抢占并 requeue
5. `cpu-short`、`cpu-normal`、`cpu-long`、`cpu-interactive`：纯 CPU 作业

`batch` 分区内 `normal` 作业不会被抢占；`batch_long` 中的 `interactive` 作业运行超过 4 小时 05 分会被驱逐。多节点作业尽量放在同一个 NVL72 机柜（18 个节点，节点名前缀相同）内，用 `--segment` 控制，默认按 16 个节点为一段。

常用查询：

```bash
sinfo -s -o '%P|%a|%l|%F'
sacctmgr -nP show assoc user=$(whoami) format=account,partition,qos
squeue -u $(whoami)
sshare -U
```

根据任务需求可调整节点数和时间。不要在没有用户确认的情况下长时间占用大量节点。

## 节点硬件

`nvl72xxx-Txx` 计算节点为 GB200 compute tray，代表节点 `nvl72173-T02` 实测配置：

```text
OS: Ubuntu 24.04.1 LTS，Linux 6.8.0-1046-nvidia-64k，aarch64
CPU: 2 × Grace（Neoverse-V2），2 sockets × 72 cores = 144 cores，1 thread/core
NUMA: 34 个 node，其中 node 0/1 为两个 socket 的内存（各约 490 GB），其余为 0 MB
内存: 约 952 GiB/node
Cache: 每核 1 MiB L2；每 socket 114 MiB L3
GPU: 4 × NVIDIA GB200，每块 189471 MiB（约 186 GB HBM3E），NVLink 5，同一 NVL72 机柜 18 个节点 72 块 GPU 互联
GPU 驱动: 580.126.20，CUDA 13.0
网络: mlx5_0/1/3/4 为 400 Gb/s NDR InfiniBand 计算网络；mlx5_2/5 为 200 Gb/s Ethernet（前端与存储）
Slurm GRES: gpu:4
```

NCCL 和 UCX 的关键参数已由集群设置（如 `NCCL_IB_HCA=mlx5_0,mlx5_1,mlx5_3,mlx5_4`、`NCCL_MNNVL_ENABLE=1`），不要覆盖。NCCL 需 2.26 以上；NVLS 默认关闭，需要时在容器中设置 `NCCL_NVLS_ENABLE=1`。遇到 UCX `no usable transports/devices` 时设置 `UCX_NET_DEVICES=mlx5_0:1,mlx5_1:1,mlx5_3:1,mlx5_4:1` 并用 `srun --container-env=UCX_NET_DEVICES` 传入；NCCL 出现段错误时先 `ulimit -s 8192`。

登录节点是 96 核 Ampere（aarch64）、191 GiB 内存，Ubuntu 24.04.1，不能用来代替计算节点数据。

节点状态经常变化，运行前以 `sinfo` 为准，避开 `down`、`drain` 等不可用节点。

## 互联网网络

登录节点和计算节点都没有设置 `http_proxy` / `https_proxy`，但实测可以直接访问 `pypi.org` 和 `github.com`，`git clone`、`pip install` 等无需代理。
