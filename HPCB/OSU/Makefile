# Mayeths' OSU Benchmark 2023.02.23 (based on OSU Micro Benchmarks 7.0.1)

CC=mpicc

CFLAGS=-std=c99 -Wall -Ofast -I./src_util
LFLAGS=-lm

.PHONY: clean p2p coll win

default: p2p coll win

p2p:  p2p_bibw p2p_bw p2p_latency
coll: coll_allgather coll_allreduce coll_alltoall coll_barrier coll_bcast
win:  win_get_bw win_get_latency win_put_bibw win_put_bw win_put_latency

p2p_bibw: src_p2p/osu_bibw.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
p2p_bw: src_p2p/osu_bw.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
p2p_latency: src_p2p/osu_latency.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)

coll_allgather: src_coll/osu_allgather.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
coll_allreduce: src_coll/osu_allreduce.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
coll_alltoall: src_coll/osu_alltoall.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
coll_barrier: src_coll/osu_barrier.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
coll_bcast: src_coll/osu_bcast.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)

win_get_bw: src_win/osu_get_bw.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
win_get_latency: src_win/osu_get_latency.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
win_put_bibw: src_win/osu_put_bibw.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
win_put_bw: src_win/osu_put_bw.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)
win_put_latency: src_win/osu_put_latency.c
	$(CC) -o $@ $< $(CFLAGS) $(LFLAGS)

clean:
	rm -f p2p_bibw p2p_bw p2p_latency
	rm -f coll_allgather coll_allreduce coll_alltoall coll_barrier coll_bcast
	rm -f win_get_bw win_get_latency win_put_bibw win_put_bw win_put_latency
