CC = gcc
CXX = g++
MPICC = mpicc
MPICXX = mpicxx # mpicxx is defined in both Intel/Open MPI

MYS_DIR ?= ..

CFLAGS = -I$(MYS_DIR)/include -Wall -Wextra -Werror -g
CXXFLAGS = -std=c++11 -I$(MYS_DIR)/include -Wall -Wextra -Werror -g
LFLAGS = 

basic: basic-gcc.c
	$(CC) $(CFLAGS) $(LFLAGS) basic-gcc.c
	$(CXX) $(CXXFLAGS) $(LFLAGS) basic-g++.cpp
	$(MPICC) $(CFLAGS) $(LFLAGS) basic-mpicc.c
	$(MPICXX) $(CXXFLAGS) $(LFLAGS) basic-mpic++.cpp

test-pool: test-pool.c
	$(CC) $(CFLAGS) $(LFLAGS) $^
