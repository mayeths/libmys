#include <mpi.h>

#define MYS_IMPL
#include "mys.h"

int main() {
	MPI_Init(NULL, NULL);
	DEBUG(0, "Test DEBUG function");
	DEBUG_ORDERED("Test DEBUG_ORDERED function");
	MPI_Finalize();
	return 0;
}
