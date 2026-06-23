---
name: cluster-eos
description: >-
  EOS 集群操作规范。适用于 ssh host `eos`，以及用户要求在 EOS 上运行、构建、测试、查看内容的场景。
---

This is user skill "cluster-eos".

# EOS 平台 Practice

本文档只描述 EOS 平台的操作方式，不包含具体项目规则。EOS 是 NVIDIA 内部 x86_64 Ubuntu + Slurm 平台，已确认可通过
`ssh eos` 登录，登录节点 prompt 类似 `haopengh@login-eos01:~$`，常用工作路径为 `~/eos-fs`。

## 登录与连接

EOS 默认通过本地 tmux 会话进入 `ssh eos`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用
`ssh eos 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIEOS` 可以保留登录会话、工作路径、调度 allocation 和环境状态，
用户也可以随时 attach 查看或协助。

查看或创建 `AIEOS` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIEOS 2>/dev/null || tmux new-session -d -s AIEOS -n cmd1
tmux list-windows -t AIEOS | grep -q 'cmd1' || tmux new-window -t AIEOS -n cmd1
tmux capture-pane -t AIEOS:cmd1 -p | tail -n 30
```

如果还未进入 EOS 登录节点，在对应 window 中发送：

```bash
tmux send-keys -t AIEOS:cmd1 'ssh eos' Enter
```

不要在已经进入 EOS 时重复执行 `ssh eos`，以免产生嵌套 SSH。若平台长时间无响应，通常说明网络、跳板或登录连接已中断，需要重新连接。

## 命令与脚本

为了支持并行任务，`AIEOS` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1`、`node1` 等命名。通常情况下，轻量命令在
`cmd1` / `cmd2` 中运行；资源申请或长时间占用的交互式作业使用专用 window。

运行命令时，应在命令后加上完成标记，方便判断命令是否结束以及退出码：

```bash
tmux send-keys -t AIEOS:cmd1 '<command>; echo __AIEOS_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AIEOS:cmd1 -p -S -120 | tail -n 80
```

如果没有看到 `__AIEOS_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。

## 工作环境

EOS 使用个人账号 `haopengh`，登录节点已确认包括：

```text
登录节点:   login-eos01.eos.clusters.nvidia.com
账号:       haopengh
家目录:     /home/haopengh
工作路径:   ~/eos-fs -> /lustre/fsw/hw_nresearch_snoise/haopengh
Slurm 账号: hw_nresearch_snoise
```

`~/eos-fs` 在当前登录记录中可直接访问，目录下已见 `DATA`、`SOURCE`、`SQSH`、`HPCX`、`MODEL`、`workspace`、`project` 等目录。
进入 EOS 后，优先在 `~/eos-fs` 下进行项目查看、构建准备和轻量文件操作；不要把项目产物堆在登录节点家目录根目录。

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应先在 EOS 上用
`pwd` 确认 `~/eos-fs` 的真实绝对路径，再使用绝对路径同步。

## 作业提交与调度系统

本平台使用 Slurm 23.02.5 管理，计算节点的数量非常多。登录节点 `login-eos01` 只用于轻量操作、查询调度系统、申请资源和管理文件。不要在登录节点运行
benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

常用查询：

```bash
sinfo -o '%P|%a|%l|%D|%N'
sinfo -p interactive -o '%P|%a|%l|%D|%t|%c|%m|%G'
sinfo -p interactive -N -o '%N|%P|%t|%c|%m|%G|%E'
squeue -u $(whoami)
sshare -U
```

可用分区以 `sinfo` 为准，已探测到：

1. `batch`：默认分区，4 小时时限
2. `backfill`：4 小时时限
3. `hp`：2 小时时限
4. `interactive`：交互分区，默认约 31 分钟，最大 2 小时
5. `hw_vlsi`：4 个节点，7 天时限
6. `large_runs` / `long_runs`：当前探测为 down

EOS 使用 fairshare 管理作业优先级，提交作业前可用 `sshare -U` 查看当前用户的 fairshare 信息。申请 Slurm 资源时必须使用账号
`hw_nresearch_snoise`；作业名按平台要求使用 `hw_nresearch_snoise-<user>.<jobname>` 形式，例如 `hw_nresearch_snoise-hhp.test`。
作业无特殊要求，一律使用`--exclusive`。

交互式申请命令模板：

```bash
tmux list-windows -t AIEOS | grep -q 'salloc1' || tmux new-window -t AIEOS -n salloc1
tmux send-keys -t AIEOS:salloc1 'ssh eos' Enter
tmux capture-pane -t AIEOS:salloc1 -p | tail -n 30
```

确认 `salloc1` 已在 EOS 登录节点 prompt 后，再发送：

```bash
tmux send-keys -t AIEOS:salloc1 \
  'salloc --exclusive -A hw_nresearch_snoise -N 1 -J hw_nresearch_snoise-hhp.test -p interactive --time=0:30:00' Enter
```

若没有比较明确的指定时间，默认按 30 分钟申请，且不要在没有用户确认的情况下长时间占用大量节点。若 `salloc` 返回的 shell 仍在登录节点，
真正运行应使用 `srun` / `mpirun` 或按平台要求 SSH 到分配节点；具体以 EOS 实测行为为准。

## 工作环境

EOS 默认 shell 为 `/bin/bash`。登录节点当前没有 `module` 命令，默认 `PATH` 中有 Slurm 命令和 `/usr/bin/python3`，但未见 `mpirun`、
`mpicc`、`nvcc`。`squeue` 在登录环境中是 `squeue -u "$USER"` 的 alias。

EOS 一般使用 Slurm 容器工作，而不是直接依赖登录节点环境。常用 SquashFS 镜像放在 `~/eos-fs/SQSH`，已见
`dsv3-25.12.23.sqsh`、`nemo-25.07.sqsh`、`nemo-25.09.sqsh`、`nemo-25.11.sqsh`、`sglang-25.10.sqsh`、
`sglang-25.11.sqsh` 等。进入容器应在已获得的 Slurm allocation 或分配到的计算节点上通过 `srun` 完成，不要在登录节点直接跑重负载。

单节点交互式容器模板：

```bash
srun --mpi=pmix \
  --nodes=1 \
  -w $(hostname -s) \
  --ntasks-per-node=8 \
  --container-image=/lustre/fsw/hw_nresearch_snoise/haopengh/SQSH/dsv3-25.12.23.sqsh \
  --container-mounts=/lustre/fsw/hw_nresearch_snoise/haopengh:/mounted_ws \
  --container-workdir=/mounted_ws \
  --container-writable \
  --no-container-mount-home \
  --pty bash
```

如果镜像旁边有对应的 `.mod/root` 目录且需要复用其中的 root 环境，可在 `--container-mounts` 里追加
`/lustre/fsw/hw_nresearch_snoise/haopengh/SQSH/<image>.mod/root:/root`。进入容器后再按项目需要激活环境，
例如 `source /root/.venv/bin/activate`。

一般使用`rsync`传输文件（除非明确指示，否则不使用`--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。

## 节点硬件

代表节点 `eos0001` 的 Slurm 配置大致如下：

```text
CPU: x86_64, 2 sockets × 56 cores/socket × 2 threads/core = 224 logical CPUs/node
内存: 约 2,060,000 MB/node
OS: Ubuntu 内核 5.15.0-88-generic
Features: h100, liquid
Slurm GRES: (null)
```

`interactive` 分区节点范围为 `eos[0001-0096,0113-0574,0576]`，节点状态经常变化。运行前以 `sinfo` 和 `scontrol show node` 为准，
避开 `down`、`drain`、`maint`、`resv` 等不可用或不可直接使用节点。GPU 数量未通过 Slurm GRES 暴露；需要在分配到的计算节点上用
`nvidia-smi` 实测确认。

## 互联网网络

当前登录环境没有检测到 `http_proxy` / `https_proxy`。不要假设 EOS 可直接访问互联网；需要 `git clone`、`pip install`、容器拉取或
外部下载时，先做轻量网络探测或向用户确认可用代理、镜像和凭据。
