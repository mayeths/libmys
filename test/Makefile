CC?=gcc
CXX?=g++
MPICC?=mpicc
MPICXX?=mpicxx

MYS_DIR ?= ..

CFLAGS ?= -std=gnu99 -Wall -Wextra -Werror -g -O3 -I$(MYS_DIR)/include
CXXFLAGS ?= -std=c++11 -Wall -Wextra -Werror -g -O3 -I$(MYS_DIR)/include
LFLAGS = 

EXAMPLES=\
	ex01.hello-gcc.exe\
	ex02.hello-g++.exe\
	ex03.hello-mpicc.exe\
	ex04.hello-mpic++.exe

TESTS=\
	test-pool.exe\
	test-memory.exe\
	test-trace.exe

default:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory examples
#	@$(MAKE) --no-print-directory tests

# Examples

examples: $(EXAMPLES)

ex01.hello-gcc.exe: ex01.hello-gcc.c
	$(CC) -o $@ $(CFLAGS) $(LFLAGS) ex01.hello-gcc.c

ex02.hello-g++.exe: ex02.hello-g++.cpp
	$(CXX) -o $@ $(CXXFLAGS) $(LFLAGS) ex02.hello-g++.cpp

ex03.hello-mpicc.exe: ex03.hello-mpicc.c
	$(MPICC) -o $@ $(CFLAGS) $(LFLAGS) ex03.hello-mpicc.c

ex04.hello-mpic++.exe: ex04.hello-mpic++.cpp
	$(MPICXX) -o $@ $(CXXFLAGS) $(LFLAGS) ex04.hello-mpic++.cpp

# Tests

tests: $(TESTS)

test-pool.exe: test-pool.c
	$(CC) -o $@ $(CFLAGS) $(LFLAGS) $^

test-memory.exe: test-memory.c
	$(CC) -o $@ $(CFLAGS) $(LFLAGS) $^

test-trace.exe: test-trace.c
	$(CC) -o $@ $(CFLAGS) $(LFLAGS) $^ -O3

# End

.PHONY: clean examples tests

clean:
	rm -f a.out *.exe
