#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.hpp"

int main() {
	DLOG(0, "Test DLOG function");
	RLOG_ORDERED("Test RLOG_ORDERED function");
	return 0;
}
