---
name: cluster-shuguang
description: >-
  曙光 Shuguang（国家超算互联网核心节点·分区一，郑州）Slurm DCU 平台操作规范。适用于 ssh host `shuguang`、`shuguangxukai`，
  以及用户要求在 shuguang 上运行、构建、测试、查看内容的场景。
---

This is user skill "cluster-shuguang".

# 曙光 Shuguang 平台 Practice

本文档只描述曙光平台的操作方式，不包含具体项目规则。这是国家超算互联网核心节点【分区一】（郑州）的 Sugon OS 8.9 + Slurm 集群，计算节点搭载
**海光 DCU**（GPU 架构 `gfx936`，`hy-smi` 中显示为 `HCU`），软件栈基于 **DTK（曙光版 ROCm）+ HIP + RCCL**，对应 NVIDIA 的
CUDA/NCCL：`hipcc`≈`nvcc`、`hy-smi`≈`nvidia-smi`、`librccl`≈`libnccl`。

## 登录与连接

曙光默认通过本地 tmux 会话进入 `ssh shuguang`。除非只是无状态、无需环境加载、只读且轻量的探测命令，否则不要使用 `ssh shuguang 'cmd'`
这类一次性命令。使用本地持久 tmux 会话 `AISG` 可以保留登录会话、Slurm allocation、计算节点工作路径和环境状态，用户也可随时 attach 查看或协助。

SSH 配置（本机 `~/.ssh/config`，登录入口 `zzeshell.scnet.cn:65032`）：

- `shuguang`：用户 `scnethpc2615`，密钥 `~/.ssh/id_rsa_shuguang`（**正式账号，多人共享**，个人工作目录 `/work2/share/scnethpc2615/huanghaopeng`）。
- `shuguangxukai`：用户 `scnethpc2667`，密钥 `~/.ssh/id_rsa_shuguang_xukai`（**其他项目组账号**，节点紧张时借用其 Slurm 资源探测）。

登录后进入登录节点 `zz-login01`。查看或创建 `AISG` 会话，并确保默认 window `cmd1` 存在：

```bash
tmux has-session -t AISG 2>/dev/null || tmux new-session -d -s AISG -n cmd1
tmux list-windows -t AISG | grep -q 'cmd1' || tmux new-window -t AISG -n cmd1
tmux capture-pane -t AISG:cmd1 -p | tail -n 30
```

如果还未进入曙光登录节点，在对应 window 中发送：

```bash
tmux send-keys -t AISG:cmd1 'ssh shuguang' Enter
```

不要在已经进入曙光时重复执行 `ssh shuguang`，以免产生嵌套 SSH。若平台长时间无响应，通常说明网络或登录连接已中断，需要重新连接。

登录入口是负载均衡，多台登录节点（ssh config已经设置强制`zz-login04`，该登录节点可以访问`/work2`），各节点 host key 不同。

## 网页登录与 SSH 密钥

SSH 私钥**有有效期，一般 30 天**，过期后需重新到网页下载。获取/更新流程：

1. 登录入口网页（点右上角“登录”）：https://www.scnet.cn/ui/mall/
2. 进入控制台：https://www.scnet.cn/ui/console/index.html#/space/dashboard
3. 控制台上方“命令行「E-Shell」”：https://www.scnet.cn/ui/console/index.html#/space/shell
4. 在 E-Shell 网页**左上角“SSH连接”**里查看 SSH 地址、端口、用户名并下载密钥（密钥有有效期）。

下载的密钥放到本机后须 `chmod 600`。
本机已有的密钥文件：`~/.ssh/id_rsa_shuguang`（scnethpc2615）、`~/.ssh/id_rsa_shuguang_xukai`（scnethpc2667），更新时覆盖对应文件即可。

参考手册：

- 平台使用手册：https://www.scnet.cn/help/docs/mainsite/introduction/
- E-Shell / SSH 连接超算集群：https://www.scnet.cn/help/docs/mainsite/hpc/cmd/connect-to-hpc/

## 命令与脚本

为了支持并行任务，`AISG` 中的 tmux window 按 `cmd1`、`cmd2`、`salloc1` 等命名。通常普通命令在默认的 `cmd1` 中运行。

登录节点 `zz-login01` 只用于编译、编辑、查询 Slurm、申请资源和管理文件。**严禁在登录节点直接运行程序**（平台 banner 明确提示），不要在登录节点跑
benchmark、MPI/OpenMP、训练、推理或长时间重负载任务。

运行命令时，应在命令后加上完成标记，方便判断命令是否结束以及退出码：

```bash
tmux send-keys -t AISG:cmd1 '<command>; echo __AISG_DONE_$?__' Enter
sleep 2
tmux capture-pane -t AISG:cmd1 -p -S -120 | tail -n 80
```

如果没有看到 `__AISG_DONE_...__`，说明命令可能仍在运行，或者正在等待输入。此时不要继续向同一 window 发送无关命令。

## 工作环境与计算节点

曙光的工作路径是：

```text
/work2/share/scnethpc2615/huanghaopeng      # 个人工作目录，盘符 ParaStor_03_work2（约 9.4 PB 并行盘）
```

`scnethpc2615` 是**多人共享账号**，账号根目录 `/work2/share/scnethpc2615/` 下各人有自己的子目录（如 `huanghaopeng`、`mxyu` 等）。本人的工作与算例统一放在 `/work2/share/scnethpc2615/huanghaopeng`。
`work2` 据厂家反馈比默认 home 目录稳定（默认目录有已知问题）。**`/work2` 只在计算节点与zz-login04登录节点挂载，其他所有登录节点访问不到**

默认工作流：

1. 在登录节点用 `salloc` 申请一个计算节点。
2. 保持 `salloc` window 不退出，以维持 allocation。
3. `ssh` 到分配到的计算节点（如 `n12r3n04`）。
4. 在计算节点上 `cd /work2/share/scnethpc2615/huanghaopeng` 后再进行项目构建、测试、运行和文件查看。

示例（申请 window）：

```bash
tmux list-windows -t AISG | grep -q 'salloc1' || tmux new-window -t AISG -n salloc1
tmux send-keys -t AISG:salloc1 'ssh shuguang' Enter
tmux capture-pane -t AISG:salloc1 -p | tail -n 30
```

确认 `salloc1` 已在曙光登录节点 prompt 后再申请资源（分区见下文）：

```bash
tmux send-keys -t AISG:salloc1 'salloc -p hpctest06 -N 1 --gres=dcu:8 --time=0:30:00' Enter
```

看到分配到节点后，在单独 window 进入该计算节点（曙光需先回到登录节点再 `ssh <node>`，无 ProxyJump）：

```bash
tmux list-windows -t AISG | grep -q 'cmd1' || tmux new-window -t AISG -n cmd1
tmux send-keys -t AISG:cmd1 'ssh shuguang' Enter
# 待登录节点 prompt 后：
tmux send-keys -t AISG:cmd1 'ssh <allocated-node>; cd /work2/share/scnethpc2615/huanghaopeng; echo __AISG_NODE_READY__' Enter
```

其中 `<allocated-node>` 替换为 `squeue` / `salloc` 输出中的节点名，例如 `n12r3n04`。可在登录节点用 `squeue -u $(whoami)` 确认当前 allocation。

## 工作环境（账号 / shell / module）

- 正式账号 `scnethpc2615`，默认 shell `bash`，prompt 形如 `[scnethpc2615@zz-login01 ~]$`。家目录 `/public/home/scnethpc2615`
  （盘符 `scnet.hx:/mnt/public`，NFS，登录/计算节点均挂载，但容量小且非主力盘，产物放 `work2`）。
- 登录后默认不加载任何 module（`module list` 为空），需要什么都得手动 `module load`。
- 模块系统是 **environment-modules 4.5.2（Tcl）**，不是 Lmod。
- 模块树分三处：`/public/software/modules/lagacy`（旧版编译器/cmake）、`/public/software/modules/base`（gcc/intel/mpi/mathlib/dtk 等基础栈）、
  `/public/software/sghpc_sdk/modulefiles`（**DTK SDK**：`compiler/dtk/*`、`app/rccl/*`、`mpi/{openmpi,intelmpi,hpcx,ucx}/.../{shca,mlnx}` 等）。
- 现成可用环境脚本`source /work2/share/scnethpc2615/huanghaopeng/set_env`如下：

```bash
module purge
module load sghpc-mpi-gcc/26.3
module load compiler/dtk/25.04.4
export MYS_DIR=/work2/share/scnethpc2615/huanghaopeng/project/libmys
```

一般使用 `rsync` 传输文件（除非明确指示，否则不使用 `--delete`）。使用 `scp`/`rsync` 时不要依赖 `~`，应使用绝对路径。`.vscode/sftp.json` 暂无曙光配置，
配置同步前需与用户确认目标路径。

```bash
rsync -avP /本地/路径 shuguang:/work2/share/scnethpc2615/huanghaopeng/
# 等价：scp / ssh n12r3n01 也都走同一跳板规则
```

前提是该计算节点正被本账号的 allocation 持有（`pam_slurm_adopt` 才放行登录）。需要传到 scnethpc2667 的节点时，把节点块的 `ProxyJump`/`User`
换成 `shuguangxukai`/`scnethpc2667`。已实测可用。

## DCU / HIP / RCCL 环境

计算节点为 8 卡海光 DCU（架构 `gfx936`），通过 DTK 提供 HIP/RCCL 等。加载与对照：

```bash
module load compiler/dtk/25.04.4        # ROCM_PATH=/public/software/compiler/dtk-25.04.4, HIP_PATH=$ROCM_PATH/hip
```

- 编译器：`hipcc`（即 `dcc`，基于 clang 17，对应 `nvcc`），目标架构 `--offload-arch=gfx936`；位于 `$ROCM_PATH/bin`。
- 监控显卡：在**计算节点**上 `hy-smi`（`/opt/hyhal/bin/hy-smi`，对应 `nvidia-smi`），输出列名为 `HCU`，单卡 PwrCap 1000W；登录节点无 DCU 设备。
- 设备查询：`rocminfo`（注意输出里 CPU agent 也叫 `Hygon C86 Processor`，GPU agent 才显示 `gfx936`，用 `rocminfo | grep -i gfx` 过滤）。
- 通信库：RCCL（对应 NCCL），主库 `$ROCM_PATH/lib/librccl.so`；RDMA 加速插件 `librccl-net-shca`（配合 `app/rccl/shca_rdma_plugins/*` 模块，
  对应曙光自研 SHCA 网卡）。
- 多卡/多机 RCCL 走 SHCA 网络时，建议加载 `app/rccl/*` 相关模块并使用 `shca` 变体的 MPI/UCX（见下）。

## 通信：MPI / RCCL 与 MPI 选择

**主力是 MPI（OpenMPI + UCX，GPU-aware），不是 RCCL。** 现场 LICOM 算例（`mxyu/run-1km-*/sub_gpu_*.sh`）即用 `mpirun` 起进程、
每张 DCU 一个 rank，通信走 UCX over SHCA，并启用 ROCm 显存直通（GPUDirect RDMA），典型环境：

```bash
module use /work2/share/sghpc_sdk/modulefiles
module load sghpc-mpi-gcc/26.3                          # SDK 自带 ucx + gcc + openmpi
export UCX_NET_DEVICES=shca_0:1,shca_1:1,shca_2:1,shca_3:1   # 用全部 4 张 SHCA
export UCX_TLS=self,mm,rc,rocm_copy,rocm_ipc           # rc=SHCA RDMA, rocm_*=DCU 显存直通（GPU-aware）
mpirun -np <N> -x UCX_UD_VERBS_TIMEOUT=120s ./mpi_bind.sh ./your_exe
```

RCCL（对应 NCCL）集群也提供（`librccl` + `librccl-net-shca` + `app/rccl/*` 模块），但主要面向深度学习式集合通信；HPC 数值模式（如 LICOM）
用 MPI 即可，一般不需要 RCCL。

MPI 选择：本机互连是曙光自研 **SHCA 400G**（非 Mellanox），DTK SDK 里的 MPI/UCX 多数提供 `shca` 与 `mlnx` 两种变体——本平台应优先选 **`shca` 变体**。
常见可用（`module av` 全量）：

- `sghpc-mpi-gcc/26.3`（SDK 自带 MPI+GCC 组合，现场默认用它）
- `mpi/intelmpi/2021.14.0`、`mpi/intelmpi/2021.17.2.94/{shca,mlnx}`
- `mpi/openmpi/4.1.8/gcc-12.2.0/{shca,mlnx}`、`mpi/openmpi/5.0.3/{gcc-8.5.0,clang}/{shca,mlnx}`
- `mpi/hpcx/2.18.0/gcc-8.5.0/{shca,mlnx}`、`mpi/ucx/1.18.0/dtk-25.04.4/{shca,mlnx}`
- 旧栈（`modules/base`）另有 intelmpi/openmpi/mpich/hpcx 多版本，与对应 `compiler/*` 配套使用。

## 作业提交与调度系统

本平台使用 Slurm 管理。`salloc` 返回的 shell 仍在登录节点，只是持有 allocation；真正运行到计算节点需 `ssh` 进入节点或用 `srun`/`mpirun`。
需要 `salloc` 时使用 tmux 专用 window（`salloc1`、`salloc2`），普通运行命令在 `cmd1`/`cmd2`。若没有明确指定时间，默认按 30 分钟申请。

作业名一般使用 `<jobname>.huanghaopeng` 形式，例如 `test.huanghaopeng`。作业无特殊要求，一律使用`--exclusive`。

分区情况：

- `sinfo` 当前只显示 **`hpctest06`**：时限 `2:00:00`，约 47 节点（`n12r3n*` / `n12r4n*` / `n13r1n*`），常见状态 alloc，少量 drain/idle。
- `sacctmgr show assoc` 显示账号关联的分区可能与 `sinfo` 不同（探测账号 `scnethpc2667` 关联到 `hpctest02`+QOS `normal`，但该分区不在 `sinfo` 列表中）。
  正式账号 `scnethpc2615` 的可用分区/QOS 以其自身 `sacctmgr -p show assoc user=scnethpc2615` 为准，提交前先确认。

常用查询：

```bash
sinfo -o '%P|%a|%l|%D|%t|%N'
squeue
squeue -u $(whoami)
sacctmgr -p show assoc user=$(whoami) format=account,partition,qos
```

资源申请示例：

```bash
salloc -p hpctest06 -N 1 --exclusive -J test.huanghaopeng --gres=dcu:8 --time=0:30:00
```

节点状态经常变化，运行前以 `sinfo` 为准，避开 `drain`/`down` 节点。不要在没有用户确认的情况下长时间占用大量节点。

## 节点硬件

计算节点（`n12r3n*` / `n12r4n*` / `n13r1n*`）实测配置：

```text
CPU:  Hygon C86 Processor (海光，x86_64), 2 sockets × 64 cores = 128 cores/node, 1 thread/core
NUMA: 8 个 NUMA domain，每域 16 核：0-15 / 16-31 / 32-47 / 48-63 / 64-79 / 80-95 / 96-111 / 112-127
内存: 约 1.0 TiB/node（free 显示 1.0Ti），无 swap
DCU:  8 × 海光 DCU（gfx936，PCI ven 0x1d94 dev 0x6320，hy-smi 显示 HCU 0-7，单卡 PwrCap 1000W）
互连: 曙光自研 SHCA 400G，4 张（shca_0-3 / ib0-3，State Active，Rate 400）；另有 2 路千兆以太 eno1/eno2 管理网（拓扑见下节）
OS:   Sugon OS 8.9（x86_64）
```

## 节点 PCIe / 网络互联拓扑

实测（`/sys/class/{infiniband,drm}/*/device` + PCIe 路径）：每节点 8 DCU + 4 SHCA，按 8 个 NUMA / 8 个 PCIe root 分布，**SHCA 网卡与 DCU 共享同一
PCIe switch（贴 DCU 放，支持 GPUDirect RDMA），不是单独挂在某个 CPU root 上**。配比 8 DCU : 4 HCA = 2:1，每张 HCA 与其中一张 DCU 同 switch。

```text
DCU(numa): card1=09:00(n0) card5=77:00(n1) card4=55:00(n2) card3=36:00(n3)
           card6=85:00(n4) card9=f5:00(n5) card8=d5:00(n6) card7=b5:00(n7)
HCA(numa): shca_0=03:00(n0) shca_1=73:00(n1) shca_2=86:00(n4) shca_3=f6:00(n5)
共 switch:  HCA 与 DCU 挂同一上游 switch，例 numa4：
           shca_2  80/80:01.1/81:00.0/82:03.0/86:00.0
           card6   80/80:01.1/81:00.0/82:01.0/83:00.0/84:00.0/85:00.0   ← 同 switch 81:00.0
```

要点：

- 网卡只装在 NUMA 0/1/4/5 上（对应 DCU 09/77/85/f5）——这 4 张 DCU 直连本地 SHCA 做 GPUDirect RDMA。
- NUMA 2/3/6/7 的 DCU（55/36/d5/b5）**没有本地网卡**，跨 root/socket 才能用到 SHCA；多机大规模时进程-卡-网卡绑定（参考现场 `mpi_bind.sh`）对性能很关键。
- UCX 用 `UCX_NET_DEVICES=shca_0:1,...,shca_3:1` 同时启用 4 张网卡，由拓扑就近选择。

## 互联网网络

曙光**默认不能直接访问互联网**：登录节点与计算节点均无 `http_proxy`/`https_proxy`，`curl https://github.com` 直接超时无响应。不要假设
`git clone` / `pip install` / `conda` 能联网。本机 SSH 对所有 host 配置了 `RemoteForward 26635`（SOCKS）与 `RemoteForward 26636 -> 127.0.0.1:36636`
（到本地 Clash），如需联网可在远端显式指向这些端口（如 `export https_proxy=socks5h://127.0.0.1:26635`）后再做轻量探测；否则优先用 module 已有软件、
本地资源或国内镜像（如清华 PyPI 源），必要时与用户确认可用代理与凭据。
