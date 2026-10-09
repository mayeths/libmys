---
name: clusters
description: >-
  远程集群操作practice，适用于用户要求在这些集群上运行、构建、测试、查看内容的场景。
  集群：ssh host `kp`、`yc`、`qh`、`eos`、`gaia`、`shuguang`、`lineshine`。
---

This is user skill "clusters". Last modified: 2026-10-09.

# 集群操作规范

本文档只描述集群总规范。各集群的具体 practice 和信息（账号、路径、环境加载、分区、硬件、网络等）放在独立 reference 文件中。本文档不包含具体项目规则。

使用方式：

1. 根据用户消息、SSH host、终端 prompt、节点名和项目上下文识别目标集群。
2. 读取该集群对应的 reference。
3. 按 reference 完成连接、环境初始化、路径选择、资源申请和命令执行。

## 集群索引

当前 skill 收录了以下集群：

| 集群 | 参考文档 |
| --- | --- |
| `kp` | `references/kp.md` |
| `yc` | `references/yc.md` |
| `qh` | `references/qh.md` |
| `eos` | `references/eos.md` |
| `gaia` | `references/gaia.md` |
| `shuguang` | `references/shuguang.md` |
| `lineshine` | `references/lineshine.md` |

## 如何创建集群 Reference

新增或重写集群 reference 时，参考 `references/yc.md`、`references/kp.md` 和 `references/gaia.md` 的结构，包含连接、tmux、账号与路径、环境、文件同步、调度、计算节点硬件、网络和互联网。

创建集群 reference 时，探测并记录集群的静态平台信息。使用只读命令分别探测登录节点和计算节点；异构集群分别探测各类计算节点。命令不存在时跳过并注明。探测时注意环境变量 `HUANGHAOPENG_PLATFORM_ID`，并把值记录到 reference。若变量未设置，创建 reference 时通知用户去 `.bashrc` 或 `.zshrc` 等环境文件添加。

示例：
```bash
# 身份与操作系统
whoami
hostname
echo $HUANGHAOPENG_PLATFORM_ID
uname -a
cat /etc/os-release
# CPU、Cache、NUMA与内存
lscpu
lscpu -C
lscpu -e=CPU,NODE,SOCKET,CORE,CACHE
numactl --hardware
free -h
# 磁盘与文件系统
lsblk -o NAME,MODEL,TYPE,SIZE,FSTYPE,MOUNTPOINTS
df -hT
# PCIe、加速器与节点内拓扑
lspci -nn
lstopo-no-graphics
nvidia-smi -L
nvidia-smi --query-gpu=name,memory.total,pci.bus_id,driver_version --format=csv,noheader
nvidia-smi topo -m
rocm-smi
hy-smi
# 网卡、InfiniBand与RDMA
ip -br link
ibv_devinfo
ibstat
rdma link
# 调度系统、分区与队列
command -v salloc srun sbatch dsub djob dinfo
sinfo -o '%P|%a|%l|%D|%N'
dinfo -q
```
