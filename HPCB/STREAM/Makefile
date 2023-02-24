# Mayeths' STREAM Benchmark 2023.02.18 (based on STREAM Benchmark 5.10)
# NOTE: stream_mpi.c was stream/Versions/stream_mpi.c
#       stream_omp.c was stream/stream.c

MPICC ?= mpicc
OMPCC ?= gcc

OPT=-DNTIMES=10 -std=c99 -Wall -lm -Ofast -ftree-vectorize
MPICFLAGS=$(OPT) # stream_mpi.c get array size from command line
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
