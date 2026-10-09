# EOS 平台 Practice

本文档只描述 EOS 平台的操作方式，不包含具体项目规则。EOS 是 NVIDIA 内部 x86_64 Ubuntu + Slurm 平台，登录节点 prompt 类似 `haopengh@login-eos01:~$`，常用工作路径为 `~/eos-fs`。

## 登录与连接

EOS 默认通过本地 tmux 会话进入 `ssh eos`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh eos 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIEOS` 可以保留登录会话、工作路径、调度 allocation 和环境状态，用户也可以随时 attach 查看或协助。

```bash
tmux has-session -t AIEOS 2>/dev/null || tmux new-session -d -s AIEOS -n cmd1
tmux list-windows -t AIEOS | grep -q 'cmd1' || tmux new-window -t AIEOS -n cmd1
tmux capture-pane -t AIEOS:cmd1 -p -J -S -100 | grep -v '^$' | tail -n 20
```

如果还未进入 EOS：

```bash
tmux send-keys -t AIEOS:cmd1 'ssh eos' Enter
```

不要在已经进入 EOS 时重复执行 SSH。若平台长时间无响应，通常说明网络、跳板或登录连接已中断，需要重新连接。

## 命令与脚本

`AIEOS` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1`、`node1` 等命名。轻量命令在 `cmd*` 中运行，资源申请使用专用 window。

```bash
tmux send-keys -t AIEOS:cmd1 '<command>; echo __AIEOS_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AIEOS:cmd1 -p -J -S -200 | grep -v '^$' | tail -n 80
```

如果没有看到 `__AIEOS_DONE_...__`，不要继续向同一 window 发送无关命令。发现命令未结束时，不应自动执行 `sleep 20; capture`；应去掉长 sleep，由用户确认后再直接 capture。短等待一般使用 5 或 10 秒。

## 工作环境

```text
登录节点: login-eos01.eos.clusters.nvidia.com
账号: haopengh
家目录: /home/haopengh
工作路径: ~/eos-fs -> /lustre/fsw/hw_nresearch_snoise/haopengh
Slurm 账号: hw_nresearch_snoise
```

项目和产物优先放在 `~/eos-fs`，不要堆在登录节点家目录根目录。同步前先确认其真实绝对路径。一般使用 `rsync`，不默认使用 `--delete`。

## 作业提交与调度系统

EOS 使用 Slurm 23.02.5。登录节点只用于轻量操作、调度查询、资源申请和文件管理，不运行 benchmark、MPI/OpenMP、训练、推理或持续重负载。

```bash
sinfo -o '%P|%a|%l|%D|%N'
sinfo -p interactive -N -o '%N|%P|%t|%c|%m|%G|%E'
squeue -u "$(whoami)"
sshare -U
```

已探测分区：

1. `batch`：默认，4 小时
2. `backfill`：4 小时
3. `hp`：2 小时
4. `interactive`：默认约 31 分钟，最大 2 小时
5. `hw_vlsi`：7 天
6. `large_runs` / `long_runs`：曾探测为 down

EOS 使用 fairshare。申请资源必须使用账号 `hw_nresearch_snoise`；作业名使用 `hw_nresearch_snoise-<user>.<jobname>`。无特殊要求使用 `--exclusive`。

```bash
tmux list-windows -t AIEOS | grep -q 'salloc1' || tmux new-window -t AIEOS -n salloc1
tmux send-keys -t AIEOS:salloc1 'ssh eos' Enter
tmux send-keys -t AIEOS:salloc1 'salloc --exclusive -A hw_nresearch_snoise -N 1 -J hw_nresearch_snoise-hhp.test -p interactive --time=0:30:00' Enter
```

若无明确时间，默认申请 30 分钟。获得 allocation 后，真正运行使用 `srun` / `mpirun` 或平台确认过的计算节点工作流。

## 容器环境

EOS 默认 shell 为 bash。登录节点通常没有 module、MPI 或 CUDA 环境。EOS 一般通过 Slurm 容器运行，SquashFS 镜像位于 `~/eos-fs/SQSH`。

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

容器只在 allocation 或计算节点中进入，不在登录节点运行重负载。

## 节点硬件

代表节点 `eos0001` 的 Slurm 配置：

```text
CPU: x86_64，2 sockets × 56 cores × 2 threads = 224 logical CPUs/node
内存: 约 2,060,000 MB/node
OS: Ubuntu，内核 5.15 系列
GPU: 8 × H100 80 GB
网络: 400 Gb/s NDR 级
Features: h100, liquid
```

节点状态经常变化，以 `sinfo` 和 `scontrol show node` 为准，避开 `down`、`drain`、`maint`、`resv`。GPU 资源可能不通过 Slurm GRES 暴露，需要在分配到的节点上用 `nvidia-smi` 确认。

## 互联网网络

登录环境默认没有 HTTP proxy。不要假设 EOS 可直接访问互联网；需要外部下载时先做轻量探测，或确认代理、内部镜像与凭据。
