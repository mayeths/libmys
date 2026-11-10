This is user skill reference "clusters/shuguang". Last modified: 2026-11-11.

# 曙光 Shuguang 平台 Practice

本文档只描述曙光平台的操作方式，不包含具体项目规则。这是国家超算互联网核心节点分区一（郑州）的 Sugon OS 8.9 + Slurm 集群，计算节点搭载海光 DCU（`gfx936`），软件栈为 DTK + HIP + RCCL。

## 登录与连接

曙光默认通过本地 tmux 会话进入 `ssh shuguang`。几乎不要使用一次性 `ssh shuguang 'cmd'`，这类方式只用于只读、不加载环境、能马上结束的探测。远程侧只能有一条命令，引号里的命令本身不超过 20 个字符，例如 `date`、`hostname`。通常情况、或者更长命令的情况都放进 tmux。使用本地持久 tmux 会话 `AISG` 保留登录、allocation、工作路径和环境状态。

SSH 账号：

- `shuguang`：正式共享账号 `scnethpc2615`，个人目录 `/work2/share/scnethpc2615/huanghaopeng`
- `shuguangxukai`：其他项目组账号 `scnethpc2667`，仅在明确需要时使用

```bash
tmux has-session -t AISG 2>/dev/null || tmux new-session -d -s AISG -n cmd1
tmux list-windows -t AISG | grep -q 'cmd1' || tmux new-window -t AISG -n cmd1
tmux capture-pane -t AISG:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
tmux send-keys -t AISG:cmd1 'ssh shuguang' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AISG:cmd1 60 5 3 'ssh'
```

不要在已经进入曙光时重复执行 SSH。入口是负载均衡；本机 SSH 配置已通过入口固定进入可访问 `/work2` 的 `zz-login04`。

## SSH 密钥更新

SSH 私钥通常有 30 天有效期。过期后从国家超算互联网控制台的 E-Shell“SSH连接”下载新密钥，覆盖对应本机文件并执行 `chmod 600`。

- 平台：https://www.scnet.cn/ui/mall/
- 控制台：https://www.scnet.cn/ui/console/index.html#/space/dashboard
- E-Shell：https://www.scnet.cn/ui/console/index.html#/space/shell

不要把私钥或凭据写入项目、命令记录或聊天回复。

## 命令与脚本

`AISG` window 按 `cmd1`、`cmd2`、`salloc1`、`node1` 等命名。登录节点严禁直接运行程序，只用于编辑、编译、调度查询、资源申请和文件管理。

### 短时间命令

预计 10 秒内结束的命令追加 `echo "__DONE_$?__"` 以获取退出码，`send-keys`、`sleep 5` 和 `capture-pane` 放在同一次 Shell tool call 中。最后一个 prompt 上方的 DONE 即本次结果；5 秒后没看到，就用 `poll_tmux_done.sh` 接着等，TID 传 `-`（见下方长时间命令）。

```bash
tmux send-keys -t AISG:cmd1 '<command>; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AISG:cmd1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

### 长时间命令

更久的命令追加完成标记 `__TASK_${TID}_DONE_$?__`，标记用 `${TID}` 拼接，不要写死（`TID` 按 `T001`、`T002` 递增），再用 skill 目录下的 `tools/poll_tmux_done.sh` 等待 DONE，多步骤任务可在中间打印 `__TASK_${TID}_STEP<n>_$?__`。快的任务 deadline 60 秒、间隔 5 秒；更长的任务每轮 180 秒、间隔 15 秒。每轮 deadline 和后台等待时长（如 `AwaitShell` 的 `block_until_ms`）都最长 300 秒（5分钟），用一两句话向用户汇报进度（最新 STEP、日志尾部），让用户知道任务仍在正常运行，而不是某个环节已经出错还在空等。接 `| tee` 时用 `${PIPESTATUS[0]}` 代替 `$?`。

`ssh`、`salloc` 等切换 shell 的命令无法追加标记，先发送，再用 `tools/poll_tmux_change.sh` 等待：最后一行不再含关键词（命令走完），且 pane 连续几轮不变（输出刷完）。关键词用命令名，不要用会出现在 prompt 里的节点名。`salloc` 排队时最后一行仍含 `salloc`，脚本会等到新 prompt 出现或到达 deadline。申请资源时 deadline 用 120 秒；超时就再开一轮，并说明还在排队。

```bash
tmux send-keys -t AISG:cmd1 'export TID=T001; bash long_task.sh; echo "__TASK_${TID}_DONE_$?__"' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AISG:cmd1 T001 60 5
bash ~/.claude/skills/clusters/tools/poll_tmux_done.sh AISG:cmd1 - 60 5
#
tmux send-keys -t AISG:salloc1 'salloc ...' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AISG:salloc1 120 5 3 'salloc'
```

工具支持后台等待（如 Cursor 的 `AwaitShell`）时，后台等待`poll_tmux_done.sh`等脚本结束。

## 工作路径与计算节点

```text
/work2/share/scnethpc2615/huanghaopeng
```

`scnethpc2615` 是多人共享账号，个人工作必须放在自己的子目录。`/work2` 是约 9.4 PB ParaStor 并行盘，只在计算节点和 `zz-login04` 挂载。

默认工作流：

1. 在 `salloc1` 申请计算节点并保持 window 不退出。
2. 在 `node1` 先 `ssh shuguang`，再 SSH 到分配节点。
3. 在计算节点进入 `/work2/share/scnethpc2615/huanghaopeng`。

```bash
tmux list-windows -t AISG | grep -q 'salloc1' || tmux new-window -t AISG -n salloc1
tmux send-keys -t AISG:salloc1 'ssh shuguang' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AISG:salloc1 60 5 3 'ssh'
tmux send-keys -t AISG:salloc1 'salloc -p hpctest06 -N 1 --exclusive --gres=dcu:8 --time=0:30:00' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AISG:salloc1 120 5 3 'salloc'
```

分配后，在 `node1` 进入计算节点。等 `ssh` 的 prompt 稳定后，再单独发送 `cd`：

```bash
tmux list-windows -t AISG | grep -q 'node1' || tmux new-window -t AISG -n node1
tmux send-keys -t AISG:node1 'ssh shuguang' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AISG:node1 60 5 3 'ssh'
tmux send-keys -t AISG:node1 'ssh <allocated-node>' Enter
bash ~/.claude/skills/clusters/tools/poll_tmux_change.sh AISG:node1 60 5 3 'ssh'
tmux send-keys -t AISG:node1 'cd /work2/share/scnethpc2615/huanghaopeng; echo "__DONE_$?__"' Enter
sleep 5
tmux capture-pane -t AISG:node1 -p -J -S -30 | grep -v '^$' | tail -n 20
```

计算节点 SSH 受 `pam_slurm_adopt` 限制，必须由当前账号持有对应 allocation。

## 工作环境

- 默认 shell：bash
- 家目录：`/public/home/scnethpc2615`
- 模块系统：environment-modules 4.5.2（Tcl），不是 Lmod
- 登录后默认 module 为空
- 主模块树：`/public/software/modules/base`、`/public/software/sghpc_sdk/modulefiles`

现有环境脚本：

```bash
source /work2/share/scnethpc2615/huanghaopeng/set_env
```

一般使用 `rsync`，不默认使用 `--delete`，并使用绝对路径。项目 `.vscode/sftp.json` 没有目标路径时先询问用户。

## DCU、HIP 与通信

计算节点有 8 张海光 DCU，架构 `gfx936`：

- `hipcc` / `dcc` 对应 `nvcc`
- `hy-smi` 对应 `nvidia-smi`
- `librccl` 对应 `libnccl`
- `rocminfo` 中 GPU agent 显示 `gfx936`

主力 HPC 通信是 OpenMPI + UCX over SHCA，使用 GPU-aware RDMA。平台互连是曙光 SHCA，不是 Mellanox，MPI/UCX 应选择 `shca` 变体。

典型环境：

```bash
module use /work2/share/sghpc_sdk/modulefiles
module load sghpc-mpi-gcc/26.3
export UCX_NET_DEVICES=shca_0:1,shca_1:1,shca_2:1,shca_3:1
export UCX_TLS=self,mm,rc,rocm_copy,rocm_ipc
```

RCCL 主要用于深度学习集合通信；一般 HPC 数值程序使用 MPI。

## 作业提交与调度系统

本平台使用 Slurm。`salloc` 只持有 allocation，真正运行需 SSH 到计算节点或使用 `srun` / `mpirun`。默认申请 30 分钟，作业无特殊要求使用 `--exclusive`。

```bash
sinfo -o '%P|%a|%l|%D|%t|%N'
squeue -u "$(whoami)"
sacctmgr -p show assoc user="$(whoami)" format=account,partition,qos
```

当前已见分区 `hpctest06`，时限 2 小时；实际可用分区和 QOS 以当前账号查询结果为准。

```bash
salloc -p hpctest06 -N 1 --exclusive -J test.huanghaopeng --gres=dcu:8 --time=0:30:00
```

节点状态随时变化，避开 `drain` / `down`，不要未经确认长时间占用大量节点。

## 节点硬件

```text
CPU: Hygon C86 Processor，x86_64，2 sockets × 64 cores = 128 cores/node
线程: 1 thread/core
NUMA: 8 个 domain，每域 16 核
内存: 约 1.0 TiB/node，无 swap
DCU: 8 × 海光 DCU，gfx936
互连: 4 × SHCA 400G
OS: Sugon OS 8.9
```

## PCIe 与网络拓扑

每节点有 8 DCU、4 SHCA，按 8 个 NUMA / PCIe root 分布。SHCA 与部分 DCU 共享同一上游 PCIe switch，支持 GPUDirect RDMA。

```text
DCU NUMA: 0 / 1 / 2 / 3 / 4 / 5 / 6 / 7
HCA NUMA: shca_0 -> 0, shca_1 -> 1, shca_2 -> 4, shca_3 -> 5
```

NUMA 0、1、4、5 上的 DCU 有本地 SHCA；NUMA 2、3、6、7 的 DCU 使用网卡需要跨 root 或 socket。多机运行时进程、DCU、NUMA、NIC 绑定非常重要。

## 互联网网络

曙光默认不能直接访问互联网。登录节点和计算节点均无默认 HTTP proxy。需要联网时可在用户确认后使用本机 SSH 已建立的远程转发代理；否则优先使用平台 module、本地资源或内网镜像。
