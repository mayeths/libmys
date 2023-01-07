CC = gcc
CXX = g++
MPICC = mpicc
MPICXX = mpic++

MYS_DIR ?= ..

CFLAGS = -std=c99 -I$(MYS_DIR)/include -Wall -Werror
CXXFLAGS = -std=c++11 -I$(MYS_DIR)/include -Wall -Werror
LFLAGS = -lm

basic: basic-gcc.c
	$(CC) $(CFLAGS) $(LFLAGS) basic-gcc.c
	$(CXX) $(CXXFLAGS) $(LFLAGS) basic-g++.cpp
	$(MPICC) $(CFLAGS) $(LFLAGS) basic-mpicc.c
	$(MPICXX) $(CXXFLAGS) $(LFLAGS) basic-mpic++.cpp
