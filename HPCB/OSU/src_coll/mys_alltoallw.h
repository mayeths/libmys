#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>

#ifdef A2A_ENABLE_GPTL
#include <gptl.h>
#else
#define GPTLstart(...) ((void)(0))
#define GPTLstop(...) ((void)(0))
#endif

/**
 * Define the group size of grouping-based alltoallw.
 * Must be positive number.
 */
#ifndef A2A_GROUP_SIZE
#define A2A_GROUP_SIZE 8
#endif

/**
 * Use gatherv and scatterv to implement grouping-based alltoallw.
 * But note that they may encounter problems with argument count == 0 on Sunway 9A system.
 * So you may fallback to naive send/recv based method by undefining these macros.
 * We will fix this problem in the future by checking arguments of gatherv/scatterv carefully before invoking them.
 */
// #define A2A_USE_GATHERV
// #define A2A_USE_SCATTERV

/*-------------------------------------------------------------------------------------*/
/*                               Internal Implementation                               */
/*-------------------------------------------------------------------------------------*/

#define _CHKRET(fncall) do {        \
    _ierr = (int)(fncall);          \
    if (_ierr != 0) goto _fini;     \
} while (0)
#define _CHKPTR(fncall) do {        \
    void *_iptr = (void *)(fncall); \
    if (_iptr == NULL) {            \
        _ierr = MPI_ERR_NO_MEM;     \
        goto _fini;                 \
    }                               \
} while (0)
#define _CHKIERR _ierr
#define _CHKJUMP _fini

#define _MAX(a, b)                     ((a) > (b) ? (a) : (b))
#define _MIN(a, b)                     ((a) < (b) ? (a) : (b))
#define _RMIDX(row, col, nrows, ncols) ((row) * (ncols) + (col))  // Row major indexing
#define _CMIDX(row, col, nrows, ncols) ((col) * (nrows) + (row))  // Col major indexing

/* Provide Linux-like static_assert if we don't run in C++ and C11 */
#if defined(__cplusplus) || defined(static_assert)
#define _STATIC_ASSERT(expr, diagnostic) static_assert(expr, diagnostic)
#else /* from glibc: misc/sys/cdefs.h [commit] 3999d26ead93990b244ada078073fb58fb8bb5be */
#define _STATIC_ASSERT(expr, diagnostic)       \
    extern int(*__Static_assert_function(void) \
    )[!!sizeof(struct { int emit_error_if_static_assert_failed : (expr) ? 2 : -1; })]
#endif

int mys_alltoallw(
    const void *sendbuf,
    const int sendcounts[],
    const int sdispls[],
    const MPI_Datatype sendtypes[],
    void *recvbuf,
    const int recvcounts[],
    const int rdispls[],
    const MPI_Datatype recvtypes[],
    MPI_Comm comm
)
{
    _STATIC_ASSERT(A2A_GROUP_SIZE > 0, "A2A_GROUP_SIZE should be a positive number");
    // The reason we hoist these variables declaration here is we use 'goto' in _CHKRET() and _CHKPTR() macros.
    // Compiler complains about "transfer of control bypasses initialization" if we declare variables after macros.
    // Many C libraries follow this coding logic for the same reason.
    // This is the simplest way in C to clean up resources like allocated memories before returning error code.
    int _CHKIERR = 0;
    int *sbytes              = NULL, *rbytes          = NULL;
    int *sendbytes           = NULL, *recvbytes       = NULL;
    uint8_t *proxyout_buffer = NULL, *proxyin_buffer  = NULL;
    int *proxyout_nbytes     = NULL, *proxyout_displs = NULL;
    uint8_t *recvbuf_ptr     = NULL, *sendbuf_ptr     = NULL;
    uint8_t *send_buffer     = NULL;
    MPI_Request *requests    = NULL;
    int stage                = -1,    i            = -1;
    int ncols                = -1,    nrows        = -1;
    int row                  = -1,    col          = -1;
    int total_sbytes         = -1,    total_rbytes = -1;
    int proxyout_len         = -1,    proxyin_len  = -1;
    int position             = -1,    offset       = -1;

    // Organize processes like matrix, where rows are independent groups.
    // The processes in the first column served as group roots.
    // All inter-row communications are proxied by roots.
    ncols = A2A_GROUP_SIZE;

    typedef struct comm_pkg_t {
        MPI_Comm comm;
        int nranks;
        int myrank;
        bool isroot;
    } comm_pkg_t;

    comm_pkg_t mat_pkg, row_pkg, col_pkg;
    {
        mat_pkg.comm = comm;
        _CHKRET(MPI_Comm_size(mat_pkg.comm, &mat_pkg.nranks));
        _CHKRET(MPI_Comm_rank(mat_pkg.comm, &mat_pkg.myrank));
        nrows = (mat_pkg.nranks - 1) / ncols + 1;
        mat_pkg.isroot = mat_pkg.myrank == 0;
    }
    // Create the intra-row communicator. Only root processes can communicate with each other in different rows.
    {
        int row_color = mat_pkg.myrank / ncols;
        _CHKRET(MPI_Comm_split(mat_pkg.comm, row_color, mat_pkg.myrank, &row_pkg.comm));
        _CHKRET(MPI_Comm_size(row_pkg.comm, &row_pkg.nranks));
        _CHKRET(MPI_Comm_rank(row_pkg.comm, &row_pkg.myrank));
        row_pkg.isroot = row_pkg.myrank == 0;
    }
    // Create the inter-row communicator, only the one created by roots matter.
    {
        _CHKRET(MPI_Comm_split(mat_pkg.comm, row_pkg.myrank, mat_pkg.myrank, &col_pkg.comm));
        _CHKRET(MPI_Comm_size(col_pkg.comm, &col_pkg.nranks));
        _CHKRET(MPI_Comm_rank(col_pkg.comm, &col_pkg.myrank));
        col_pkg.isroot = col_pkg.myrank == 0;
    }

    for (stage = 0; stage < nrows; stage++) {
        GPTLstart("stage");
        /* The following formula find our opposite row on current stage.
         * 看下面这个4行的匹配阵例子，打o说明两行匹配，如(1,2)为o，说明行1和行2匹配。
         *    第0个stage时，4行的矩阵就是画(0,0),(3,1),(2,2),(1,3)的斜线，也就是0和2自己内部通信，1和3通信
         *    第1个stage时，4行的矩阵就是画(0,1),(1,0),(2,3),(3,2)的斜线，也就是0和1通信，2和3通信
         *    第2个stage时，4行的矩阵就是画(0,2),(1,1),(2,0),(3,3)的斜线，也就是0和2通信，1和3自己内部通信
         *    第3个stage时，4行的矩阵就是画(0,3),(1,2),(2,1),(3,0)的斜线，也就是0和3通信，1和2通信
         *    这些stage结束之后，stage矩阵就全点亮了，也就是两两都进行过数据交换，保证了正确性。
         *           | stage 0 | stage 1 | stage 2 | stage 3 | stage阶段
         *       行0 | o x x x | x o x x | x x o x | x x x o | 0 1 2 3
         *       行1 | x x x o | o x x x | x o x x | x x o x | 1 2 3 0
         *       行2 | x x o x | x x x o | o x x x | x o x x | 2 3 0 1
         *       行3 | x o x x | x x o x | x x x o | o x x x | 3 0 1 2
         * 通过 A2A_GROUP_SIZE 平衡行数和列数（该宏也就是列数，我们一行中的所有列算是一组，行长代理完成行间通信），
         * 行数影响 stage 数量，越多则需要的stage迭代步数越多，从而影响总体性能；
         * 列数影响每个stage中 gather/scatter 的进程数量，从而影响单步性能。
         * 最多需要 nrows 个循环就能完成这些stage，并且所有行在每个stage中都有活干。
         */
        row = (nrows - col_pkg.myrank + stage) % nrows;

        GPTLstart("pre_thing");

        // Mark these variables as NULL so we can simply free them when cleanup.
        // Freeing NULL is safe that guaranteed by C standard.
        sbytes          = NULL, rbytes          = NULL;
        sendbytes       = NULL, recvbytes       = NULL;
        proxyout_buffer = NULL, proxyin_buffer  = NULL;
        proxyout_nbytes = NULL, proxyout_displs = NULL;
        recvbuf_ptr     = NULL, sendbuf_ptr     = NULL;
        send_buffer     = NULL;
        requests        = NULL;

        GPTLstart("pre_thing_10");
        _CHKPTR(sbytes = (int *)calloc(ncols, sizeof(int)));
        _CHKPTR(rbytes = (int *)calloc(ncols, sizeof(int)));
        total_sbytes = 0;
        total_rbytes = 0;
        for (col = 0; col < ncols; col++) {
            int mat_rank = row * ncols + col;
            if (mat_rank >= mat_pkg.nranks)
                break;

            sbytes[col] = 0;
            rbytes[col] = 0;
            if (sendcounts[mat_rank] > 0)
                _CHKRET(MPI_Type_size(sendtypes[mat_rank], &sbytes[col]));
            if (recvcounts[mat_rank] > 0)
                _CHKRET(MPI_Type_size(recvtypes[mat_rank], &rbytes[col]));
            sbytes[col] = sbytes[col] * sendcounts[mat_rank];
            rbytes[col] = rbytes[col] * recvcounts[mat_rank];
            total_sbytes += sbytes[col];
            total_rbytes += rbytes[col];
        }
        GPTLstop("pre_thing_10");

        GPTLstart("pre_thing_20");
        sendbytes = NULL;
        recvbytes = NULL;
        proxyout_nbytes = NULL;
        proxyout_len = 0;
        proxyin_len = 0;
        if (row_pkg.isroot) {
            _CHKPTR(sendbytes       = (int *)calloc(ncols * ncols, sizeof(int)));
            _CHKPTR(recvbytes       = (int *)calloc(ncols * ncols, sizeof(int)));
            _CHKPTR(proxyout_nbytes = (int *)calloc(ncols, sizeof(int)));
        }
        _CHKRET(MPI_Gather(sbytes, ncols, MPI_INT, sendbytes, ncols, MPI_INT, 0, row_pkg.comm));
        _CHKRET(MPI_Gather(rbytes, ncols, MPI_INT, recvbytes, ncols, MPI_INT, 0, row_pkg.comm));
        _CHKRET(MPI_Gather(&total_sbytes, 1, MPI_INT, proxyout_nbytes, 1, MPI_INT, 0, row_pkg.comm));
        _CHKRET(MPI_Reduce(&total_sbytes, &proxyout_len, 1, MPI_INT, MPI_SUM, 0, row_pkg.comm));
        _CHKRET(MPI_Reduce(&total_rbytes, &proxyin_len, 1, MPI_INT, MPI_SUM, 0, row_pkg.comm));
        GPTLstop("pre_thing_20");

        GPTLstart("pre_thing_30");
        proxyout_buffer = NULL;
        proxyin_buffer = NULL;
        proxyout_displs = NULL;
        if (row_pkg.isroot) {
            GPTLstart("pre_thing_31");
            _CHKPTR(proxyout_buffer = (uint8_t *)malloc(proxyout_len * sizeof(uint8_t)));
            _CHKPTR(proxyin_buffer  = (uint8_t *)malloc(proxyin_len * sizeof(uint8_t)));
            _CHKPTR(proxyout_displs = (int *)malloc(ncols * sizeof(int)));
            GPTLstop("pre_thing_31");
            GPTLstart("pre_thing_32");
            for (i = 1; i < ncols; i++) {
                proxyout_displs[i] = proxyout_displs[i - 1] + proxyout_nbytes[i - 1];
            }
            GPTLstop("pre_thing_32");
        }
        GPTLstop("pre_thing_30");
        GPTLstop("pre_thing");

        /* [1.0] Intra Row Gather: Pack our data to memory */
        GPTLstart("intra_row_gather");
        _CHKPTR(send_buffer = (uint8_t *)calloc(_MAX(total_sbytes, 1), sizeof(uint8_t)));
        position = 0;
        for (col = 0; col < ncols; col++) {
            int mat_rank = row * ncols + col;
            if (mat_rank >= mat_pkg.nranks)
                break;
            if (sendcounts[mat_rank] == 0)
                continue;

            sendbuf_ptr = &((uint8_t *)sendbuf)[sdispls[mat_rank]];
            _CHKRET(MPI_Pack(
                sendbuf_ptr, sendcounts[mat_rank], sendtypes[mat_rank],
                send_buffer, _MAX(total_sbytes, 1), &position, row_pkg.comm
            ));
        }

        /* [1.1] Intra Row Gather: Send to root process */
#ifdef A2A_USE_GATHERV
        _CHKRET(MPI_Gatherv(
            send_buffer, total_sbytes, MPI_UINT8_T,
            proxyout_buffer, proxyout_nbytes, proxyout_displs, MPI_UINT8_T,
            0, row_pkg.comm
        ));
#else
        {
            requests = NULL;
            int nrequest = 0;
            if (row_pkg.isroot) {
                _CHKPTR(requests = (MPI_Request *)calloc(sizeof(MPI_Request), row_pkg.nranks));
                for (i = 0; i < row_pkg.nranks; i++) {
                    if (proxyout_nbytes[i] == 0)
                        continue;

                    _CHKRET(MPI_Irecv(
                        &proxyout_buffer[proxyout_displs[i]], proxyout_nbytes[i],
                        MPI_UINT8_T, i, 7749, row_pkg.comm,
                        &requests[nrequest]
                    ));
                    nrequest += 1;
                }
            }
            if (total_sbytes > 0) {
                _CHKRET(MPI_Send(send_buffer, total_sbytes, MPI_UINT8_T, 0, 7749, row_pkg.comm));
            }
            if (row_pkg.isroot) {
                _CHKRET(MPI_Waitall(nrequest, requests, MPI_STATUSES_IGNORE));
                free(requests);
            }
        }
#endif
        GPTLstop("intra_row_gather");

        /* [2.0] Inter Row Exchange */
        GPTLstart("inter_row_exchange");
        if (row_pkg.isroot) {
            _CHKRET(MPI_Sendrecv(
                proxyout_buffer, proxyout_len, MPI_UINT8_T, row, 8864,
                proxyin_buffer, proxyin_len, MPI_UINT8_T, row, 8864,
                col_pkg.comm, MPI_STATUS_IGNORE
            ));
        }
        GPTLstop("inter_row_exchange");

        /* [3.0] Intra Row Scatter: Scatter data from processes in opposite row one by one */
        offset = 0;
        GPTLstart("intra_row_scatter");
        for (col = 0; col < ncols; col++) {
            int mat_rank = row * ncols + col;
            if (mat_rank >= mat_pkg.nranks)
                break;

            int *displs = NULL;
            int *counts = NULL;
            if (row_pkg.isroot) {
                _CHKPTR(displs = (int *)calloc(ncols, sizeof(int)));
                _CHKPTR(counts = (int *)calloc(ncols, sizeof(int)));
                displs[0] = offset;
                counts[0] = recvbytes[_RMIDX(0, col, ncols, ncols)];
                for (i = 1; i < ncols; i++) {
                    displs[i] = displs[i - 1] + counts[i - 1];
                    counts[i] = recvbytes[_RMIDX(i, col, ncols, ncols)];
                }
            }

            recvbuf_ptr = &((uint8_t *)recvbuf)[rdispls[mat_rank]];
#ifdef A2A_USE_SCATTERV
            _CHKRET(MPI_Scatterv(
                proxyin_buffer, counts, displs, MPI_UINT8_T,
                recvbuf_ptr, recvcounts[mat_rank], recvtypes[mat_rank],
                0, row_pkg.comm
            ));
#else
            {
                requests = NULL;
                int nrequest = 0;
                if (row_pkg.isroot) {
                    _CHKPTR(requests = (MPI_Request *)calloc(sizeof(MPI_Request), row_pkg.nranks));
                    for (i = 0; i < row_pkg.nranks; i++) {
                        if (counts[i] == 0)
                            continue;

                        _CHKRET(MPI_Isend(
                            &proxyin_buffer[displs[i]], counts[i],
                            MPI_UINT8_T, i, 9981, row_pkg.comm,
                            &requests[nrequest]
                        ));
                        nrequest += 1;
                    }
                }
                if (recvcounts[mat_rank] != 0) {
                    _CHKRET(MPI_Recv(
                        recvbuf_ptr, recvcounts[mat_rank], recvtypes[mat_rank],
                        0, 9981, row_pkg.comm,
                        MPI_STATUS_IGNORE
                    ));
                }
                if (row_pkg.isroot) {
                    _CHKRET(MPI_Waitall(nrequest, requests, MPI_STATUSES_IGNORE));
                    free(requests);
                }
            }
#endif

            if (row_pkg.isroot) {
                offset = displs[ncols - 1] + counts[ncols - 1];
                free(displs);
                free(counts);
            }
        }
        GPTLstop("intra_row_scatter");

        /* [4.0] End of Communication Between Two Rows */
_CHKJUMP:
        GPTLstart("post_thing");
        if (row_pkg.isroot) {
            free(sendbytes);
            free(recvbytes);
            free(proxyout_nbytes);
            free(proxyout_displs);
            free(proxyout_buffer);
            free(proxyin_buffer);
        }
        free(rbytes);
        free(sbytes);
        free(send_buffer);
        if (_CHKIERR != 0) {
            MPI_Abort(MPI_COMM_WORLD, 7777777);
            break;
        }
        GPTLstop("post_thing");

        GPTLstop("stage");
    }

    return _CHKIERR;
}

#undef _CHKRET
#undef _CHKPTR
#undef _CHKIERR
#undef _CHKJUMP
#undef _MAX
#undef _MIN
#undef _RMIDX
#undef _CMIDX
#undef _STATIC_ASSERT

#ifndef A2A_ENABLE_GPTL
#undef GPTLstart
#undef GPTLstop
#endif
