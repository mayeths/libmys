#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.hpp"


int main() {
	DEBUG(0, "Test DEBUG function");
	DEBUG_ORDERED("Test DEBUG_ORDERED function");
	return 0;
}

