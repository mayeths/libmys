This is user skill reference "clusters/qh". Last modified: 2026-11-11.

# QH 集群 Practice

本文档只描述 QH 集群的平台操作方式，不包含具体项目规则。QH 是 Rocky Linux 8.9（x86_64）+ Slurm 的异构集群，含 Intel Xeon Max（HBM）与 AMD EPYC 两类节点。

## 登录与连接

QH 集群默认通过本地 tmux 会话进入 `ssh qh`。几乎不要使用一次性 `ssh qh 'cmd'`，这类方式只用于只读、不加载环境、能马上结束的探测。远程侧只能有一条命令，引号里的命令本身不超过 20 个字符，例如 `date`、`hostname`。通常情况、或者更长命令的情况都放进 tmux。使用本地持久 tmux 会话 `AIQH` 可以保留工作路径、编译器环境、MPI 环境和 Python 环境等状态，用户也可以随时 attach 查看或协助。

查看或创建 `AIQH` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIQH 2>/dev/null || tmux new-session -d -s AIQH -n cmd1
tmux list-windows -t AIQH | grep -q 'cmd1' || tmux new-window -t AIQH -n cmd1
tmux capture-pane -t AIQH:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

若集群长时间无响应，通常说明网络或登录连接已中断，需要重新 `ssh qh`。

## 命令与脚本

为了支持并行任务，`AIQH` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1` 等命名。通常情况下，普通命令在默认的 `cmd1` 中运行。

如果还未进入 qh 集群，在对应 window 中发送，再用 `poll_tmux_change.sh` 等待：

```bash
tmux send-keys -t AIQH:cmd1 'ssh qh' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIQH:cmd1 60 5 3 'ssh'
```

不要在已经进入 qh 时重复执行 `ssh qh`，以免产生嵌套 SSH。

### 短时间命令

预计 10 秒内结束的命令追加 `echo "__DONE_$?__"` 以获取退出码，`send-keys`、`sleep 5` 和 `capture-pane` 放在同一次 Shell tool call 中。最后一个 prompt 上方的 DONE 即本次结果；5 秒后没看到，就用 `poll_tmux_done.sh` 接着等，TID 传 `-`（见下方长时间命令）。

```bash
tmux send-keys -t AIQH:cmd1 '<command>; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AIQH:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

### 长时间命令

更久的命令追加完成标记 `__TASK_${TID}_DONE_$?__`，标记用 `${TID}` 拼接，不要写死（`TID` 按 `T001`、`T002` 递增），再用 skill 目录下的 `tools/poll_tmux_done.sh` 等待 DONE，多步骤任务可在中间打印 `__TASK_${TID}_STEP<n>_$?__`。快的任务 deadline 60 秒、间隔 5 秒；更长的任务每轮 180 秒、间隔 15 秒。每轮 deadline 和后台等待时长（如 `AwaitShell` 的 `block_until_ms`）都最长 300 秒（5分钟），用一两句话向用户汇报进度（最新 STEP、日志尾部），让用户知道任务仍在正常运行，而不是某个环节已经出错还在空等。接 `| tee` 时用 `${PIPESTATUS[0]}` 代替 `$?`。

`ssh`、`salloc` 等切换 shell 的命令无法追加标记，先发送，再用 `tools/poll_tmux_change.sh` 等待：最后一行不再含关键词（命令走完），且 pane 连续几轮不变（输出刷完）。关键词用命令名，不要用会出现在 prompt 里的节点名。`salloc` 排队时最后一行仍含 `salloc`，脚本会等到新 prompt 出现或到达 deadline。申请资源时 deadline 用 120 秒；超时就再开一轮，并说明还在排队。

```bash
tmux send-keys -t AIQH:cmd1 'export TID=T001; bash long_task.sh; echo "__TASK_${TID}_DONE_$?__"' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIQH:cmd1 T001 60 5
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AIQH:cmd1 - 60 5
#
tmux send-keys -t AIQH:salloc1 'salloc ...' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIQH:salloc1 120 5 3 'salloc'
```

工具支持后台等待（如 Cursor 的 `AwaitShell`）时，后台等待`poll_tmux_done.sh`等脚本结束。

## 工作环境

QH 使用共享账号 `xuewei`（组 `xuewei_group`）。始终使用登录默认的 `bash`。常用目录：

```text
个人目录: /online1/xuewei_group/xuewei/huanghp
并行盘: /online1
账号家目录: /home/xuewei_group/xuewei
软件栈: /apps
```

工作和算例一律放在个人目录 `/online1/xuewei_group/xuewei/huanghp`（`~/data/huanghp`），与共享账号其他用户隔离。

进入 qh 后，加载环境统一 source 个人 `set_env`（必须 source，不能直接执行，因为它会重设 `HOME`）。`source set_env` 会返回，按普通命令追加 `__DONE_$?__`：

```bash
source /online1/xuewei_group/xuewei/huanghp/set_env
```

`set_env` 会把 `HOME` 切到个人目录并 `cd` 过去、source 个人 libmys 环境、清理 module 后加载个人默认软件栈。如确需其他软件，再用 `module load` 追加。

个人环境中的平台标识为 `HUANGHAOPENG_PLATFORM_ID=qh`。

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。

## 作业提交与调度系统

本集群使用 Slurm 23.02 管理。登录节点 `login01` 只用于轻量操作，不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需要使用 `srun` 或 `mpirun`。需要 `salloc` 时，应使用 tmux 中的专用 window，按 `salloc1`、`salloc2` 等命名；普通运行命令仍在 `cmd1`、`cmd2` 等 window 中执行。`srun` 和 `mpirun` 按普通命令追加 DONE 或 TASK 标记。若没有明确指定时间，默认按 30 分钟申请。

作业名一般使用 `<jobname>.huanghaopeng`，无特殊要求一律使用 `--exclusive`。

可用 Slurm 分区：

1. `intel`：默认分区，Intel Xeon Max 节点（`qhcn###`），数百节点，无时限
2. `amd`：AMD EPYC 大内存节点（`qhdn###`），3 节点，无时限
3. `intel_expr`：Intel 调试/试验分区，4 节点，时限 2 小时

```bash
tmux list-windows -t AIQH | grep -q 'salloc1' || tmux new-window -t AIQH -n salloc1
tmux send-keys -t AIQH:salloc1 'ssh qh' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIQH:salloc1 60 5 3 'ssh'
tmux send-keys -t AIQH:salloc1 'salloc -p intel -N <nodes> --exclusive -J test.huanghaopeng --time=0:30:00' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AIQH:salloc1 120 5 3 'salloc'
```

## 节点硬件

Intel 节点 `qhcn###`：

```text
OS: Rocky Linux 8.9，Linux 4.18
CPU: Intel Xeon CPU Max 9462, 2 sockets × 32 cores = 64 cores/node
线程: 1 thread/core
NUMA: 8 个 domain，每域 8 核
内存: 约 503 GiB/node（RealMemory 514000 MB），无 swap，含片上 HBM
Cache/core: 48 KiB L1d + 32 KiB L1i + 2 MiB L2；lscpu 报告 75 MiB L3
指令集: AVX-512、AVX512-FP16/BF16、AMX、AVX-VNNI
网络: 1 × Mellanox ConnectX-6，100 Gb/s InfiniBand
```

AMD 节点 `qhdn###`：

```text
OS: Rocky Linux 8.9，Linux 4.18
CPU: AMD EPYC 9654，2 sockets × 96 cores = 192 cores/node
线程: 1 thread/core
NUMA: 8 个 domain，每域 24 核
内存: 约 1.5 TiB/node（RealMemory 1546000 MB），无 swap
Cache/core: 32 KiB L1d + 32 KiB L1i + 1 MiB L2；lscpu 报告 32 MiB L3
指令集: AVX-512、AVX512-BF16、AVX-VNNI
```

`/online1` 为约 17 PiB Lustre over InfiniBand。

## 互联网网络

QH 默认不能直接访问互联网。不要假设 `git clone` / `pip install` / `conda` 能联网，应优先使用已有 module 和本地资源；确需联网时先与用户确认代理或内网镜像。
