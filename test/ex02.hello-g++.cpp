#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>
#include <map>
#include <algorithm>

#define MYS_IMPL
#define MYS_NO_MPI
#include "mys.h"

int main() {
    TLOG_ORDERED("Test TLOG_ORDERED function");
    DLOG(0, "Test DLOG function");
    ILOG(0, "Test ILOG function");
    WLOG(0, "Test WLOG function");
    ELOG(0, "Test ELOG function");
    FLOG(0, "Test FLOG function");
    RLOG(0, "Test RLOG function");
    return 0;
}
