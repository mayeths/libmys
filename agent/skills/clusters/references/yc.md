This is user skill reference "clusters/yc". Last modified: 2026-11-11.

# YC 集群 Practice

本文档只描述 YC 集群的平台操作方式，不包含具体项目规则。

## 登录与连接

YC 集群默认通过本地 tmux 会话进入 `ssh yc`。几乎不要使用一次性 `ssh yc 'cmd'`，这类方式只用于只读、不加载环境、能马上结束的探测。远程侧只能有一条命令，引号里的命令本身不超过 20 个字符，例如 `date`、`hostname`。通常情况、或者更长命令的情况都放进 tmux。使用本地持久 tmux 会话 `AIYC` 可以保留工作路径、编译器环境、MPI 环境、CUDA 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIYC` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIYC 2>/dev/null || tmux new-session -d -s AIYC -n cmd1
tmux list-windows -t AIYC | grep -q 'cmd1' || tmux new-window -t AIYC -n cmd1
tmux capture-pane -t AIYC:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

集群 VPN 与登录会话约 30 分钟空闲后可能断开。若集群长时间无响应，通常说明网络或登录连接已中断，需要重新 `ssh yc`。当前 SSH 已使用 `~/.ssh/control-XXX` 做连接持久化。

## 命令与脚本

为了支持并行任务，`AIYC` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1` 等命名。通常情况下，普通命令在默认的 `cmd1` 中运行。

如果还未进入 yc 集群，在对应 window 中发送，再用 `poll_tmux_change.sh` 等待：

```bash
tmux send-keys -t AIYC:cmd1 'ssh yc' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIYC:cmd1 60 5 3 'ssh'
```

不要在已经进入 yc 时重复执行 `ssh yc`，以免产生嵌套 SSH。

### 短时间命令

预计 10 秒内结束的命令追加 `echo "__DONE_$?__"` 以获取退出码，`send-keys`、`sleep 5` 和 `capture-pane` 放在同一次 Shell tool call 中。最后一个 prompt 上方的 DONE 即本次结果；5 秒后没看到，就用 `poll_tmux_done.sh` 接着等，TID 传 `-`（见下方长时间命令）。

```bash
tmux send-keys -t AIYC:cmd1 '<command>; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AIYC:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

### 长时间命令

更久的命令追加完成标记 `__TASK_${TID}_DONE_$?__`，标记用 `${TID}` 拼接，不要写死（`TID` 按 `T001`、`T002` 递增），再用 skill 目录下的 `tools/poll_tmux_done.sh` 等待 DONE，多步骤任务可在中间打印 `__TASK_${TID}_STEP<n>_$?__`。快的任务 deadline 60 秒、间隔 5 秒；更长的任务每轮 180 秒、间隔 15 秒。每轮 deadline 和后台等待时长（如 `AwaitShell` 的 `block_until_ms`）都最长 300 秒（5分钟），用一两句话向用户汇报进度（最新 STEP、日志尾部），让用户知道任务仍在正常运行，而不是某个环节已经出错还在空等。接 `| tee` 时用 `${PIPESTATUS[0]}` 代替 `$?`。

`ssh`、`salloc`、`huanghp-zsh` 等切换 shell 的命令无法追加标记，先发送，再用 `tools/poll_tmux_change.sh` 等待：最后一行不再含关键词（命令走完），且 pane 连续几轮不变（输出刷完）。关键词用命令名，不要用会出现在 prompt 里的节点名。`salloc` 排队时最后一行仍含 `salloc`，脚本会等到新 prompt 出现或到达 deadline。申请资源时 deadline 用 120 秒；超时就再开一轮，并说明还在排队。

```bash
tmux send-keys -t AIYC:cmd1 'export TID=T001; bash long_task.sh; echo "__TASK_${TID}_DONE_$?__"' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIYC:cmd1 T001 60 5
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIYC:cmd1 - 60 5
#
tmux send-keys -t AIYC:salloc1 'salloc ...' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIYC:salloc1 120 5 3 'salloc'
```

工具支持后台等待（如 Cursor 的 `AwaitShell`）时，后台等待`poll_tmux_done.sh`等脚本结束。

## 工作环境

共享账号为 `xuew`，个人环境家目录是：

```text
/online1/ycsc_xuew/xuew/huanghp-home
```

进入盐城集群后，应运行 `huanghp-zsh` 启动个人 zsh，以使用个人编译环境、Python 环境等配置。`huanghp-zsh` 会切换 shell，发送后用 `poll_tmux_change.sh` 等待，关键词用 `huanghp-zsh`。预期 prompt 类似：

```text
xuew@psn002 ~ ❯
```

个人环境中的平台标识为 `HUANGHAOPENG_PLATFORM_ID=yc`。

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。

## 作业提交与调度系统

本集群使用 Slurm 管理。登录节点 `psn002` 等只用于轻量操作，不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。MPI 程序通常先在 `salloc` window 中申请资源，再从运行命令的 window 使用 `mpirun` 启动。`srun` 和 `mpirun` 按普通命令追加 DONE 或 TASK 标记。若没有比较明确的指定 salloc 时间，默认按 30 分钟申请。

作业名一般使用 `<jobname>.huanghaopeng` 形式，例如 `test.huanghaopeng`。作业无特殊要求，一律使用 `--exclusive`。

常用 Slurm 分区：

1. `q_amd_share`：默认 AMD EPYC 分区，bn 节点
2. `q_intel_share`：Intel 分区，dn 节点

示例：

```bash
tmux list-windows -t AIYC | grep -q 'salloc1' || tmux new-window -t AIYC -n salloc1
tmux send-keys -t AIYC:salloc1 'ssh yc' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIYC:salloc1 60 5 3 'ssh'
tmux send-keys -t AIYC:salloc1 'salloc -p q_amd_share -N <nodes> --exclusive -J test.huanghaopeng --time=0:30:00' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIYC:salloc1 120 5 3 'salloc'
```

## 节点硬件

`q_amd_share` / `bn` 节点实测配置：

```text
OS: CentOS 7，Linux 3.10
CPU: AMD EPYC 7H12, 2 sockets × 64 cores = 128 cores/node
线程: 1 thread/core
NUMA: 2 个 NUMA domain，0-63 与 64-127
内存: 约 188 GiB/node，无 swap
Cache/core: 32 KiB L1d + 32 KiB L1i + 512 KiB L2；lscpu 报告 16 MiB L3
网络: 1 × Mellanox ConnectX-5，100 Gb/s InfiniBand
指令集: AVX2，无 AVX-512
```

`q_intel_share` / `dn` 节点实测配置：

```text
OS: CentOS 7，Linux 3.10
CPU: Intel Xeon Platinum 8358，2 sockets × 32 cores = 64 cores/node
线程: 1 thread/core
NUMA: 2 个 domain，每域 32 核
内存: 约 251 GiB/node，无 swap
Cache/core: 48 KiB L1d + 32 KiB L1i + 1.25 MiB L2；lscpu 报告 48 MiB L3
网络: 1 × Mellanox ConnectX-5，100 Gb/s InfiniBand
指令集: AVX-512、AVX-VNNI
```

共享文件系统 `/online1` 为约 7 PiB Lustre over InfiniBand。

## 互联网网络

YC 访问互联网需要 HTTP proxy，默认 `.bashrc` 已设置：

```bash
export http_proxy=http://174.0.250.13:3128
export https_proxy=http://174.0.250.13:3128
```
