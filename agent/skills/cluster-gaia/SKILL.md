---
name: cluster-gaia
description: >-
  GAIA Slurm GPU 平台操作规范。适用于 ssh host `gaia`、`dgx-gaia-*`，以及用户要求在 GAIA 上运行、构建、测试、查看内容的场景。
---

This is user skill "cluster-gaia".

# GAIA 平台 Practice

本文档只描述 GAIA 平台的操作方式，不包含具体项目规则。GAIA 是 NVIDIA 内部 Ubuntu 22.04 + Slurm GPU 平台，登录节点为
`tlv01-e2e-slurm12`，计算节点为 `dgx-gaia-*`。

## 登录与连接

GAIA 默认通过本地 tmux 会话进入 `ssh gaia`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用
`ssh gaia 'cmd'` 这类一次性命令。使用本地持久 tmux 会话 `AIGAIA` 可以保留登录会话、Slurm allocation、计算节点工作路径和环境状态，
用户也可以随时 attach 查看或协助。

查看或创建 `AIGAIA` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AIGAIA 2>/dev/null || tmux new-session -d -s AIGAIA -n cmd1
tmux list-windows -t AIGAIA | grep -q 'cmd1' || tmux new-window -t AIGAIA -n cmd1
tmux capture-pane -t AIGAIA:cmd1 -p | tail -n 30
```

如果还未进入 GAIA 登录节点，在对应 window 中发送：

```bash
tmux send-keys -t AIGAIA:cmd1 'ssh gaia' Enter
```

不要在已经进入 GAIA 时重复执行 `ssh gaia`，以免产生嵌套 SSH。若平台长时间无响应，通常说明网络、跳板或登录连接已中断，需要重新连接。
本机 SSH 配置中 `gaia` 通过 `hpclogin` 跳板进入，`dgx-gaia-*` 可通过 `gaia` 作为 ProxyJump。

## 命令与脚本

为了支持并行任务，`AIGAIA` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1`、`node1` 等命名。

登录节点 `tlv01-e2e-slurm12` 只用于轻量操作、查询 Slurm、申请资源和管理文件。不要在登录节点运行 benchmark、MPI/OpenMP、训练、推理或
长时间重负载任务。

运行命令时，应在命令后加上完成标记，方便判断命令是否结束以及退出码：

```bash
tmux send-keys -t AIGAIA:cmd1 '<command>; echo __AIGAIA_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AIGAIA:cmd1 -p -S -120 | tail -n 80
```

如果没有看到 `__AIGAIA_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。

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
tmux list-windows -t AIGAIA | grep -q 'salloc1' || tmux new-window -t AIGAIA -n salloc1
tmux send-keys -t AIGAIA:salloc1 'ssh gaia' Enter
tmux capture-pane -t AIGAIA:salloc1 -p | tail -n 30
```

确认 `salloc1` 已在 GAIA 登录节点 prompt 后，再发送：

```bash
tmux send-keys -t AIGAIA:salloc1 'salloc -p GAIA -N 1 --gres=gpu:8 --time=0:30:00' Enter
```

看到 `salloc` 分配到节点后，在单独 window 进入该计算节点：

```bash
tmux list-windows -t AIGAIA | grep -q 'node1' || tmux new-window -t AIGAIA -n node1
tmux send-keys -t AIGAIA:node1 'ssh <allocated-dgx-gaia-node>' Enter
tmux capture-pane -t AIGAIA:node1 -p | tail -n 30
```

确认 `node1` 已在计算节点 prompt 后，再发送：

```bash
tmux send-keys -t AIGAIA:node1 'cd ~/gaia-fs; echo __AIGAIA_NODE_READY__' Enter
```

其中 `<allocated-dgx-gaia-node>` 应替换为 `salloc` 输出中的节点名，例如 `dgx-gaia-14`。本机 SSH 配置已为 `dgx-gaia-*` 设置通过 `gaia`
跳板连接。如需确认当前 allocation，可在登录节点运行 `squeue -u $(whoami)`。

## 工作环境

GAIA 使用个人账号 `haopengh`，默认 shell 为 `bash`，家目录为：

```text
/labhome/haopengh
```

默认 `.bashrc` 只设置少量别名、`ENROOT_CONFIG_PATH=$HOME/.config/enroot`、`ngc-cli` 和 `$HOME/.local/bin`。默认没有已加载 module。
如需 CUDA、MPI、NCCL、Python、容器等环境，先在计算节点上确认现有模块或项目脚本，不要假设登录节点和计算节点环境完全一致。

一般使用`rsync`传输文件（除非明确指示，否则不使用`--delete`）。使用 `scp` 或 `rsync` 时不要依赖 `~`，应使用绝对路径。涉及工作区内容时优先在计算节点确认 `~/gaia-fs` 的真实路径与可用性。

## 作业提交与调度系统

本平台使用 Slurm 管理。默认分区为 `GAIA`，节点范围为 `dgx-gaia-[09-63]`。`salloc` window 用于持有 allocation；真正工作应通过 SSH 进入
分配到的计算节点执行。若没有比较明确的指定时间，默认按 30 分钟申请。
作业名一般使用 `<jobname>.huanghaopeng` 形式，例如 `test.huanghaopeng`。作业无特殊要求，一律使用`--exclusive`。

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

`dgx-gaia-*` 计算节点探测到的 Slurm 配置大致如下：

```text
CPU: 224 logical CPUs/node, 2 sockets, 56 cores/socket, 2 threads/core
内存: 约 2,063,900 MB/node
GPU: gpu:8
OS: Ubuntu 22.04 系列，计算节点为 NVIDIA 内核
```

节点状态经常变化，运行前以 `sinfo` 为准，避开 `down`、`drain`、`inval` 等不可用节点。

## 互联网网络

当前登录环境没有检测到 `http_proxy` / `https_proxy`。不要假设 GAIA 可直接访问互联网；需要 `git clone`、`pip install`、容器拉取或外部下载时，
先做轻量网络探测或向用户确认可用代理、镜像和凭据。
