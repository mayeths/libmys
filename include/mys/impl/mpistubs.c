/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../assert.h"
// #include "uthash_hash.h"

#ifdef MYS_NO_MPI
/**************************************************/
/* MPI stubs to generate serial codes without mpi */
/**************************************************/

// struct mys_MPI_Request_key {
//     int count;
//     mys_MPI_Datatype datatype;
//     int tag;
// };

// typedef struct mys_MPI_Request_struct {
//     mys_MPI_Request_key key;
//     void *src;
//     void *dst;
//     _mys_UT_hash_handle hh;
// } mys_MPI_Request_struct;

// struct _mys_mpistubs_G {
//     bool inited;
//     mys_MPI_Request_struct *unresolved_requests;
// };
// static struct _mys_mpistubs_G _mys_mpistubs_g;

// static void _mys_mpistubs_init()
// {
//     if (_mys_mpistubs_g.inited) return;
//     _mys_mpistubs_g.unresolved_requests = NULL;
//     _mys_mpistubs_g.inited = true;
// }

#include <string.h>
#include <time.h>
#ifdef POSIX_COMPLIANCE
#include <sys/time.h>
#elif defined(OS_WINDOWS)
#include <windows.h>
#endif

MYS_PUBLIC int mys_MPI_Initialized(int *flag)
{
    *flag = 1;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Init(int *argc, char ***argv)
{
    (void)argc;
    (void)argv;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    (void)argc;
    (void)argv;
    (void)required;
    *provided = mys_MPI_THREAD_SINGLE;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Finalize()
{
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Abort(mys_MPI_Comm comm, int errorcode)
{
    (void)comm;
    exit(errorcode);
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Comm_rank(mys_MPI_Comm comm, int *rank)
{
    (void)comm;
    *rank = 0;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Comm_size(mys_MPI_Comm comm, int *size)
{
    (void)comm;
    *size = 1;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Comm_split(mys_MPI_Comm comm, int n, int m, mys_MPI_Comm *comms)
{
    (void)comm;
    (void)n;
    (void)m;
    *comms = comm;
   return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Comm_split_type(mys_MPI_Comm comm, int split_type, int key, mys_MPI_Info info, mys_MPI_Comm *newcomm)
{
    (void)comm;
    (void)split_type;
    (void)key;
    (void)info;
    *newcomm = comm;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Comm_dup(mys_MPI_Comm comm, mys_MPI_Comm *newcomm)
{
    *newcomm = comm;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Comm_free(mys_MPI_Comm *comm)
{
    (void)comm;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Probe(int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status)
{
    (void)source;
    (void)tag;
    (void)comm;
    (void)status;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Get_count(mys_MPI_Status *status, mys_MPI_Datatype datatype, int *count)
{
    (void)status;
    (void)datatype;
    (void)count;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Recv(void *buf, int count, mys_MPI_Datatype datatype, int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status)
{
    (void)buf;
    (void)count;
    (void)datatype;
    (void)source;
    (void)tag;
    (void)comm;
    (void)status;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Send(const void *buf, int count, mys_MPI_Datatype datatype, int dest, int tag, mys_MPI_Comm comm)
{
    (void)buf;
    (void)count;
    (void)datatype;
    (void)dest;
    (void)tag;
    (void)comm;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Irecv(void *buf, int count, mys_MPI_Datatype datatype, int source, int tag, mys_MPI_Comm comm, mys_MPI_Request *request)
{
    // _mys_mpistubs_init();
    (void)buf;
    (void)count;
    (void)datatype;
    (void)source;
    (void)tag;
    (void)comm;
    (void)request;
    // mys_MPI_Request_key key;
    // key.count = count;
    // key.datatype = datatype;
    // key.tag = tag;
    // mys_MPI_Request_struct *old_request;
    // _HASH_FIND(hh, _mys_mpistubs_g.unresolved_requests, &key, sizeof(mys_MPI_Request_key), old_request);
    // if (old_request != NULL) {
    //     old_request->dst = buf;
    //     return mys_MPI_SUCCESS;
    // }

    // mys_MPI_Request_struct *new_request = (mys_MPI_Request_struct *)malloc(sizeof(mys_MPI_Request_struct));
    // new_request->key = key;
    // new_request->src = NULL;
    // new_request->dst = buf;
    // _HASH_ADD(hh, _mys_mpistubs_g.unresolved_requests, key, sizeof(mys_MPI_Request_key), new_request);
    // *request = new_request;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Isend(const void *buf, int count, mys_MPI_Datatype datatype, int dest, int tag, mys_MPI_Comm comm, mys_MPI_Request *request)
{
    // _mys_mpistubs_init();
    (void)buf;
    (void)count;
    (void)datatype;
    (void)dest;
    (void)tag;
    (void)comm;
    (void)request;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Waitall(int count, mys_MPI_Request *array_of_requests, mys_MPI_Status *array_of_statuses)
{
    (void)count;
    (void)array_of_requests;
    (void)array_of_statuses;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Barrier(mys_MPI_Comm comm)
{
    (void)comm;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Bcast(void *buffer, int count, mys_MPI_Datatype datatype, int root, mys_MPI_Comm comm)
{
    (void)buffer;
    (void)count;
    (void)datatype;
    (void)root;
    (void)comm;
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Gather(void *sendbuf, int sendcount, mys_MPI_Datatype sendtype, void *recvbuf, int recvcount, mys_MPI_Datatype recvtype, int root, mys_MPI_Comm comm)
{
    (void)root;
    return mys_MPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}

MYS_PUBLIC int mys_MPI_Allreduce(void *sendbuf, void *recvbuf, int count, mys_MPI_Datatype datatype, mys_MPI_Op op, mys_MPI_Comm comm)
{
   // FIXME: support MPI_MAX, MPI_MIN and other op. Throw error if not implemented
    (void)op;
    (void)comm;
    int i;

    if (sendbuf == mys_MPI_IN_PLACE)
        sendbuf = recvbuf;

    switch (datatype)
    {
        case mys_MPI_INT:
        {
            int *crecvbuf = (int *)recvbuf;
            int *csendbuf = (int *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_LONG_LONG_INT:
        {
            long long int *crecvbuf = (long long int *)recvbuf;
            long long int *csendbuf = (long long int *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_FLOAT:
        {
            float *crecvbuf = (float *)recvbuf;
            float *csendbuf = (float *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_DOUBLE:
        {
            double *crecvbuf = (double *)recvbuf;
            double *csendbuf = (double *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_LONG_DOUBLE:
        {
            long double *crecvbuf = (long double *)recvbuf;
            long double *csendbuf = (long double *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_CHAR:
        {
            char *crecvbuf = (char *)recvbuf;
            char *csendbuf = (char *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_LONG:
        {
            long *crecvbuf = (long *)recvbuf;
            long *csendbuf = (long *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_BYTE:
        {
            memcpy(recvbuf, sendbuf, count);
        }
        break;

        case mys_MPI_DOUBLE_INT:
        {
            struct _double_int_t { double d; int i; };
            struct _double_int_t *crecvbuf = (struct _double_int_t *)recvbuf;
            struct _double_int_t *csendbuf = (struct _double_int_t *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_INT32_T:
        {
            int32_t *crecvbuf = (int32_t *)recvbuf;
            int32_t *csendbuf = (int32_t *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_INT64_T:
        {
            int64_t *crecvbuf = (int64_t *)recvbuf;
            int64_t *csendbuf = (int64_t *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_UINT32_T:
        {
            uint32_t *crecvbuf = (uint32_t *)recvbuf;
            uint32_t *csendbuf = (uint32_t *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_UINT64_T:
        {
            uint64_t *crecvbuf = (uint64_t *)recvbuf;
            uint64_t *csendbuf = (uint64_t *)sendbuf;
            for (i = 0; i < count; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        default:
        {
            THROW_NOT_IMPL();
        }

        break;

    }
    return mys_MPI_SUCCESS;
}

MYS_PUBLIC int mys_MPI_Allgather(void *sendbuf, int sendcount, mys_MPI_Datatype sendtype, void *recvbuf, int recvcount, mys_MPI_Datatype recvtype, mys_MPI_Comm comm)
{
    (void)comm;
    (void)recvcount;
    int i;

    if (sendtype != recvtype)
        THROW_NOT_IMPL();

    if (sendbuf == mys_MPI_IN_PLACE)
        sendbuf = recvbuf;

    switch (sendtype)
    {
        case mys_MPI_INT:
        {
            int *crecvbuf = (int *)recvbuf;
            int *csendbuf = (int *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_LONG_LONG_INT:
        {
            long long int *crecvbuf = (long long int *)recvbuf;
            long long int *csendbuf = (long long int *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_FLOAT:
        {
            float *crecvbuf = (float *)recvbuf;
            float *csendbuf = (float *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_DOUBLE:
        {
            double *crecvbuf = (double *)recvbuf;
            double *csendbuf = (double *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_LONG_DOUBLE:
        {
            long double *crecvbuf = (long double *)recvbuf;
            long double *csendbuf = (long double *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_CHAR:
        {
            char *crecvbuf = (char *)recvbuf;
            char *csendbuf = (char *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_LONG:
        {
            long *crecvbuf = (long *)recvbuf;
            long *csendbuf = (long *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_BYTE:
        {
            memcpy(recvbuf, sendbuf, sendcount);
        }
        break;

        case mys_MPI_DOUBLE_INT:
        {
            struct _double_int_t { double d; int i; };
            struct _double_int_t *crecvbuf = (struct _double_int_t *)recvbuf;
            struct _double_int_t *csendbuf = (struct _double_int_t *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_INT32_T:
        {
            int32_t *crecvbuf = (int32_t *)recvbuf;
            int32_t *csendbuf = (int32_t *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_INT64_T:
        {
            int64_t *crecvbuf = (int64_t *)recvbuf;
            int64_t *csendbuf = (int64_t *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_UINT32_T:
        {
            uint32_t *crecvbuf = (uint32_t *)recvbuf;
            uint32_t *csendbuf = (uint32_t *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        case mys_MPI_UINT64_T:
        {
            uint64_t *crecvbuf = (uint64_t *)recvbuf;
            uint64_t *csendbuf = (uint64_t *)sendbuf;
            for (i = 0; i < sendcount; i++)
            {
                crecvbuf[i] = csendbuf[i];
            }
        }
        break;

        default:
        {
            THROW_NOT_IMPL();
        }
    }

   return mys_MPI_SUCCESS;
}

MYS_PUBLIC double mys_MPI_Wtime()
{
#ifdef POSIX_COMPLIANCE
#if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
#endif
#elif defined(OS_WINDOWS)
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / frequency.QuadPart;
#else
   clock_t t = clock();
   return (double)t / (double)CLOCKS_PER_SEC;
#endif
}

#else // MYS_NO_MPI
/*************************************************/
/* MPI stubs to generate parallel codes with mpi */
/*************************************************/

MYS_PUBLIC int mys_MPI_Initialized(int *flag)
{
#ifdef MYS_USE_PMPI
    return PMPI_Initialized(flag);
#else
    return MPI_Initialized(flag);
#endif
}

MYS_PUBLIC int mys_MPI_Init(int *argc, char ***argv)
{
#ifdef MYS_USE_PMPI
    return PMPI_Init(argc, argv);
#else
    return MPI_Init(argc, argv);
#endif
}

MYS_PUBLIC int mys_MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
#ifdef MYS_USE_PMPI
    return PMPI_Init_thread(argc, argv, required, provided);
#else
    return MPI_Init_thread(argc, argv, required, provided);
#endif
}

MYS_PUBLIC int mys_MPI_Finalize()
{
#ifdef MYS_USE_PMPI
    return PMPI_Finalize();
#else
    return MPI_Finalize();
#endif
}

MYS_PUBLIC int mys_MPI_Abort(mys_MPI_Comm comm, int errorcode)
{
#ifdef MYS_USE_PMPI
    return PMPI_Abort(comm, errorcode);
#else
    return MPI_Abort(comm, errorcode);
#endif
}

MYS_PUBLIC int mys_MPI_Comm_rank(mys_MPI_Comm comm, int *rank)
{
#ifdef MYS_USE_PMPI
    return PMPI_Comm_rank(comm, rank);
#else
    return MPI_Comm_rank(comm, rank);
#endif
}

MYS_PUBLIC int mys_MPI_Comm_size(mys_MPI_Comm comm, int *size)
{
#ifdef MYS_USE_PMPI
    return PMPI_Comm_size(comm, size);
#else
    return MPI_Comm_size(comm, size);
#endif
}

MYS_PUBLIC int mys_MPI_Comm_split(mys_MPI_Comm comm, int n, int m, mys_MPI_Comm *comms)
{
#ifdef MYS_USE_PMPI
    return PMPI_Comm_split(comm, n, m, comms);
#else
    return MPI_Comm_split(comm, n, m, comms);
#endif
}

MYS_PUBLIC int mys_MPI_Comm_split_type(mys_MPI_Comm comm, int split_type, int key, MPI_Info info, mys_MPI_Comm *newcomm)
{
#ifdef MYS_USE_PMPI
    return PMPI_Comm_split_type(comm, split_type, key, info, newcomm);
#else
    return MPI_Comm_split_type(comm, split_type, key, info, newcomm);
#endif
}

MYS_PUBLIC int mys_MPI_Comm_dup(mys_MPI_Comm comm, mys_MPI_Comm *newcomm)
{
#ifdef MYS_USE_PMPI
    return PMPI_Comm_dup(comm, newcomm);
#else
    return MPI_Comm_dup(comm, newcomm);
#endif
}

MYS_PUBLIC int mys_MPI_Comm_free(mys_MPI_Comm *comm)
{
#ifdef MYS_USE_PMPI
   return PMPI_Comm_free(comm);
#else
   return MPI_Comm_free(comm);
#endif
}

MYS_PUBLIC int mys_MPI_Probe(int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status)
{
#ifdef MYS_USE_PMPI
    return PMPI_Probe(source, tag, comm, status);
#else
    return MPI_Probe(source, tag, comm, status);
#endif
}

MYS_PUBLIC int mys_MPI_Get_count(mys_MPI_Status *status, mys_MPI_Datatype datatype, int *count)
{
#ifdef MYS_USE_PMPI
    return PMPI_Get_count(status, datatype, count);
#else
    return MPI_Get_count(status, datatype, count);
#endif
}

MYS_PUBLIC int mys_MPI_Recv(void *buf, int count, mys_MPI_Datatype datatype, int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status)
{
#ifdef MYS_USE_PMPI
    return PMPI_Recv(buf, count, datatype, source, tag, comm, status);
#else
    return MPI_Recv(buf, count, datatype, source, tag, comm, status);
#endif
}

MYS_PUBLIC int mys_MPI_Send(const void *buf, int count, mys_MPI_Datatype datatype, int dest, int tag, mys_MPI_Comm comm)
{
#ifdef MYS_USE_PMPI
    return PMPI_Send(buf, count, datatype, dest, tag, comm);
#else
    return MPI_Send(buf, count, datatype, dest, tag, comm);
#endif
}

MYS_PUBLIC int mys_MPI_Irecv(void *buf, int count, mys_MPI_Datatype datatype, int source, int tag, mys_MPI_Comm comm, mys_MPI_Request *request)
{
#ifdef MYS_USE_PMPI
    return PMPI_Irecv(buf, count, datatype, source, tag, comm, request);
#else
    return MPI_Irecv(buf, count, datatype, source, tag, comm, request);
#endif
}

MYS_PUBLIC int mys_MPI_Isend(const void *buf, int count, mys_MPI_Datatype datatype, int dest, int tag, mys_MPI_Comm comm, mys_MPI_Request *request)
{
#ifdef MYS_USE_PMPI
    return PMPI_Isend(buf, count, datatype, dest, tag, comm, request);
#else
    return MPI_Isend(buf, count, datatype, dest, tag, comm, request);
#endif
}

MYS_PUBLIC int mys_MPI_Waitall(int count, mys_MPI_Request *array_of_requests, mys_MPI_Status *array_of_statuses)
{
#ifdef MYS_USE_PMPI
    return PMPI_Waitall(count, array_of_requests, array_of_statuses);
#else
    return MPI_Waitall(count, array_of_requests, array_of_statuses);
#endif
}

MYS_PUBLIC int mys_MPI_Barrier(mys_MPI_Comm comm)
{
#ifdef MYS_USE_PMPI
    return PMPI_Barrier(comm);
#else
    return MPI_Barrier(comm);
#endif
}

MYS_PUBLIC int mys_MPI_Bcast(void *buffer, int count, mys_MPI_Datatype datatype, int root, mys_MPI_Comm comm)
{
#ifdef MYS_USE_PMPI
    return PMPI_Bcast(buffer, count, datatype, root, comm);
#else
    return MPI_Bcast(buffer, count, datatype, root, comm);
#endif
}

MYS_PUBLIC int mys_MPI_Gather(void *sendbuf, int sendcount, mys_MPI_Datatype sendtype, void *recvbuf, int recvcount, mys_MPI_Datatype recvtype, int root, mys_MPI_Comm comm)
{
#ifdef MYS_USE_PMPI
   return PMPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
#else
   return MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm);
#endif
}

MYS_PUBLIC int mys_MPI_Allreduce(void *sendbuf, void *recvbuf, int count, mys_MPI_Datatype datatype, mys_MPI_Op op, mys_MPI_Comm comm)
{
#ifdef MYS_USE_PMPI
    return PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
#else
    return MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
#endif
}

MYS_PUBLIC int mys_MPI_Allgather(void *sendbuf, int sendcount, mys_MPI_Datatype sendtype, void *recvbuf, int recvcount, mys_MPI_Datatype recvtype, mys_MPI_Comm comm)
{
#ifdef MYS_USE_PMPI
   return PMPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
#else
   return MPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
#endif
}

MYS_PUBLIC double mys_MPI_Wtime()
{
#ifdef MYS_USE_PMPI
    return PMPI_Wtime();
#else
    return MPI_Wtime();
#endif
}

#endif // MYS_NO_MPI
