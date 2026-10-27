This is user skill reference "clusters/gaia". Last modified: 2026-10-26.

# GAIA 平台 Practice

本文档只描述 GAIA 平台的操作方式，不包含具体项目规则。GAIA 是 NVIDIA 内部 Ubuntu 22.04 + Slurm GPU 平台，登录节点为 `tlv01-e2e-slurm12`，计算节点为 `dgx-gaia-*`。

## 登录与连接

GAIA 默认通过本地 tmux 会话进入 `ssh gaia`。几乎不要使用一次性 `ssh gaia 'cmd'`，这类方式只用于只读、不加载环境、能马上结束的探测。远程侧只能有一条命令，引号里的命令本身不超过 20 个字符，例如 `date`、`hostname`。通常情况、或者更长命令的情况都放进 tmux。使用本地持久 tmux 会话 `AIGAIA` 可以保留登录会话、Slurm allocation、计算节点工作路径和环境状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIGAIA` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIGAIA 2>/dev/null || tmux new-session -d -s AIGAIA -n cmd1
tmux list-windows -t AIGAIA | grep -q 'cmd1' || tmux new-window -d -t AIGAIA -n cmd1
tmux capture-pane -t AIGAIA:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

如果还未进入 GAIA 登录节点，在对应 window 中发送：

```bash
tmux send-keys -t AIGAIA:cmd1 'ssh gaia' Enter
```

不要在已经进入 GAIA 时重复执行 `ssh gaia`，以免产生嵌套 SSH。若平台长时间无响应，通常说明网络、跳板或登录连接已中断，需要重新连接。本机 SSH 配置中 `gaia` 通过 `hpclogin` 跳板进入，`dgx-gaia-*` 可通过 `gaia` 作为 ProxyJump。

## 命令与脚本

为了支持并行任务，`AIGAIA` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1`、`node1` 等命名。

登录节点 `tlv01-e2e-slurm12` 只用于轻量操作、查询 Slurm、申请资源和管理文件。不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

### 短时间命令

预计 10 秒内结束的命令追加 `echo "__DONE_$?__"` 以获取退出码，`send-keys`、`sleep 5` 和 `capture-pane` 放在同一次 Shell tool call 中。最后一个 prompt 上方的 DONE 即本次结果；5 秒后没看到，就用 `poll_tmux_done.sh` 接着等，TID 传 `-`（见下方长时间命令）。

```bash
tmux send-keys -t AIGAIA:cmd1 '<command>; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AIGAIA:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

### 长时间命令

更久的命令追加完成标记 `__TASK_${TID}_DONE_$?__`，标记用 `${TID}` 拼接，不要写死（`TID` 按 `T001`、`T002` 递增），再用 skill 目录下的 `tools/poll_tmux_done.sh` 等待 DONE，多步骤任务可在中间打印 `__TASK_${TID}_STEP<n>_$?__`。快的任务 deadline 60 秒、间隔 5 秒；更长的任务每轮 180 秒、间隔 15 秒。接 `| tee` 时用 `${PIPESTATUS[0]}` 代替 `$?`。

`ssh`、`salloc`、`srun --pty` 等切换 shell 的命令无法追加标记，先发送，再用 `tools/poll_tmux_change.sh` 等待：最后一行不再含关键词（命令走完），且 pane 连续几轮不变（输出刷完）。关键词用命令名，不要用会出现在 prompt 里的节点名。

```bash
tmux send-keys -t AIGAIA:cmd1 'export TID=T001; bash long_task.sh; echo "__TASK_${TID}_DONE_$?__"' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIGAIA:cmd1 T001 60 5
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIGAIA:cmd1 - 60 5
#
tmux send-keys -t AIGAIA:cmd1 'salloc ...' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIGAIA:cmd1 60 5 3 'salloc'
```

工具支持后台等待（如 Cursor 的 `AwaitShell`）时，后台等待`poll_tmux_done.sh`等脚本结束。

## 工作环境与计算节点

GAIA 的工作路径是：

```text
~/gaia-fs
```

`~/gaia-fs` 只有在 `salloc` 之后进入计算节点才可访问。不要在登录节点直接 `cd ~/gaia-fs` 或假设 `/mnt/lustre/gaia/...` 已挂载可用。

默认工作流是：

1. 在登录节点申请一个计算节点。
2. 保持 `salloc` window 不退出，以维持 allocation。
3. `ssh` 到分配到的 `dgx-gaia-*` 计算节点。
4. 在计算节点上 `cd ~/gaia-fs` 后再进行项目构建、测试、运行和文件查看。

示例：

```bash
tmux list-windows -t AIGAIA | grep -q 'salloc1' || tmux new-window -d -t AIGAIA -n salloc1
tmux send-keys -t AIGAIA:salloc1 'ssh gaia' Enter
sleep 5
tmux capture-pane -t AIGAIA:salloc1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

确认 `salloc1` 已在 GAIA 登录节点 prompt 后，申请节点：

```bash
tmux send-keys -t AIGAIA:salloc1 'salloc -p GAIA -N 1 --exclusive -J test.huanghaopeng --time=0:30:00' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIGAIA:salloc1 60 5 3 'salloc'
```

看到 `Nodes ... are ready for job` 后，在单独 window 进入该计算节点：

```bash
tmux list-windows -t AIGAIA | grep -q 'node1' || tmux new-window -d -t AIGAIA -n node1
tmux send-keys -t AIGAIA:node1 'ssh <allocated-dgx-gaia-node>' Enter
sleep 10
tmux capture-pane -t AIGAIA:node1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

确认 `node1` 已在计算节点 prompt 后，再发送：

```bash
tmux send-keys -t AIGAIA:node1 'cd ~/gaia-fs' Enter
```

其中 `<allocated-dgx-gaia-node>` 应替换为 `salloc` 输出中的节点名，例如 `dgx-gaia-14`。本机 SSH 配置已为 `dgx-gaia-*` 设置通过 `gaia` 跳板连接。如需确认当前 allocation，可在登录节点运行 `squeue -u $(whoami)`。

## 工作环境

GAIA 使用个人账号 `haopengh`，默认 shell 为 `bash`。

```text
登录节点: tlv01-e2e-slurm12.lab.nvidia.com
家目录: /labhome/haopengh（NFS，配额 5 GB，已用约 82%）
工作路径: ~/gaia-fs -> /mnt/lustre/gaia/haopengh（Lustre /gaiafs，约 518 TB，仅计算节点挂载）
本地盘: 计算节点 /raid，约 28 TB ext4（md RAID）
平台标识: HUANGHAOPENG_PLATFORM_ID=gaia（在 ~/.bashrc 中设置）
```

家目录配额很小，项目、产物、容器镜像都放到 `~/gaia-fs`，不要放在家目录。

默认 `.bashrc` 只设置少量别名、`ENROOT_CONFIG_PATH=$HOME/.config/enroot`、`ngc-cli` 和 `$HOME/.local/bin`。`module` 命令存在但没有可用模块。计算节点自带软件栈：

- CUDA：`/usr/local/cuda` 指向 CUDA 13.0，另装有 `/usr/local/cuda-12.4`；驱动 580.95.05。
- MPI：系统 MLNX OFED 自带 `/usr/mpi/gcc/openmpi-4.1.9a1`，另有 `ucx_info`。
- 容器：计算节点有 `enroot`，`srun` 支持 Pyxis 的 `--container-image`；登录节点没有 `enroot`。

登录节点没有 `nvidia-smi`、`ibv_devinfo` 等工具，硬件与软件环境以计算节点为准。

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。涉及工作区内容时优先在计算节点确认 `~/gaia-fs` 的真实路径与可用性。

## 作业提交与调度系统

本平台使用 Slurm 22.05.2 管理。只有一个分区 `GAIA`（默认），节点范围为 `dgx-gaia-[09-63]`，共 55 台；分区 `DefaultTime=8:00:00`、`MaxTime=UNLIMITED`、`OverSubscribe=EXCLUSIVE`；`sacctmgr` 查不到关联账号，申请时无需 `-A`。`salloc` window 用于持有 allocation；真正工作应通过 SSH 进入分配到的计算节点执行。分区默认时间是 8 小时，因此必须显式写 `--time`；若没有比较明确的指定时间，默认按 30 分钟申请。

作业名一般使用 `<jobname>.huanghaopeng` 形式，例如 `test.huanghaopeng`。作业无特殊要求，一律使用 `--exclusive`。

常用查询：

```bash
sinfo -o '%P|%a|%l|%D|%N'
sinfo -N -o '%N|%P|%t|%c|%m|%G|%E'
squeue -u $(whoami)
```

资源申请示例：

```bash
salloc -p GAIA -N 1 --exclusive -J test.huanghaopeng --time=0:30:00
```

根据任务需求可调整 GPU 数量、节点数和时间。不要在没有用户确认的情况下长时间占用大量节点。

## 节点硬件

`dgx-gaia-*` 计算节点为 DGX H100，代表节点 `dgx-gaia-38` 实测配置：

```text
OS: Ubuntu 22.04.4 LTS，Linux 5.15.0-1093-nvidia
CPU: Intel Xeon Platinum 8480C（Sapphire Rapids），2 sockets × 56 cores × 2 threads = 224 logical CPUs
NUMA: 2 个 domain，每 socket 一个；GPU0 亲和 NUMA 0（CPU 0-55,112-167），完整映射用 `nvidia-smi topo -m` 查看
内存: 约 2 TiB/node（每 NUMA 约 1 TB），无 swap；Slurm RealMemory 2063900 MB
Cache/core: 48 KiB L1d + 32 KiB L1i + 2 MiB L2；每 socket 105 MiB L3
指令集: AVX-512（含 FP16/BF16/VNNI）、AMX（BF16/INT8）、AVX-VNNI
GPU: 8 × NVIDIA H100 80GB HBM3，全互联 NVLink（NV18，每 link 26.6 GB/s）
GPU 驱动: 580.95.05，CUDA 13.0
网络: 12 个 mlx5 HCA，均为 InfiniBand
  mlx5_0/3/4/5/6/9/10/11: 400 Gb/s NDR，计算网络，每 NUMA 4 张
  mlx5_1/7: 200 Gb/s
  mlx5_2/8: PORT_DOWN
Slurm GRES: gpu:8（少数节点报告 gpu:1）；Features 为 su1–su4
```

登录节点 `tlv01-e2e-slurm12` 是 4 vCPU（AMD EPYC Genoa）、15 GiB 内存的虚拟机，Ubuntu 22.04.4，Linux 5.15.0-174-generic，不能用来代替计算节点数据。

节点状态经常变化，运行前以 `sinfo` 为准，避开 `down`、`drain`、`inval` 等不可用节点。

## 互联网网络

登录节点和计算节点都没有设置 `http_proxy` / `https_proxy`，但实测可以直接访问 `pypi.org` 和 `github.com`，`git clone`、`pip install` 等无需代理。
