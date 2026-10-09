This is user skill misc "clusters/donau". Last modified: 2026-10-09.

# Donau CLI 原始帮助

以下内容于 2026-10-09 从 LineShine `login01` 采集。`dsub`、`djob` 属于 Donau CLI；`dinfo` 来自该平台的 Donau toolkit，其他 Donau 部署可能不同。若当前命令输出与本文冲突，以当前 `--help` 为准。

## `dsub --help`

```text
Usage: dsub [options] command [arguments...]
basic options:
          --array                  submits an array job with array size. Range: [2, 4096]
    -A,   --account                submits a job to a specified account.
    -bf,  --batch-file             batch submits jobs using a file. Example: dsub -bf "submit.txt"
    -cp,  --checkpoint             submits a job with the checkpoint function enabled.
          --container              submits a container job by specifying image and options. Example: image=busybox,options='--rm'
    -d,   --desc                   submits a job with a job description.
    -I,   --interactive            submits an interactive job and creates a pseudo-terminal when the job starts.
          --mpi                    specifies the MPI type. Supported: hmpi, openmpi, intelmpi, mpich
    -n,   --name                   submits a job with the specified job name.
    -q,   --queue                  submits a job to a specified queue.
    -s,   --script                 submits a script job with the options identified by #DSUB in the script.
          --tag                    specifies the custom job tag. e.g.: comment_key1=value1,comment_key2=value2
    -w,   --watch                  submits a blocking job and displays the jobs status on the console.
    -wo,  --watch-output           submits a blocking job and displays the job status and run logs on the console.
          --x11                    submits a job using SSH X11 forwarding.
scheduling options:
    -ds,  --data-stage             specifies a data staging configuration file.
    -bb,  --burst-buffer           specifies a burst buffer configuration file.
    -D,   --dependency             specifies dependencies between jobs. Example: dsub -D "1=RUNNING""echo 'hello world'"
    -jr,  --job-requeue            specifies the maximum number of job requeue times when certain 'exitCode' conditions are met. Example: 3(exitCode=101-110)
    -p,   --priority               specifies a job priority. A larger value indicates a higher priority. Default: 1. Range: [1, 9999]
    -S,   --schedule-time          specifies the time to schedule a job. Format: YYYY/mm/dd HH:MM:SS
          --topology               specifies the topology unit for a job. Example: 'type=L1,pendtime=60'|'blade[count=3]'
    -x,   --exclusive              specifies the exclusivity level for a job. Supported level: job
resource options:
    -a,   --affinity               specifies affinity resource requirements (CPUs and memory) for a job. Example: numa[count=1, distribution=pack]
    -aa,  --any-arch               runs a job on nodes of any architecture.
    -ar,  --adv-reservation        specifies the advanced reservation for a job.
    -jR,  --job-resource           specifies job resource requirements.
          --label                  assigns a job to nodes with specified labels. Example: pmi,ssd,!maintenance
    -mR,  --min-resource           specifies the minimum replica resource requirements for a job. Example: cpu=1,mem=128MB
    -nl,  --node-list              lists nodes, resource pools, or topology to run a job. Example: rack1 host1 respool+3 host[10-20]+2 !host15
    -nn,  --nnodes                 specifies the number of nodes to run a job.
    -N,   --replica                specifies the number of task replicas. Default: 1. Range: [1, 2147483647].
                                   The value can be a number or a value range in the format of min,max. Example: -N 3,5
    -O,   --order                  specifies a candidate node sequence policy. Example: cpu:mem:util
    -rpn, --replica-per-node       specifies the maximum number of replicas a job runs on each node. Range: [1, 2048]
    -R,   --resource               specifies replica resource requirements. Default: cpu=1,mem=128MB
    -RG,  --resource-groups        specifies multiple resource groups with resource requirements for a job. Example: -RG '{-R 'cpu=8'} + {-N 2}'
    -RL,  --resource-limit         specifies the replica resource limit. Supported: mem. Example: mem=10485760MB
runtime options:
          --cwd                    specifies the working directory.
    -e,   --error                  appends the standard error output of a job to the specified file.
    -eo,  --error-override         overrides the standard error output of a job to the specified file.
          --env                    specifies environment variables for a job. Example: none|all|EXEC_HOME=/root
    -o,   --output                 appends the standard output of a job to the specified file.
                                   If only -o is specified for dsub, stdout and stderr are output to the file specified by -o.
    -oo,  --output-override        overrides the standard output of a job to the specified file.
                                   If only -oo is specified for dsub, stdout and stderr are output to the file specified by -oo.
          --posthook               specifies post-execution command after a job completes.
          --prehook                specifies pre-execution command before a job starts.
          --precheck               specifies the type of precheck and the number of redundant nodes. Example: type=basic,backup-nodes=2.
    -rnc, --resize-notify-cmd      specifies the path to the resize-notify-cmd script of a resizable job.
    -T,   --timeout                sets the timeout interval for a job, in seconds. Default: 0. Range: [0, 31536000]
    -ug,  --usergroup              sets the user group ID. Range: [0, 4294967294]
other options:
    -J,   --json                   displays received packets as JSON strings.
    -h,   --help                   displays help information for the command.
```

## `djob --help`

```text
Usage: djob [options] [jobid | jobid.index]
options:
    -A,   --account                queries jobs by account.
    -dr,  --detail-reason          queries details about the WAITING or PENDING state. It must be used together with -l or -ll.
          --dependency             queries dependencies of jobs.
    -dg,  --dep-group              queries dependent job groups of jobs. It must be used together with --dependency.
    -D,   --done                   queries completed (SUCCEEDED and FAILED) jobs.
    -en,  --exec-node              queries jobs by execution node.
    -et,  --end-time               queries jobs by end time. Format: YYYY/mm/dd HH:MM:SS
          --expand                 displays job without compressed execNodes.
    -hr,  --host-reason            queries the host-level reason why a job is in the WAITING or PENDING state. It must be used together with -l or -ll, and can be set to 1 (brief) or 2 (details).
    -J,   --json                   displays received packets as JSON strings.
    -l,   --long                   displays information in long format.
    -ll,  --longlong               display in long format, contains rusage/affinity/pids info
    -n,   --name                   queries jobs by name. Fuzzy query is supported.
          --no-header              displays information without header.
    -o,   --output                 displays information in custom format.
    -p,   --page                   specifies the page number for a paginated query. Default: 1. Range: [1, 65535]
    -ps,  --page-size              specifies the page size for a paginated query. Default: 200. Range: [1, 200]
    -q,   --queue                  queries jobs by queue.
    -s,   --job-state              queries jobs by state.
    -st,  --start-time             queries jobs by start time. Format: YYYY/mm/dd HH:MM:SS
          --step                   query step info of jobs. Example: djob --step jobID
    -t,   --time-type              queries jobs by time point. Supported values: submit | start | end
    -u,   --user                   queries jobs by user. Supported users: all | [user]
    -ug,  --usergroup              queries jobs by user group. Supported user groups: all | [user group]
    -w,   --wide                   displays information in wide format.
    -h,   --help                   displays help information for the command.
```

## `dinfo --help`

```text
Usage: dinfo [OPTIONS]
Required Options:
    -q:
        查询队列信息
    -r:
        查询机框/柜信息
    -N:
        查询集群节点信息
    --type:
        信息展示形式：rack/queue/node

Examples:
    # 查询队列内节点信息，按框统计展示
    dinfo -q queuename --type rack
    dinfo -q all --type rack

    # 查询队列内节点信息，按节点状态统计展示
    dinfo -q queuename
    dinfo -q all

    # 查询队列内节点信息，按列表展示(不建议查询all或default，查询慢)
    dinfo -q queuename --type node

    # 查询框/柜内节点信息，按节点状态统计展示
    dinfo -r A01,C04-1
    dinfo -r A
    dinfo -r all

    # 查询框/柜内节点信息，按列表展示
    dinfo -r A01,C04-1 --type node
    dinfo -r A --type node

    # 查询框/柜内节点信息，按队列统计展示
    dinfo -r A01,C04-1 --type queue
    # 查询框/柜内节点属于指定队列的节点信息
    dinfo -r A01 --type queue --type_para queuename

    # 查询所有节点信息，按节点状态统计展示
    dinfo -N

    # 查询指定节点的所属队列
    dinfo -N nodename --type queue

    # 查询指定节点的所属机框
    dinfo -N nodename --type rack
```
