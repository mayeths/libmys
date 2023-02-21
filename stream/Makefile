# Mayeths' STREAM Benchmark 230218 (based on STREAM Benchmark 5.10)
# NOTE: stream_omp.c was renamed from stream-5.10/stream.c
#       stream_mpi.c was copied from stream-5.10/Versions/stream_mpi.c

MPICC ?= mpicc
OMPCC ?= gcc

# stream_mpi.c use STREAM_ARRAY_SIZE from runtime argument
OPT=-DNTIMES=10 -std=c99 -Wall -lm -Ofast -ftree-vectorize
MPICFLAGS=$(OPT)
OMPCFLAGS=$(OPT) -fopenmp -mcmodel=large -DSTREAM_ARRAY_SIZE=100000000

.PHONY: clean

default: clean stream_mpi.exe

# stream_mpi.c also support openmp but we won't use it
stream_mpi.exe: stream_mpi.c
	$(MPICC) -o $@ $(MPICFLAGS) $<

stream_omp.exe: stream_omp.c
	$(OMPCC) -o $@ $(OMPCFLAGS) $<

clean:
	rm -f stream_mpi.exe stream_omp.exe
