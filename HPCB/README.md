## HPCB - High Performance Computing Benchmarks (of Mayeths)

**HPCB** is inspired by [HPC Challenge Benchmark](https://en.wikipedia.org/wiki/HPC_Challenge_Benchmark) and [UL HPC MPI Tutorial](https://ulhpc-tutorials.readthedocs.io/en/latest/parallel/mpi/OSU_MicroBenchmarks/). It's a suite based on 5 widely-used benchmarks that help understanding several performance aspects of a computing cluster, such as the computational efficiency, the bandwidth of local memory access, and the latency of remote communication.

### Motivation

Logging into a new HPC cluster for the first time can be daunting. What's the promising peak performance? what's the limitations? Will the software stack influence the performance of our application? Furthermore, different machines may have their own issues, such as communication disturbance, memory bandwidth problems, and so on.

Obtaining a clear picture of the system's capabilities is always worthwhile, as it can accelerate the troubleshooting process when encounter performance issues. This's why we perform benchmarking.

### Benchmark Suite

For the sake of simplifying, only 5 self-contained MPI(+OpenMP) based benchmarks are chosen:

- **[HPL](https://netlib.org/benchmark/hpl/)** – This benchmark measures performance of a solver for a dense system of linear equations. The score of Flop/s ranks the supercomputers on [TOP 500](https://www.top500.org/). It's chosen since it's the most renowned and leading HPC benchmark.
- **[HPCG](https://hpcg-benchmark.org/)** - This benchmark measures performance of a multigrid preconditioned conjugate gradient solver. It's chosen for its comprehensiveness, which offers another rank of [TOP 500](https://www.top500.org/lists/hpcg/).
- **[NPB](https://www.nas.nasa.gov/software/npb.html)** - This suite measures performance of 9 benchmarks from various domains. It's chosen because it's frequently used in academic literature to validate the feasibility of ideas.
- **[STREAM](https://www.cs.virginia.edu/stream/ref.html)** – This micro-benchmark measures the peak achievable bandwidth of a local memory system. It's chosen because of widely-used for evaluating memory performance.
- **[OSU](https://mvapich.cse.ohio-state.edu/benchmarks/)** - This micro-benchmark measures the communication performance between processes, including bandwidth and latency. It's chosen because it's one of the reliable way to evaluate network capabilities.

Diagram below shows how we use them to understand the different aspects of a cluster. Currently, the performance of I/O and accelerators like GPU(for AI workload) are not being evaluated.

```
========================== - - - - - - -
| COMP. | MEM.   | COMM. | I/O, GPU ... |
========================== - - - - - - -
| HPL   | STREAM | OSU   | -> System capabilities
--------------------------
| HPCG      | NPB        | -> Time based end-to-end performance
--------------------------
```

### Performance Evaluation

The following metrics are used to understand system capabilities.

#### Comupational Efficiency
...
- How to measure?
- The performance indicators: Flops per seconds (like GFLOPS).

#### Local Memory Bandwidth
...
- The performance indicators: Bytes per seconds (like GB/s).

#### Remote Communication Bandwidth and Latency

#### Time to solve

### Problems Diagnosis

Identifying potential system fault is also essential before testing user application. The benchmarks in HPCB give us the possibility to achieve this goal.

#### Disturbance in Computation by Operating System
#### Imbalanced Performance of Different NUMA Memory
#### Imbalanced Communication Bandwidth or Latency
#### Disturbance in Communication by Another Application

### Evaluation Details

#### Version of benchmarks

- HPL - 2.3 [(download)](https://netlib.org/benchmark/hpl/hpl-2.3.tar.gz)
- HPCG - 3.1 [(download)](https://hpcg-benchmark.org/downloads/hpcg-3.1.tar.gz)
- STREAM - 5.10-MPI [(download)](https://www.cs.virginia.edu/stream/FTP/Code/Versions/stream_mpi.c)
- OSU - 7.0.1 [(download)](http://mvapich.cse.ohio-state.edu/download/mvapich/osu-micro-benchmarks-7.0.1.tar.gz)
- NPB - 3.4.2-MPI [(download)](https://www.nas.nasa.gov/assets/nas/npb/NPB3.4.2.tar.gz)

Code of these benchmarks, particularly the kernels used for measurement, **have not been modified extensively**. However, to support problem diagnosis memtioned above, new features may be added to the drivers of kernel without bothering correctness and performance.

Some minor modifications are also applied to
- simplify the selection of input arguments;
- improve output formatting;
- provide alerts for potential invalid results.

Irrelevant files (such as `TODO`) are removed to make the working directory clear. Instructions to build, install and run are preserved. `LICENSE` and `COPYRIGHT` are preserved too. If these changes are not suitable for you, test with original code instead.

#### System Classes

Up to 2023, HPC servers can be classified into three categories based on their slot sizes:

- **Class A** workstations: 4\~16 nodes (typically 100\~1000 MPI ranks). They are commonly used for daily debugging and testing.
- **Class B** in-house clusters: 16\~128 nodes (typically 1000\~10000 MPI ranks). They are great for validating scalability and identifying bottlenecks in new ideas.
- **Class C** supercomputers: 128+ nodes (typically > 10000 MPI ranks). They are supported by professional maintainers or companies.

We collect and provide some reference results for these 3 scales:

- Laptop(Macbook) with M1 CPU, labeled with `M1`.
- Workstation with Kunpeng920 CPU, labeled with `KP`.
- Workstation with Intel Xeon Gold 6132 CPU, labeled with `LT`.
- Supercomputer with AMD EPYC 7H12 CPU, labeled with `YC`.

The special `xxxxx.ipynb` notebook plots their results together.
