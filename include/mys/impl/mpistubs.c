/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
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

#ifdef MYS_NO_MPI
/**************************************************/
/* MPI stubs to generate serial codes without mpi */
/**************************************************/

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
    return MPI_Initialized(flag);
}

MYS_PUBLIC int mys_MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    return MPI_Init_thread(argc, argv, required, provided);
}

MYS_PUBLIC int mys_MPI_Finalize()
{
    return MPI_Finalize();
}

MYS_PUBLIC int mys_MPI_Comm_rank(mys_MPI_Comm comm, int *rank)
{
    return MPI_Comm_rank(comm, rank);
}

MYS_PUBLIC int mys_MPI_Comm_size(mys_MPI_Comm comm, int *size)
{
    return MPI_Comm_size(comm, size);
}

MYS_PUBLIC int mys_MPI_Comm_split(mys_MPI_Comm comm, int n, int m, mys_MPI_Comm *comms)
{
    return MPI_Comm_split(comm, n, m, comms);
}

MYS_PUBLIC int mys_MPI_Comm_split_type(mys_MPI_Comm comm, int split_type, int key, MPI_Info info, mys_MPI_Comm *newcomm)
{
    return MPI_Comm_split_type(comm, split_type, key, info, newcomm);
}

MYS_PUBLIC int mys_MPI_Comm_free(mys_MPI_Comm *comm)
{
   return MPI_Comm_free(comm);
}

MYS_PUBLIC int mys_MPI_Probe(int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status)
{
    return MPI_Probe(source, tag, comm, status);
}

MYS_PUBLIC int mys_MPI_Get_count(mys_MPI_Status *status, mys_MPI_Datatype datatype, int *count)
{
    return MPI_Get_count(status, datatype, count);
}

MYS_PUBLIC int mys_MPI_Recv(void *buf, int count, mys_MPI_Datatype datatype, int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status)
{
    return MPI_Recv(buf, count, datatype, source, tag, comm, status);
}

MYS_PUBLIC int mys_MPI_Send(const void *buf, int count, mys_MPI_Datatype datatype, int dest, int tag, mys_MPI_Comm comm)
{
    return MPI_Send(buf, count, datatype, dest, tag, comm);
}

MYS_PUBLIC int mys_MPI_Barrier(mys_MPI_Comm comm)
{
    return MPI_Barrier(comm);
}

MYS_PUBLIC int mys_MPI_Bcast(void *buffer, int count, mys_MPI_Datatype datatype, int root, mys_MPI_Comm comm)
{
    return MPI_Bcast(buffer, count, datatype, root, comm);
}

MYS_PUBLIC int mys_MPI_Allreduce(void *sendbuf, void *recvbuf, int count, mys_MPI_Datatype datatype, mys_MPI_Op op, mys_MPI_Comm comm)
{
    return MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
}

MYS_PUBLIC int mys_MPI_Allgather(void *sendbuf, int sendcount, mys_MPI_Datatype sendtype, void *recvbuf, int recvcount, mys_MPI_Datatype recvtype, mys_MPI_Comm comm)
{
   return MPI_Allgather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, comm);
}

MYS_PUBLIC double mys_MPI_Wtime()
{
    return MPI_Wtime();
}

#endif // MYS_NO_MPI
