#if !defined(MYS_NO_MPI)
#include <mpi.h>
#else
#if !defined(__MYS_REPLACED_MPI__)
#define __MYS_REPLACED_MPI__

/*********************************************/
// SPI
/*********************************************/

#define SPI_VERSION 3
#define SPI_SUBVERSION 1
/*
 * Error classes and codes
 * Do not change the values of these without also modifying mpif.h.in.
 */
#define SPI_SUCCESS                   0
#define SPI_ERR_BUFFER                1
#define SPI_ERR_COUNT                 2
#define SPI_ERR_TYPE                  3
#define SPI_ERR_TAG                   4
#define SPI_ERR_COMM                  5
#define SPI_ERR_RANK                  6
#define SPI_ERR_REQUEST               7
#define SPI_ERR_ROOT                  8
#define SPI_ERR_GROUP                 9
#define SPI_ERR_OP                    10
#define SPI_ERR_TOPOLOGY              11
#define SPI_ERR_DIMS                  12
#define SPI_ERR_ARG                   13
#define SPI_ERR_UNKNOWN               14
#define SPI_ERR_TRUNCATE              15
#define SPI_ERR_OTHER                 16
#define SPI_ERR_INTERN                17
#define SPI_ERR_IN_STATUS             18
#define SPI_ERR_PENDING               19
#define SPI_ERR_ACCESS                20
#define SPI_ERR_AMODE                 21
#define SPI_ERR_ASSERT                22
#define SPI_ERR_BAD_FILE              23
#define SPI_ERR_BASE                  24
#define SPI_ERR_CONVERSION            25
#define SPI_ERR_DISP                  26
#define SPI_ERR_DUP_DATAREP           27
#define SPI_ERR_FILE_EXISTS           28
#define SPI_ERR_FILE_IN_USE           29
#define SPI_ERR_FILE                  30
#define SPI_ERR_INFO_KEY              31
#define SPI_ERR_INFO_NOKEY            32
#define SPI_ERR_INFO_VALUE            33
#define SPI_ERR_INFO                  34
#define SPI_ERR_IO                    35
#define SPI_ERR_KEYVAL                36
#define SPI_ERR_LOCKTYPE              37
#define SPI_ERR_NAME                  38
#define SPI_ERR_NO_MEM                39
#define SPI_ERR_NOT_SAME              40
#define SPI_ERR_NO_SPACE              41
#define SPI_ERR_NO_SUCH_FILE          42
#define SPI_ERR_PORT                  43
#define SPI_ERR_QUOTA                 44
#define SPI_ERR_READ_ONLY             45
#define SPI_ERR_RMA_CONFLICT          46
#define SPI_ERR_RMA_SYNC              47
#define SPI_ERR_SERVICE               48
#define SPI_ERR_SIZE                  49
#define SPI_ERR_SPAWN                 50
#define SPI_ERR_UNSUPPORTED_DATAREP   51
#define SPI_ERR_UNSUPPORTED_OPERATION 52
#define SPI_ERR_WIN                   53
#define SPI_T_ERR_MEMORY              54
#define SPI_T_ERR_NOT_INITIALIZED     55
#define SPI_T_ERR_CANNOT_INIT         56
#define SPI_T_ERR_INVALID_INDEX       57
#define SPI_T_ERR_INVALID_ITEM        58
#define SPI_T_ERR_INVALID_HANDLE      59
#define SPI_T_ERR_OUT_OF_HANDLES      60
#define SPI_T_ERR_OUT_OF_SESSIONS     61
#define SPI_T_ERR_INVALID_SESSION     62
#define SPI_T_ERR_CVAR_SET_NOT_NOW    63
#define SPI_T_ERR_CVAR_SET_NEVER      64
#define SPI_T_ERR_PVAR_NO_STARTSTOP   65
#define SPI_T_ERR_PVAR_NO_WRITE       66
#define SPI_T_ERR_PVAR_NO_ATOMIC      67
#define SPI_ERR_RMA_RANGE             68
#define SPI_ERR_RMA_ATTACH            69
#define SPI_ERR_RMA_FLAVOR            70
#define SPI_ERR_RMA_SHARED            71
#define SPI_T_ERR_INVALID             72
#define SPI_T_ERR_INVALID_NAME        73
#define SPI_ERR_LASTCODE              92
/* Per MPI-3 p349 47, MPI_ERR_LASTCODE must be >= the last predefined
   MPI_ERR_<foo> code. Set the last code to allow some room for adding
   error codes without breaking ABI. */

typedef unsigned long long int SPI_Datatype;
static const SPI_Datatype SPI_DATATYPE_NULL         = (SPI_Datatype)0;
static const SPI_Datatype SPI_BYTE                  = (SPI_Datatype)10000;
static const SPI_Datatype SPI_CHAR                  = (SPI_Datatype)10001;
static const SPI_Datatype SPI_SIGNED_CHAR           = (SPI_Datatype)10002;
static const SPI_Datatype SPI_UNSIGNED_CHAR         = (SPI_Datatype)10003;
static const SPI_Datatype SPI_WCHAR                 = (SPI_Datatype)10004;
static const SPI_Datatype SPI_SHORT                 = (SPI_Datatype)10005;
static const SPI_Datatype SPI_UNSIGNED_SHORT        = (SPI_Datatype)10006;
static const SPI_Datatype SPI_INT                   = (SPI_Datatype)10007;
static const SPI_Datatype SPI_UNSIGNED              = (SPI_Datatype)10008;
static const SPI_Datatype SPI_LONG                  = (SPI_Datatype)10009;
static const SPI_Datatype SPI_UNSIGNED_LONG         = (SPI_Datatype)10010;
static const SPI_Datatype SPI_LONG_LONG             = (SPI_Datatype)10011;
static const SPI_Datatype SPI_UNSIGNED_LONG_LONG    = (SPI_Datatype)10012;
static const SPI_Datatype SPI_FLOAT                 = (SPI_Datatype)10013;
static const SPI_Datatype SPI_DOUBLE                = (SPI_Datatype)10014;
static const SPI_Datatype SPI_LONG_DOUBLE           = (SPI_Datatype)10015;
static const SPI_Datatype SPI_INT8_T                = (SPI_Datatype)10016;
static const SPI_Datatype SPI_INT16_T               = (SPI_Datatype)10017;
static const SPI_Datatype SPI_INT32_T               = (SPI_Datatype)10018;
static const SPI_Datatype SPI_INT64_T               = (SPI_Datatype)10019;
static const SPI_Datatype SPI_UINT8_T               = (SPI_Datatype)10020;
static const SPI_Datatype SPI_UINT16_T              = (SPI_Datatype)10021;
static const SPI_Datatype SPI_UINT32_T              = (SPI_Datatype)10022;
static const SPI_Datatype SPI_UINT64_T              = (SPI_Datatype)10023;
static const SPI_Datatype SPI_C_BOOL                = (SPI_Datatype)10024;
static const SPI_Datatype SPI_C_COMPLEX             = (SPI_Datatype)10025;
static const SPI_Datatype SPI_C_DOUBLE_COMPLEX      = (SPI_Datatype)10026;
static const SPI_Datatype SPI_C_LONG_DOUBLE_COMPLEX = (SPI_Datatype)10027;

typedef unsigned long long int SPI_Op;
static const SPI_Op SPI_OP_NULL = (SPI_Op)0;
static const SPI_Op SPI_MAX     = (SPI_Op)20000;
static const SPI_Op SPI_MIN     = (SPI_Op)20001;
static const SPI_Op SPI_SUM     = (SPI_Op)20002;
static const SPI_Op SPI_PROD    = (SPI_Op)20003;
static const SPI_Op SPI_LAND    = (SPI_Op)20004;
static const SPI_Op SPI_BAND    = (SPI_Op)20005;
static const SPI_Op SPI_LOR     = (SPI_Op)20006;
static const SPI_Op SPI_BOR     = (SPI_Op)20007;
static const SPI_Op SPI_LXOR    = (SPI_Op)20008;
static const SPI_Op SPI_BXOR    = (SPI_Op)20009;
static const SPI_Op SPI_MINLOC  = (SPI_Op)20010;
static const SPI_Op SPI_MAXLOC  = (SPI_Op)20011;
static const SPI_Op SPI_REPLACE = (SPI_Op)20012;

typedef unsigned long long int SPI_Request;
static const SPI_Request SPI_REQUEST_NULL = (SPI_Request)0;

enum {
  SPI_THREAD_SINGLE,
  SPI_THREAD_FUNNELED,
  SPI_THREAD_SERIALIZED,
  SPI_THREAD_MULTIPLE
};

typedef unsigned long long int SPI_Comm;
static const SPI_Comm SPI_COMM_WORLD = (SPI_Comm)0;
static const SPI_Comm SPI_COMM_SELF = ~(SPI_Comm)0;

static int __SPI_initialized = 0;

int SPI_Initialized(int *inited) {
    *inited = __SPI_initialized;
    return SPI_SUCCESS;
}

int SPI_Init_thread(int *argc, char ***argv, int required, int *provided) {
    __SPI_initialized = 1;
    return SPI_SUCCESS;
}

int SPI_Init(int *argc, char ***argv) {
    int dummy = 0;
    return SPI_Init_thread(argc, argv, SPI_THREAD_SINGLE, &dummy);
}

int SPI_Comm_rank(SPI_Comm comm, int *myrank) {
    if (comm == SPI_COMM_WORLD || comm == SPI_COMM_SELF) {
        *myrank = 0;
        return SPI_SUCCESS;
    }
    return SPI_ERR_COMM;
}

int SPI_Comm_size(SPI_Comm comm, int *nranks) {
    if (comm == SPI_COMM_WORLD || comm == SPI_COMM_SELF) {
        *nranks = 1;
        return SPI_SUCCESS;
    }
    return SPI_ERR_COMM;
}

int SPI_Barrier(SPI_Comm comm) {
    if (comm == SPI_COMM_WORLD || comm == SPI_COMM_SELF) {
        return SPI_SUCCESS;
    }
    return SPI_ERR_COMM;
}

int SPI_Barrier(SPI_Comm comm) {
    if (comm == SPI_COMM_WORLD || comm == SPI_COMM_SELF) {
        return SPI_SUCCESS;
    }
    return SPI_ERR_COMM;
}

int SPI_Type_contiguous(int count, MPI_Datatype oldtype, MPI_Datatype *newtype) {
    return SPI_SUCCESS;
}

int SPI_Type_commit(MPI_Datatype *datatype) {
    return SPI_SUCCESS;
}

/*********************************************/
// MPI replacement
/*********************************************/

#define MPI_VERSION    SPI_VERSION
#define MPI_SUBVERSION SPI_SUBVERSION

#define MPI_SUCCESS                   SPI_SUCCESS
#define MPI_ERR_BUFFER                SPI_ERR_BUFFER
#define MPI_ERR_COUNT                 SPI_ERR_COUNT
#define MPI_ERR_TYPE                  SPI_ERR_TYPE
#define MPI_ERR_TAG                   SPI_ERR_TAG
#define MPI_ERR_COMM                  SPI_ERR_COMM
#define MPI_ERR_RANK                  SPI_ERR_RANK
#define MPI_ERR_REQUEST               SPI_ERR_REQUEST
#define MPI_ERR_ROOT                  SPI_ERR_ROOT
#define MPI_ERR_GROUP                 SPI_ERR_GROUP
#define MPI_ERR_OP                    SPI_ERR_OP
#define MPI_ERR_TOPOLOGY              SPI_ERR_TOPOLOGY
#define MPI_ERR_DIMS                  SPI_ERR_DIMS
#define MPI_ERR_ARG                   SPI_ERR_ARG
#define MPI_ERR_UNKNOWN               SPI_ERR_UNKNOWN
#define MPI_ERR_TRUNCATE              SPI_ERR_TRUNCATE
#define MPI_ERR_OTHER                 SPI_ERR_OTHER
#define MPI_ERR_INTERN                SPI_ERR_INTERN
#define MPI_ERR_IN_STATUS             SPI_ERR_IN_STATUS
#define MPI_ERR_PENDING               SPI_ERR_PENDING
#define MPI_ERR_ACCESS                SPI_ERR_ACCESS
#define MPI_ERR_AMODE                 SPI_ERR_AMODE
#define MPI_ERR_ASSERT                SPI_ERR_ASSERT
#define MPI_ERR_BAD_FILE              SPI_ERR_BAD_FILE
#define MPI_ERR_BASE                  SPI_ERR_BASE
#define MPI_ERR_CONVERSION            SPI_ERR_CONVERSION
#define MPI_ERR_DISP                  SPI_ERR_DISP
#define MPI_ERR_DUP_DATAREP           SPI_ERR_DUP_DATAREP
#define MPI_ERR_FILE_EXISTS           SPI_ERR_FILE_EXISTS
#define MPI_ERR_FILE_IN_USE           SPI_ERR_FILE_IN_USE
#define MPI_ERR_FILE                  SPI_ERR_FILE
#define MPI_ERR_INFO_KEY              SPI_ERR_INFO_KEY
#define MPI_ERR_INFO_NOKEY            SPI_ERR_INFO_NOKEY
#define MPI_ERR_INFO_VALUE            SPI_ERR_INFO_VALUE
#define MPI_ERR_INFO                  SPI_ERR_INFO
#define MPI_ERR_IO                    SPI_ERR_IO
#define MPI_ERR_KEYVAL                SPI_ERR_KEYVAL
#define MPI_ERR_LOCKTYPE              SPI_ERR_LOCKTYPE
#define MPI_ERR_NAME                  SPI_ERR_NAME
#define MPI_ERR_NO_MEM                SPI_ERR_NO_MEM
#define MPI_ERR_NOT_SAME              SPI_ERR_NOT_SAME
#define MPI_ERR_NO_SPACE              SPI_ERR_NO_SPACE
#define MPI_ERR_NO_SUCH_FILE          SPI_ERR_NO_SUCH_FILE
#define MPI_ERR_PORT                  SPI_ERR_PORT
#define MPI_ERR_QUOTA                 SPI_ERR_QUOTA
#define MPI_ERR_READ_ONLY             SPI_ERR_READ_ONLY
#define MPI_ERR_RMA_CONFLICT          SPI_ERR_RMA_CONFLICT
#define MPI_ERR_RMA_SYNC              SPI_ERR_RMA_SYNC
#define MPI_ERR_SERVICE               SPI_ERR_SERVICE
#define MPI_ERR_SIZE                  SPI_ERR_SIZE
#define MPI_ERR_SPAWN                 SPI_ERR_SPAWN
#define MPI_ERR_UNSUPPORTED_DATAREP   SPI_ERR_UNSUPPORTED_DATAREP
#define MPI_ERR_UNSUPPORTED_OPERATION SPI_ERR_UNSUPPORTED_OPERATION
#define MPI_ERR_WIN                   SPI_ERR_WIN
#define MPI_T_ERR_MEMORY              SPI_T_ERR_MEMORY
#define MPI_T_ERR_NOT_INITIALIZED     SPI_T_ERR_NOT_INITIALIZED
#define MPI_T_ERR_CANNOT_INIT         SPI_T_ERR_CANNOT_INIT
#define MPI_T_ERR_INVALID_INDEX       SPI_T_ERR_INVALID_INDEX
#define MPI_T_ERR_INVALID_ITEM        SPI_T_ERR_INVALID_ITEM
#define MPI_T_ERR_INVALID_HANDLE      SPI_T_ERR_INVALID_HANDLE
#define MPI_T_ERR_OUT_OF_HANDLES      SPI_T_ERR_OUT_OF_HANDLES
#define MPI_T_ERR_OUT_OF_SESSIONS     SPI_T_ERR_OUT_OF_SESSIONS
#define MPI_T_ERR_INVALID_SESSION     SPI_T_ERR_INVALID_SESSION
#define MPI_T_ERR_CVAR_SET_NOT_NOW    SPI_T_ERR_CVAR_SET_NOT_NOW
#define MPI_T_ERR_CVAR_SET_NEVER      SPI_T_ERR_CVAR_SET_NEVER
#define MPI_T_ERR_PVAR_NO_STARTSTOP   SPI_T_ERR_PVAR_NO_STARTSTOP
#define MPI_T_ERR_PVAR_NO_WRITE       SPI_T_ERR_PVAR_NO_WRITE
#define MPI_T_ERR_PVAR_NO_ATOMIC      SPI_T_ERR_PVAR_NO_ATOMIC
#define MPI_ERR_RMA_RANGE             SPI_ERR_RMA_RANGE
#define MPI_ERR_RMA_ATTACH            SPI_ERR_RMA_ATTACH
#define MPI_ERR_RMA_FLAVOR            SPI_ERR_RMA_FLAVOR
#define MPI_ERR_RMA_SHARED            SPI_ERR_RMA_SHARED
#define MPI_T_ERR_INVALID             SPI_T_ERR_INVALID
#define MPI_T_ERR_INVALID_NAME        SPI_T_ERR_INVALID_NAME
#define MPI_ERR_LASTCODE              SPI_ERR_LASTCODE

#define MPI_Op      SPI_Op
#define MPI_OP_NULL SPI_OP_NULL
#define MPI_MAX     SPI_MAX
#define MPI_MIN     SPI_MIN
#define MPI_SUM     SPI_SUM
#define MPI_PROD    SPI_PROD
#define MPI_LAND    SPI_LAND
#define MPI_BAND    SPI_BAND
#define MPI_LOR     SPI_LOR
#define MPI_BOR     SPI_BOR
#define MPI_LXOR    SPI_LXOR
#define MPI_BXOR    SPI_BXOR
#define MPI_MINLOC  SPI_MINLOC
#define MPI_MAXLOC  SPI_MAXLOC
#define MPI_REPLACE SPI_REPLACE

#define MPI_Request      SPI_Request
#define MPI_REQUEST_NULL SPI_REQUEST_NULL

#define MPI_THREAD_SINGLE     SPI_THREAD_SINGLE
#define MPI_THREAD_FUNNELED   SPI_THREAD_FUNNELED
#define MPI_THREAD_SERIALIZED SPI_THREAD_SERIALIZED
#define MPI_THREAD_MULTIPLE   SPI_THREAD_MULTIPLE

#define MPI_Comm       SPI_Comm
#define MPI_COMM_WORLD SPI_COMM_WORLD
#define MPI_COMM_SELF  SPI_COMM_SELF

#define MPI_Initialized     SPI_Initialized
#define MPI_Init_thread     SPI_Init_thread
#define MPI_Init            SPI_Init
#define MPI_Comm_rank       SPI_Comm_rank
#define MPI_Comm_size       SPI_Comm_size
#define MPI_Barrier         SPI_Barrier
#define MPI_Type_contiguous SPI_Type_contiguous
#define MPI_Type_commit     SPI_Type_commit

#endif /*__MYS_REPLACED_MPI__*/

#endif /*!defined(MYS_NO_MPI)*/
