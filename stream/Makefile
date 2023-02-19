# Mayeths' STREAM Benchmark 230218 (based on STREAM Benchmark 5.10)
# NOTE: stream_omp.c was renamed from stream-5.10/stream.c
#       stream_mpi.c was copied from stream-5.10/Versions/stream_mpi.c

CC ?= gcc
MPICC ?= mpicc

CFLAGS = -DSTREAM_ARRAY_SIZE=100000000 -DNTIMES=10
# stream_mpi.c use STREAM_ARRAY_SIZE from runtime argument
MPICFLAGS = -DNTIMES=10

OPT=-std=c99 -Wall -lm -g -Ofast -ftree-vectorize -fopenmp -mcmodel=large

all: stream_mpi.exe stream_omp.exe

# stream_mpi.c also support openmp
stream_mpi.exe: stream_mpi.c
	$(MPICC) -o $@ $(OPT) $(MPICFLAGS) $<

stream_omp.exe: stream_omp.c
	$(CC) -o $@ $(OPT) $(CFLAGS) $<

clean:
	rm -f stream_mpi.exe stream_omp.exe
