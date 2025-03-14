echo "HPCG benchmark input file" >> bin/KP/HPL.dat
echo "Sandia National Laboratories; University of Tennessee, Knoxville" >> bin/KP/HPL.dat
echo "128 64 72" >> bin/KP/HPL.dat
echo "1" >> bin/KP/HPL.dat

salloc -w kp101 -N 1 --exclusive mpirun -x OMP_NUM_THREADS=1 -n 128 --cpus-per-rank 1 ./xhpcg
salloc -w kp101 -N 1 --exclusive mpirun -x OMP_NUM_THREADS=32 -n 4 -rf 1N4P32T.rf ./xhpcg

salloc -w kp101 -N 1 --exclusive mpirun -x OMP_NUM_THREADS=1 -n 16 ./xhpcg
salloc -w kp101 -N 1 --exclusive mpirun -x OMP_NUM_THREADS=8 -n 16 -rf 1N16P8T.rf ./xhpcg

salloc -w kp101 -N 1 --exclusive mpirun -x OMP_NUM_THREADS=32 -n 1 -rf 1N4P32T.rf ./xhpcg : -x OMP_NUM_THREADS=32 -n 3 -rf 1N4P32T.rf ./xhpcg

-x MIMALLOC_VERBOSE=1 -x LD_PRELOAD=$(realpath ~/project/mimalloc-2.1.2/out/release/libmimalloc.so)

salloc -w kp102 -N 1 --exclusive mpirun -x MIMALLOC_VERBOSE=1 -x LD_PRELOAD=$(realpath ~/project/mimalloc-2.1.2/out/release/libmimalloc.so) -x OMP_NUM_THREADS=1 -n 1 --cpus-per-rank 1 ./miniFE.x nx=64 ny=64 nz=1024 px=4 py=4 pz=8 :  -x OMP_NUM_THREADS=1 -n 127 --cpus-per-rank 1 ./miniFE.x nx=64 ny=64 nz=1024 px=4 py=4 pz=8 2>&1 | tee LOG.heapsize/64x64x1024-4x4x8.log