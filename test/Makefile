TEST_CC?=gcc
TEST_CXX?=g++
TEST_MPICC?=mpicc
TEST_MPICXX?=mpicxx

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
	$(TEST_CC) -o $@ $(CFLAGS) $(LFLAGS) ex01.hello-gcc.c

ex02.hello-g++.exe: ex02.hello-g++.cpp
	$(TEST_CXX) -o $@ $(CXXFLAGS) $(LFLAGS) ex02.hello-g++.cpp

ex03.hello-mpicc.exe: ex03.hello-mpicc.c
	$(TEST_MPICC) -o $@ $(CFLAGS) $(LFLAGS) ex03.hello-mpicc.c

ex04.hello-mpic++.exe: ex04.hello-mpic++.cpp
	$(TEST_MPICXX) -o $@ $(CXXFLAGS) $(LFLAGS) ex04.hello-mpic++.cpp

# Tests

tests: $(TESTS)

test-pool.exe: test-pool.c
	$(TEST_CC) -o $@ $(CFLAGS) $(LFLAGS) $^

test-memory.exe: test-memory.c
	$(TEST_CC) -o $@ $(CFLAGS) $(LFLAGS) $^

test-trace.exe: test-trace.c
	$(TEST_CC) -o $@ $(CFLAGS) $(LFLAGS) $^ -O3

# End

.PHONY: clean examples tests

clean:
	rm -f a.out *.exe
