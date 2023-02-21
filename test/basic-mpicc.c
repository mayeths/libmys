// Dont include mpi.h before mys.hpp because
// struct timespec in GCC 5.2 require _POSIX_C_SOURCE
// which is just defined in mys.h
#define MYS_IMPL
#include "mys.h"

int main() {
	MPI_Init(NULL, NULL);
	DLOG(0, "Test DLOG function");
	RLOG_ORDERED("Test RLOG_ORDERED function");
	MPI_Finalize();
	return 0;
}
