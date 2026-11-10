This is user skill reference "clusters/eos". Last modified: 2026-11-11.

# EOS 平台 Practice

本文档只描述 EOS 平台的操作方式，不包含具体项目规则。EOS 是 NVIDIA 内部 x86_64 Ubuntu + Slurm 平台，登录节点 prompt 类似 `haopengh@login-eos01:~$`，常用工作路径为 `~/eos-fs`。

## 登录与连接

EOS 默认通过本地 tmux 会话进入 `ssh eos`。几乎不要使用一次性 `ssh eos 'cmd'`，这类方式只用于只读、不加载环境、能马上结束的探测。远程侧只能有一条命令，引号里的命令本身不超过 20 个字符，例如 `date`、`hostname`。通常情况、或者更长命令的情况都放进 tmux。使用本地持久 tmux 会话 `AIEOS` 可以保留登录会话、工作路径、调度 allocation 和环境状态，用户也可以随时 attach 查看或协助。

```bash
tmux has-session -t AIEOS 2>/dev/null || tmux new-session -d -s AIEOS -n cmd1
tmux list-windows -t AIEOS | grep -q 'cmd1' || tmux new-window -t AIEOS -n cmd1
tmux capture-pane -t AIEOS:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

如果还未进入 EOS：

```bash
tmux send-keys -t AIEOS:cmd1 'ssh eos' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIEOS:cmd1 60 5 3 'ssh'
```

不要在已经进入 EOS 时重复执行 SSH。若平台长时间无响应，通常说明网络、跳板或登录连接已中断，需要重新连接。

## 命令与脚本

`AIEOS` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1`、`node1` 等命名。轻量命令在 `cmd*` 中运行，资源申请使用专用 window。

### 短时间命令

预计 10 秒内结束的命令追加 `echo "__DONE_$?__"` 以获取退出码，`send-keys`、`sleep 5` 和 `capture-pane` 放在同一次 Shell tool call 中。最后一个 prompt 上方的 DONE 即本次结果；5 秒后没看到，就用 `poll_tmux_done.sh` 接着等，TID 传 `-`（见下方长时间命令）。

```bash
tmux send-keys -t AIEOS:cmd1 '<command>; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AIEOS:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

### 长时间命令

更久的命令追加完成标记 `__TASK_${TID}_DONE_$?__`，标记用 `${TID}` 拼接，不要写死（`TID` 按 `T001`、`T002` 递增），再用 skill 目录下的 `tools/poll_tmux_done.sh` 等待 DONE，多步骤任务可在中间打印 `__TASK_${TID}_STEP<n>_$?__`。快的任务 deadline 60 秒、间隔 5 秒；更长的任务每轮 180 秒、间隔 15 秒。每轮 deadline 和后台等待时长（如 `AwaitShell` 的 `block_until_ms`）都最长 300 秒（5分钟），用一两句话向用户汇报进度（最新 STEP、日志尾部），让用户知道任务仍在正常运行，而不是某个环节已经出错还在空等。接 `| tee` 时用 `${PIPESTATUS[0]}` 代替 `$?`。

`ssh`、`salloc`、`srun --pty` 等切换 shell 的命令无法追加标记，先发送，再用 `tools/poll_tmux_change.sh` 等待：最后一行不再含关键词（命令走完），且 pane 连续几轮不变（输出刷完）。关键词用命令名，不要用会出现在 prompt 里的节点名。`salloc` 排队时最后一行仍含 `salloc`，脚本会等到新 prompt 出现或到达 deadline。申请资源时 deadline 用 120 秒；超时就再开一轮，并说明还在排队。

```bash
tmux send-keys -t AIEOS:cmd1 'export TID=T001; bash long_task.sh; echo "__TASK_${TID}_DONE_$?__"' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIEOS:cmd1 T001 60 5
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIEOS:cmd1 - 60 5
#
tmux send-keys -t AIEOS:salloc1 'salloc ...' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIEOS:salloc1 120 5 3 'salloc'
```

工具支持后台等待（如 Cursor 的 `AwaitShell`）时，后台等待`poll_tmux_done.sh`等脚本结束。

## 工作环境

```text
登录节点: login-eos01.eos.clusters.nvidia.com
账号: haopengh
家目录: /home/haopengh（NFS，Isilon）
工作路径: ~/eos-fs -> /lustre/fsw/hw_nresearch_snoise/haopengh（Lustre /lfs3/fsw，约 9.2 PB，登录与计算节点均挂载）
其他共享盘: /lustre/fsr、/lustre/share（Lustre），/project（NFS）
本地盘: 计算节点 /local 约 1.8 TB，/raid 约 6 TB，/raid/scratch 约 6 TB，/raid/pcc 约 15 TB
Slurm 账号: hw_nresearch_snoise
平台标识: HUANGHAOPENG_PLATFORM_ID=eos（在 ~/.bashrc 中设置）
```

项目和产物优先放在 `~/eos-fs`，不要堆在登录节点家目录根目录。同步前先确认其真实绝对路径。一般使用 `rsync`，不默认使用 `--delete`。

EOS 登录和计算节点都没有 `module` 命令，计算节点宿主机上也没有 `nvcc`、`mpirun`，CUDA、MPI、NCCL 等工具链一律通过容器获得。

## 作业提交与调度系统

EOS 使用 Slurm 23.02.5。登录节点只用于轻量操作、调度查询、资源申请和文件管理，不运行 benchmark、MPI/OpenMP、训练、推理或持续重负载。

```bash
sinfo -o '%P|%a|%l|%D|%N'
sinfo -p interactive -N -o '%N|%P|%t|%c|%m|%G|%E'
squeue -u "$(whoami)"
sshare -U
```

已探测分区（节点为 `eos[0001-0576]` 中的约 556 台，`hw_vlsi` 仅 4 台）：

| 分区 | 状态 | 默认时间 | 最大时间 | 说明 |
| --- | --- | --- | --- | --- |
| `batch` | up | 2 小时 | 4 小时 | 默认分区 |
| `backfill` | up | 2 小时 | 4 小时 | |
| `hp` | up | 1 小时 | 2 小时 | |
| `interactive` | up | 31 分钟 | 2 小时 | 交互调试首选 |
| `hw_vlsi` | up | 31 分钟 | 7 天 | `eos[0570-0571,0574-0575]` |
| `large_runs` | down | 31 分钟 | 2 小时 | 最少 256 节点 |
| `long_runs` | down | 31 分钟 | 1 天 | |

EOS 使用 fairshare。申请资源必须使用账号 `hw_nresearch_snoise`；作业名使用 `hw_nresearch_snoise-<user>.<jobname>`。无特殊要求使用 `--exclusive`。

```bash
tmux list-windows -t AIEOS | grep -q 'salloc1' || tmux new-window -t AIEOS -n salloc1
tmux send-keys -t AIEOS:salloc1 'ssh eos' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIEOS:salloc1 60 5 3 'ssh'
tmux send-keys -t AIEOS:salloc1 'salloc --exclusive -A hw_nresearch_snoise -N 1 -J hw_nresearch_snoise-hhp.test -p interactive --time=0:30:00' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIEOS:salloc1 120 5 3 'salloc'
```

若无明确时间，默认申请 30 分钟。与 YC、KP 不同，EOS 的 `salloc` 拿到 allocation 后会直接把 shell 放到分配的第一个计算节点上（prompt 变为 `haopengh@eos0065:~$` 这类形式），并会提示 `setlocale: LC_ALL` 警告，可忽略。单节点任务可直接在这个 shell 中执行；多节点任务和容器任务仍通过 `srun` 启动。退出该 shell 即释放 allocation。

## 容器环境

EOS 默认 shell 为 bash。EOS 通过 Slurm + Pyxis/enroot 运行容器（登录与计算节点都有 `enroot`），SquashFS 镜像位于 `~/eos-fs/SQSH`，已有 `nemo-25.07/09/11.sqsh`、`sglang-25.10.sqsh`、`dsv3-25.12.23.sqsh` 等。

```bash
srun --mpi=pmix \
  --nodes=1 \
  -w "$(hostname -s)" \
  --ntasks-per-node=8 \
  --container-image=/lustre/fsw/hw_nresearch_snoise/haopengh/SQSH/<image>.sqsh \
  --container-mounts=/lustre/fsw/hw_nresearch_snoise/haopengh:/mounted_ws \
  --container-workdir=/mounted_ws \
  --container-writable \
  --no-container-mount-home \
  --pty bash
```

用 `srun --pty` 进入容器时，先发送上述命令，再用 `poll_tmux_change.sh` 等待，关键词用 `srun`。容器只在 allocation 或计算节点中进入，不在登录节点运行重负载。

## 节点硬件

计算节点为 DGX H100（液冷），代表节点 `eos0065` 实测配置：

```text
OS: Ubuntu 22.04.3 LTS，Linux 5.15.0-88-generic
CPU: Intel Xeon Platinum 8480C（Sapphire Rapids），2 sockets × 56 cores × 2 threads = 224 logical CPUs
NUMA: 2 个 domain，每 socket 一个；GPU0 亲和 NUMA 0（CPU 0-55,112-167），完整映射用 `nvidia-smi topo -m` 查看
内存: 约 2 TiB/node（每 NUMA 约 1 TB），无 swap；Slurm RealMemory 2060000 MB
Cache/core: 48 KiB L1d + 32 KiB L1i + 2 MiB L2；每 socket 105 MiB L3
指令集: AVX-512（含 FP16/BF16/VNNI）、AMX（BF16/INT8）、AVX-VNNI
GPU: 8 × NVIDIA H100 80GB HBM3，全互联 NVLink（NV18，每 link 26.6 GB/s）
GPU 驱动: 535.129.03，CUDA 12.2（宿主机驱动版本限制容器内 CUDA 版本）
网络: mlx5_0/1/3/4/5/6/7/9/10/11 共 10 张 400 Gb/s NDR InfiniBand，每 NUMA 5 张；mlx5_2/8 为 Ethernet，PORT_DOWN
Slurm GRES: 无（GPU 不通过 GRES 暴露，独占节点即获得 8 张 GPU）
Features: h100, liquid, viking, podXX, ibcleafXX-XX
```

登录节点 `login-eos01` 是 2 × Intel Xeon Gold 6248（40 cores × 2 threads = 80 logical CPUs）、376 GiB 内存，无 GPU，不能用来代替计算节点数据。

节点状态经常变化，以 `sinfo` 和 `scontrol show node` 为准，避开 `down`、`drain`、`maint`、`resv`。

## 互联网网络

登录节点和计算节点都没有设置 HTTP proxy，但实测可以直接访问 `pypi.org` 和 `github.com`，外部下载无需代理。
