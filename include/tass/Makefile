CXX = mpicxx
CFLAGS = -O3 -fopenmp #-DNDEBUG

CL  = mpicxx
LFLAGS = -lm -lstdc++

helmholtz : main.o #par_struct_mv.o seq_struct_mv.o
	$(CL) $^ $(LFLAGS) -o $@

%.o : %.cpp
	$(CXX) -c $^ $(CFLAGS)

.PHONY: clean

clean : 
	rm helmholtz *.o 