#pragma once

#include "config.h"

#if defined(MYS_NO_MPI)

/*********************************************/
// MYSPI
/*********************************************/

#define MYSPI_VERSION 3
#define MYSPI_SUBVERSION 1
/*
 * Error classes and codes
 * Do not change the values of these without also modifying mpif.h.in.
 */
#define MYSPI_SUCCESS                   0
#define MYSPI_ERR_BUFFER                1
#define MYSPI_ERR_COUNT                 2
#define MYSPI_ERR_TYPE                  3
#define MYSPI_ERR_TAG                   4
#define MYSPI_ERR_COMM                  5
#define MYSPI_ERR_RANK                  6
#define MYSPI_ERR_REQUEST               7
#define MYSPI_ERR_ROOT                  8
#define MYSPI_ERR_GROUP                 9
#define MYSPI_ERR_OP                    10
#define MYSPI_ERR_TOPOLOGY              11
#define MYSPI_ERR_DIMS                  12
#define MYSPI_ERR_ARG                   13
#define MYSPI_ERR_UNKNOWN               14
#define MYSPI_ERR_TRUNCATE              15
#define MYSPI_ERR_OTHER                 16
#define MYSPI_ERR_INTERN                17
#define MYSPI_ERR_IN_STATUS             18
#define MYSPI_ERR_PENDING               19
#define MYSPI_ERR_ACCESS                20
#define MYSPI_ERR_AMODE                 21
#define MYSPI_ERR_ASSERT                22
#define MYSPI_ERR_BAD_FILE              23
#define MYSPI_ERR_BASE                  24
#define MYSPI_ERR_CONVERSION            25
#define MYSPI_ERR_DISP                  26
#define MYSPI_ERR_DUP_DATAREP           27
#define MYSPI_ERR_FILE_EXISTS           28
#define MYSPI_ERR_FILE_IN_USE           29
#define MYSPI_ERR_FILE                  30
#define MYSPI_ERR_INFO_KEY              31
#define MYSPI_ERR_INFO_NOKEY            32
#define MYSPI_ERR_INFO_VALUE            33
#define MYSPI_ERR_INFO                  34
#define MYSPI_ERR_IO                    35
#define MYSPI_ERR_KEYVAL                36
#define MYSPI_ERR_LOCKTYPE              37
#define MYSPI_ERR_NAME                  38
#define MYSPI_ERR_NO_MEM                39
#define MYSPI_ERR_NOT_SAME              40
#define MYSPI_ERR_NO_SPACE              41
#define MYSPI_ERR_NO_SUCH_FILE          42
#define MYSPI_ERR_PORT                  43
#define MYSPI_ERR_QUOTA                 44
#define MYSPI_ERR_READ_ONLY             45
#define MYSPI_ERR_RMA_CONFLICT          46
#define MYSPI_ERR_RMA_SYNC              47
#define MYSPI_ERR_SERVICE               48
#define MYSPI_ERR_SIZE                  49
#define MYSPI_ERR_SPAWN                 50
#define MYSPI_ERR_UNSUPPORTED_DATAREP   51
#define MYSPI_ERR_UNSUPPORTED_OPERATION 52
#define MYSPI_ERR_WIN                   53
#define MYSPI_T_ERR_MEMORY              54
#define MYSPI_T_ERR_NOT_INITIALIZED     55
#define MYSPI_T_ERR_CANNOT_INIT         56
#define MYSPI_T_ERR_INVALID_INDEX       57
#define MYSPI_T_ERR_INVALID_ITEM        58
#define MYSPI_T_ERR_INVALID_HANDLE      59
#define MYSPI_T_ERR_OUT_OF_HANDLES      60
#define MYSPI_T_ERR_OUT_OF_SESSIONS     61
#define MYSPI_T_ERR_INVALID_SESSION     62
#define MYSPI_T_ERR_CVAR_SET_NOT_NOW    63
#define MYSPI_T_ERR_CVAR_SET_NEVER      64
#define MYSPI_T_ERR_PVAR_NO_STARTSTOP   65
#define MYSPI_T_ERR_PVAR_NO_WRITE       66
#define MYSPI_T_ERR_PVAR_NO_ATOMIC      67
#define MYSPI_ERR_RMA_RANGE             68
#define MYSPI_ERR_RMA_ATTACH            69
#define MYSPI_ERR_RMA_FLAVOR            70
#define MYSPI_ERR_RMA_SHARED            71
#define MYSPI_T_ERR_INVALID             72
#define MYSPI_T_ERR_INVALID_NAME        73
#define MYSPI_ERR_LASTCODE              92
/* Per MPI-3 p349 47, MPI_ERR_LASTCODE must be >= the last predefined
   MPI_ERR_<foo> code. Set the last code to allow some room for adding
   error codes without breaking ABI. */

typedef unsigned long long int MYSPI_Datatype;
static const MYSPI_Datatype MYSPI_DATATYPE_NULL         = (MYSPI_Datatype)0;
static const MYSPI_Datatype MYSPI_BYTE                  = (MYSPI_Datatype)10000;
static const MYSPI_Datatype MYSPI_CHAR                  = (MYSPI_Datatype)10001;
static const MYSPI_Datatype MYSPI_SIGNED_CHAR           = (MYSPI_Datatype)10002;
static const MYSPI_Datatype MYSPI_UNSIGNED_CHAR         = (MYSPI_Datatype)10003;
static const MYSPI_Datatype MYSPI_WCHAR                 = (MYSPI_Datatype)10004;
static const MYSPI_Datatype MYSPI_SHORT                 = (MYSPI_Datatype)10005;
static const MYSPI_Datatype MYSPI_UNSIGNED_SHORT        = (MYSPI_Datatype)10006;
static const MYSPI_Datatype MYSPI_INT                   = (MYSPI_Datatype)10007;
static const MYSPI_Datatype MYSPI_UNSIGNED              = (MYSPI_Datatype)10008;
static const MYSPI_Datatype MYSPI_LONG                  = (MYSPI_Datatype)10009;
static const MYSPI_Datatype MYSPI_UNSIGNED_LONG         = (MYSPI_Datatype)10010;
static const MYSPI_Datatype MYSPI_LONG_LONG             = (MYSPI_Datatype)10011;
static const MYSPI_Datatype MYSPI_UNSIGNED_LONG_LONG    = (MYSPI_Datatype)10012;
static const MYSPI_Datatype MYSPI_FLOAT                 = (MYSPI_Datatype)10013;
static const MYSPI_Datatype MYSPI_DOUBLE                = (MYSPI_Datatype)10014;
static const MYSPI_Datatype MYSPI_LONG_DOUBLE           = (MYSPI_Datatype)10015;
static const MYSPI_Datatype MYSPI_INT8_T                = (MYSPI_Datatype)10016;
static const MYSPI_Datatype MYSPI_INT16_T               = (MYSPI_Datatype)10017;
static const MYSPI_Datatype MYSPI_INT32_T               = (MYSPI_Datatype)10018;
static const MYSPI_Datatype MYSPI_INT64_T               = (MYSPI_Datatype)10019;
static const MYSPI_Datatype MYSPI_UINT8_T               = (MYSPI_Datatype)10020;
static const MYSPI_Datatype MYSPI_UINT16_T              = (MYSPI_Datatype)10021;
static const MYSPI_Datatype MYSPI_UINT32_T              = (MYSPI_Datatype)10022;
static const MYSPI_Datatype MYSPI_UINT64_T              = (MYSPI_Datatype)10023;
static const MYSPI_Datatype MYSPI_C_BOOL                = (MYSPI_Datatype)10024;
static const MYSPI_Datatype MYSPI_C_COMPLEX             = (MYSPI_Datatype)10025;
static const MYSPI_Datatype MYSPI_C_DOUBLE_COMPLEX      = (MYSPI_Datatype)10026;
static const MYSPI_Datatype MYSPI_C_LONG_DOUBLE_COMPLEX = (MYSPI_Datatype)10027;

typedef unsigned long long int MYSPI_Op;
static const MYSPI_Op MYSPI_OP_NULL = (MYSPI_Op)0;
static const MYSPI_Op MYSPI_MAX     = (MYSPI_Op)20000;
static const MYSPI_Op MYSPI_MIN     = (MYSPI_Op)20001;
static const MYSPI_Op MYSPI_SUM     = (MYSPI_Op)20002;
static const MYSPI_Op MYSPI_PROD    = (MYSPI_Op)20003;
static const MYSPI_Op MYSPI_LAND    = (MYSPI_Op)20004;
static const MYSPI_Op MYSPI_BAND    = (MYSPI_Op)20005;
static const MYSPI_Op MYSPI_LOR     = (MYSPI_Op)20006;
static const MYSPI_Op MYSPI_BOR     = (MYSPI_Op)20007;
static const MYSPI_Op MYSPI_LXOR    = (MYSPI_Op)20008;
static const MYSPI_Op MYSPI_BXOR    = (MYSPI_Op)20009;
static const MYSPI_Op MYSPI_MINLOC  = (MYSPI_Op)20010;
static const MYSPI_Op MYSPI_MAXLOC  = (MYSPI_Op)20011;
static const MYSPI_Op MYSPI_REPLACE = (MYSPI_Op)20012;

typedef unsigned long long int MYSPI_Request;
static const MYSPI_Request MYSPI_REQUEST_NULL = (MYSPI_Request)0;

enum {
  MYSPI_THREAD_SINGLE,
  MYSPI_THREAD_FUNNELED,
  MYSPI_THREAD_SERIALIZED,
  MYSPI_THREAD_MULTIPLE
};

typedef unsigned long long int MYSPI_Comm;
static const MYSPI_Comm MYSPI_COMM_WORLD = (MYSPI_Comm)0;
static const MYSPI_Comm MYSPI_COMM_SELF = ~(MYSPI_Comm)0;

static int __SPI_initialized = 0;

MYS_API static int MYSPI_Initialized(int *inited) {
    *inited = __SPI_initialized;
    return MYSPI_SUCCESS;
}

MYS_API static int MYSPI_Init_thread(int *argc, char ***argv, int required, int *provided) {
    __SPI_initialized = 1;
    return MYSPI_SUCCESS;
}

MYS_API static int MYSPI_Init(int *argc, char ***argv) {
    int dummy = 0;
    return MYSPI_Init_thread(argc, argv, MYSPI_THREAD_SINGLE, &dummy);
}

MYS_API static int MYSPI_Comm_rank(MYSPI_Comm comm, int *myrank) {
    if (comm == MYSPI_COMM_WORLD || comm == MYSPI_COMM_SELF) {
        *myrank = 0;
        return MYSPI_SUCCESS;
    }
    return MYSPI_ERR_COMM;
}

MYS_API static int MYSPI_Comm_size(MYSPI_Comm comm, int *nranks) {
    if (comm == MYSPI_COMM_WORLD || comm == MYSPI_COMM_SELF) {
        *nranks = 1;
        return MYSPI_SUCCESS;
    }
    return MYSPI_ERR_COMM;
}

MYS_API static int MYSPI_Barrier(MYSPI_Comm comm) {
    if (comm == MYSPI_COMM_WORLD || comm == MYSPI_COMM_SELF) {
        return MYSPI_SUCCESS;
    }
    return MYSPI_ERR_COMM;
}

MYS_API static int MYSPI_Type_contiguous(int count, MYSPI_Datatype oldtype, MYSPI_Datatype *newtype) {
    return MYSPI_SUCCESS;
}

MYS_API static int MYSPI_Type_commit(MYSPI_Datatype *datatype) {
    return MYSPI_SUCCESS;
}

#else

#include <mpi.h>

#define MYSPI_VERSION    MPI_VERSION
#define MYSPI_SUBVERSION MPI_SUBVERSION

#define MYSPI_SUCCESS                   MPI_SUCCESS
#define MYSPI_ERR_BUFFER                MPI_ERR_BUFFER
#define MYSPI_ERR_COUNT                 MPI_ERR_COUNT
#define MYSPI_ERR_TYPE                  MPI_ERR_TYPE
#define MYSPI_ERR_TAG                   MPI_ERR_TAG
#define MYSPI_ERR_COMM                  MPI_ERR_COMM
#define MYSPI_ERR_RANK                  MPI_ERR_RANK
#define MYSPI_ERR_REQUEST               MPI_ERR_REQUEST
#define MYSPI_ERR_ROOT                  MPI_ERR_ROOT
#define MYSPI_ERR_GROUP                 MPI_ERR_GROUP
#define MYSPI_ERR_OP                    MPI_ERR_OP
#define MYSPI_ERR_TOPOLOGY              MPI_ERR_TOPOLOGY
#define MYSPI_ERR_DIMS                  MPI_ERR_DIMS
#define MYSPI_ERR_ARG                   MPI_ERR_ARG
#define MYSPI_ERR_UNKNOWN               MPI_ERR_UNKNOWN
#define MYSPI_ERR_TRUNCATE              MPI_ERR_TRUNCATE
#define MYSPI_ERR_OTHER                 MPI_ERR_OTHER
#define MYSPI_ERR_INTERN                MPI_ERR_INTERN
#define MYSPI_ERR_IN_STATUS             MPI_ERR_IN_STATUS
#define MYSPI_ERR_PENDING               MPI_ERR_PENDING
#define MYSPI_ERR_ACCESS                MPI_ERR_ACCESS
#define MYSPI_ERR_AMODE                 MPI_ERR_AMODE
#define MYSPI_ERR_ASSERT                MPI_ERR_ASSERT
#define MYSPI_ERR_BAD_FILE              MPI_ERR_BAD_FILE
#define MYSPI_ERR_BASE                  MPI_ERR_BASE
#define MYSPI_ERR_CONVERSION            MPI_ERR_CONVERSION
#define MYSPI_ERR_DISP                  MPI_ERR_DISP
#define MYSPI_ERR_DUP_DATAREP           MPI_ERR_DUP_DATAREP
#define MYSPI_ERR_FILE_EXISTS           MPI_ERR_FILE_EXISTS
#define MYSPI_ERR_FILE_IN_USE           MPI_ERR_FILE_IN_USE
#define MYSPI_ERR_FILE                  MPI_ERR_FILE
#define MYSPI_ERR_INFO_KEY              MPI_ERR_INFO_KEY
#define MYSPI_ERR_INFO_NOKEY            MPI_ERR_INFO_NOKEY
#define MYSPI_ERR_INFO_VALUE            MPI_ERR_INFO_VALUE
#define MYSPI_ERR_INFO                  MPI_ERR_INFO
#define MYSPI_ERR_IO                    MPI_ERR_IO
#define MYSPI_ERR_KEYVAL                MPI_ERR_KEYVAL
#define MYSPI_ERR_LOCKTYPE              MPI_ERR_LOCKTYPE
#define MYSPI_ERR_NAME                  MPI_ERR_NAME
#define MYSPI_ERR_NO_MEM                MPI_ERR_NO_MEM
#define MYSPI_ERR_NOT_SAME              MPI_ERR_NOT_SAME
#define MYSPI_ERR_NO_SPACE              MPI_ERR_NO_SPACE
#define MYSPI_ERR_NO_SUCH_FILE          MPI_ERR_NO_SUCH_FILE
#define MYSPI_ERR_PORT                  MPI_ERR_PORT
#define MYSPI_ERR_QUOTA                 MPI_ERR_QUOTA
#define MYSPI_ERR_READ_ONLY             MPI_ERR_READ_ONLY
#define MYSPI_ERR_RMA_CONFLICT          MPI_ERR_RMA_CONFLICT
#define MYSPI_ERR_RMA_SYNC              MPI_ERR_RMA_SYNC
#define MYSPI_ERR_SERVICE               MPI_ERR_SERVICE
#define MYSPI_ERR_SIZE                  MPI_ERR_SIZE
#define MYSPI_ERR_SPAWN                 MPI_ERR_SPAWN
#define MYSPI_ERR_UNSUPPORTED_DATAREP   MPI_ERR_UNSUPPORTED_DATAREP
#define MYSPI_ERR_UNSUPPORTED_OPERATION MPI_ERR_UNSUPPORTED_OPERATION
#define MYSPI_ERR_WIN                   MPI_ERR_WIN
#define MYSPI_T_ERR_MEMORY              MPI_T_ERR_MEMORY
#define MYSPI_T_ERR_NOT_INITIALIZED     MPI_T_ERR_NOT_INITIALIZED
#define MYSPI_T_ERR_CANNOT_INIT         MPI_T_ERR_CANNOT_INIT
#define MYSPI_T_ERR_INVALID_INDEX       MPI_T_ERR_INVALID_INDEX
#define MYSPI_T_ERR_INVALID_ITEM        MPI_T_ERR_INVALID_ITEM
#define MYSPI_T_ERR_INVALID_HANDLE      MPI_T_ERR_INVALID_HANDLE
#define MYSPI_T_ERR_OUT_OF_HANDLES      MPI_T_ERR_OUT_OF_HANDLES
#define MYSPI_T_ERR_OUT_OF_SESSIONS     MPI_T_ERR_OUT_OF_SESSIONS
#define MYSPI_T_ERR_INVALID_SESSION     MPI_T_ERR_INVALID_SESSION
#define MYSPI_T_ERR_CVAR_SET_NOT_NOW    MPI_T_ERR_CVAR_SET_NOT_NOW
#define MYSPI_T_ERR_CVAR_SET_NEVER      MPI_T_ERR_CVAR_SET_NEVER
#define MYSPI_T_ERR_PVAR_NO_STARTSTOP   MPI_T_ERR_PVAR_NO_STARTSTOP
#define MYSPI_T_ERR_PVAR_NO_WRITE       MPI_T_ERR_PVAR_NO_WRITE
#define MYSPI_T_ERR_PVAR_NO_ATOMIC      MPI_T_ERR_PVAR_NO_ATOMIC
#define MYSPI_ERR_RMA_RANGE             MPI_ERR_RMA_RANGE
#define MYSPI_ERR_RMA_ATTACH            MPI_ERR_RMA_ATTACH
#define MYSPI_ERR_RMA_FLAVOR            MPI_ERR_RMA_FLAVOR
#define MYSPI_ERR_RMA_SHARED            MPI_ERR_RMA_SHARED
#define MYSPI_T_ERR_INVALID             MPI_T_ERR_INVALID
#define MYSPI_T_ERR_INVALID_NAME        MPI_T_ERR_INVALID_NAME
#define MYSPI_ERR_LASTCODE              MPI_ERR_LASTCODE

#define MYSPI_Op      MPI_Op
#define MYSPI_OP_NULL MPI_OP_NULL
#define MYSPI_MAX     MPI_MAX
#define MYSPI_MIN     MPI_MIN
#define MYSPI_SUM     MPI_SUM
#define MYSPI_PROD    MPI_PROD
#define MYSPI_LAND    MPI_LAND
#define MYSPI_BAND    MPI_BAND
#define MYSPI_LOR     MPI_LOR
#define MYSPI_BOR     MPI_BOR
#define MYSPI_LXOR    MPI_LXOR
#define MYSPI_BXOR    MPI_BXOR
#define MYSPI_MINLOC  MPI_MINLOC
#define MYSPI_MAXLOC  MPI_MAXLOC
#define MYSPI_REPLACE MPI_REPLACE

#define MYSPI_Request      MPI_Request
#define MYSPI_REQUEST_NULL MPI_REQUEST_NULL

#define MYSPI_THREAD_SINGLE     MPI_THREAD_SINGLE
#define MYSPI_THREAD_FUNNELED   MPI_THREAD_FUNNELED
#define MYSPI_THREAD_SERIALIZED MPI_THREAD_SERIALIZED
#define MYSPI_THREAD_MULTIPLE   MPI_THREAD_MULTIPLE

#define MYSPI_Comm       MPI_Comm
#define MYSPI_COMM_WORLD MPI_COMM_WORLD
#define MYSPI_COMM_SELF  MPI_COMM_SELF

#define MYSPI_Initialized     MPI_Initialized
#define MYSPI_Init_thread     MPI_Init_thread
#define MYSPI_Init            MPI_Init
#define MYSPI_Comm_rank       MPI_Comm_rank
#define MYSPI_Comm_size       MPI_Comm_size
#define MYSPI_Barrier         MPI_Barrier
#define MYSPI_Type_contiguous MPI_Type_contiguous
#define MYSPI_Type_commit     MPI_Type_commit

#endif /*defined(MYS_NO_MPI)*/