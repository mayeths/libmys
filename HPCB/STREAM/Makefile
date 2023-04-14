# Mayeths' STREAM Benchmark 2023.02.18 (based on STREAM Benchmark 5.10)
# NOTE: stream.c was stream/Versions/stream_mpi.c

MPICC ?= mpicc

MPICFLAGS=-DNTIMES=10 -Wall -Wextra -lm -Ofast -ftree-vectorize

.PHONY: clean default naive sunway

default: clean naive

naive: naive.o
	$(MPICC) -o stream $(MPICFLAGS) -std=c99  src/stream.c $^

sunway: naive.o swgld_host.o swgld_slave.o swdma_host.o swdma_slave.o
	$(MPICC) -o stream $(MPICFLAGS) -std=c99 -DTEST_SUNWAY_GLD -DTEST_SUNWAY_DMA  src/stream.c $^

naive.o: src/impl-naive.c
	$(MPICC) -o $@ $(MPICFLAGS) -c $<

swgld_host.o: src/impl-swgld_host.c
	$(MPICC) -mhost -o $@ $(MPICFLAGS) -c $<

swgld_slave.o: src/impl-swgld_slave.c
	$(MPICC) -mslave -msimd -o $@ $(MPICFLAGS) -c $<

swdma_host.o: src/impl-swdma_host.c
	$(MPICC) -mhost -o $@ $(MPICFLAGS) -c $<

swdma_slave.o: src/impl-swdma_slave.c
	$(MPICC) -mslave -msimd -o $@ $(MPICFLAGS) -c $<

clean:
	rm -f stream *.o
