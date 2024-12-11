#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>
#include <map>
#include <algorithm>

#include <mpi.h>

#define MYS_IMPL
#include "mys.h"

int main() {
    MPI_Init(NULL, NULL);
    TLOG_ORDERED("Test TLOG_ORDERED function");
    DLOG(0, "Test DLOG function");
    ILOG(0, "Test ILOG function");
    WLOG(0, "Test WLOG function");
    ELOG(0, "Test ELOG function");
    FLOG(0, "Test FLOG function");
    RLOG(0, "Test RLOG function");
    MPI_Finalize();
    return 0;
}
