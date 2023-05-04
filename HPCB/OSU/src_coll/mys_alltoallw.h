#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <mpi.h>

#include <mys.h>

#ifdef A2A_ENABLE_GPTL
#include <gptl.h>
#else
#define GPTLstart(...)
#define GPTLstop(...)
#endif

#include "nodeloc.h"

/*-------------------------------------------------------------------------------------*/
/*                               Internal Implementation                               */
/*-------------------------------------------------------------------------------------*/

#define _MAX(a, b)                     ((a) > (b) ? (a) : (b))
#define _MIN(a, b)                     ((a) < (b) ? (a) : (b))
#define _RMIDX(row, col, nrows, ncols) ((row) * (ncols) + (col))  // Row major indexing
#define _CMIDX(row, col, nrows, ncols) ((col) * (nrows) + (row))  // Col major indexing

typedef struct comm_pkg_t {
    MPI_Comm comm;
    int nranks;
    int myrank;
    bool isroot;
} comm_pkg_t;

static void _mys_form_matrix(
    MPI_Comm comm,
    int *nrows,
    int *ncols,
    comm_pkg_t *mat_pkg,
    comm_pkg_t *row_pkg,
    comm_pkg_t *col_pkg
)
{
    groupinfo_t ginfo = get_group(comm);
    int matrank_offset = 0;
    MPI_Exscan(&ginfo.intra_nranks, &matrank_offset, 1, MPI_INT, MPI_SUM, ginfo.inter_comm);
    MPI_Bcast(&matrank_offset, 1, MPI_INT, 0, ginfo.intra_comm);

    int shape[2] = {ginfo.inter_nranks, ginfo.intra_nranks};
    MPI_Allreduce(MPI_IN_PLACE, shape, 2, MPI_INT, MPI_MAX, comm);
    *nrows = shape[0];
    *ncols = shape[1];

    {
        MPI_Comm_dup(comm, &mat_pkg->comm);
        MPI_Comm_size(mat_pkg->comm, &mat_pkg->nranks);
        MPI_Comm_rank(mat_pkg->comm, &mat_pkg->myrank);
        mat_pkg->isroot = mat_pkg->myrank == 0;
    }
    // Create the intra-row communicator. Only root processes can communicate with each other in different rows.
    {
        MPI_Comm_dup(ginfo.intra_comm, &row_pkg->comm);
        MPI_Comm_size(row_pkg->comm, &row_pkg->nranks);
        MPI_Comm_rank(row_pkg->comm, &row_pkg->myrank);
        row_pkg->isroot = row_pkg->myrank == 0;
    }
    // Create the inter-row communicator, only the one created by roots matter.
    {
        MPI_Comm_dup(ginfo.inter_comm, &col_pkg->comm);
        MPI_Comm_size(col_pkg->comm, &col_pkg->nranks);
        MPI_Comm_rank(col_pkg->comm, &col_pkg->myrank);
        col_pkg->isroot = col_pkg->myrank == 0;
    }

    // int my_new_rank = matrank_offset + ginfo.intra_myrank;
    // DLOG_ORDERED("rank=%d matrank_offset=%d my_new_rank=%d", mat_pkg->myrank, matrank_offset, my_new_rank);

    MPI_Comm_free(&ginfo.inter_comm);
    MPI_Comm_free(&ginfo.intra_comm);
    free(ginfo.intra_ranks);
}

typedef struct a2a_shared_ctx_t {
    struct {
        MPI_Win win;
        uint8_t *buf;
        int glolen;
        int loclen;
        int offset;
        int capacity;
    } proxyout;
    struct {
        MPI_Win win;
        uint8_t *buf;
        int glolen;
        int capacity;
    } proxyin;
} a2a_shared_ctx_t;

static a2a_shared_ctx_t a2a_shared_ctx = {
    .proxyout = {
        .win = MPI_WIN_NULL,
        .buf = NULL,
        .glolen = 0,
        .loclen = 0,
        .offset = 0,
        .capacity = 0,
    },
    .proxyin = {
        .win = MPI_WIN_NULL,
        .buf = NULL,
        .glolen = 0,
        .capacity = 0,
    }
};

static void adjust_ctx_shared(
    a2a_shared_ctx_t *ctx,
    int total_sbytes, // Total send bytes to opposite row
    int total_rbyte, // Total recv bytes from opposite row
    comm_pkg_t *row_pkg
)
{
    MPI_Allreduce(&total_sbytes, &ctx->proxyout.glolen, 1, MPI_INT, MPI_SUM, row_pkg->comm);
    if (ctx->proxyout.glolen > ctx->proxyout.capacity) {
        if (ctx->proxyout.win != MPI_WIN_NULL) {
            MPI_Win_free(&ctx->proxyout.win);
        }
        MPI_Win_allocate_shared(
            total_sbytes, sizeof(uint8_t), MPI_INFO_NULL, row_pkg->comm,
            &ctx->proxyout.buf, &ctx->proxyout.win
        );
        ctx->proxyout.loclen = total_sbytes;
        // The value in recvbuf of MPI_Exscan on process 0 is undefined and unreliable
        // as recvbuf is not significant for process 0. Set ctx->proxyout.offset = 0 for rank 0.
        ctx->proxyout.offset = 0;
        MPI_Exscan(&total_sbytes, &ctx->proxyout.offset, 1, MPI_INT, MPI_SUM, row_pkg->comm);
        ctx->proxyout.capacity = ctx->proxyout.glolen;
    } else {
        int offset = 0;
        MPI_Exscan(&total_sbytes, &offset, 1, MPI_INT, MPI_SUM, row_pkg->comm);
        int adjust = offset - ctx->proxyout.offset;
        ctx->proxyout.buf += adjust;
        ctx->proxyout.loclen = total_sbytes;
        ctx->proxyout.offset += adjust;
    }

    MPI_Allreduce(&total_rbyte, &ctx->proxyin.glolen, 1, MPI_INT, MPI_SUM, row_pkg->comm);
    if (ctx->proxyin.glolen > ctx->proxyin.capacity) {
        if (ctx->proxyin.win != MPI_WIN_NULL) {
            MPI_Win_free(&ctx->proxyin.win);
        }
        int alloc_len = row_pkg->isroot ? ctx->proxyin.glolen : 0;
        MPI_Win_allocate_shared(
            alloc_len, sizeof(uint8_t), MPI_INFO_NULL, row_pkg->comm,
            &ctx->proxyin.buf, &ctx->proxyin.win
        );
        if (!row_pkg->isroot) {
            ctx->proxyin.buf -= ctx->proxyin.glolen;
        }
        ctx->proxyin.capacity = ctx->proxyin.glolen;
    } else {
        // Do nothing
    }
}

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
    GPTLstart("mys_alltoallw");
    int *sbytes              = NULL, *rbytes          = NULL;
    uint8_t *recvbuf_ptr     = NULL, *sendbuf_ptr     = NULL;
    int stage                = -1,    i            = -1;
    int ncols                = -1,    nrows        = -1;
    int row                  = -1,    col          = -1;
    int total_sbytes         = -1,    total_rbytes = -1;
    int position             = -1;
    assert(sendbuf != recvbuf);

    GPTLstart("FormMatrix");
    comm_pkg_t mat_pkg, row_pkg, col_pkg;
    _mys_form_matrix(comm, &nrows, &ncols, &mat_pkg, &row_pkg, &col_pkg);
    assert((size_t)nrows * (size_t)ncols < (size_t)INT_MAX);
    GPTLstop("FormMatrix");

    for (stage = 0; stage < nrows; stage++) {
        row = (nrows - col_pkg.myrank + stage) % nrows;

        sbytes          = NULL, rbytes          = NULL;
        recvbuf_ptr     = NULL, sendbuf_ptr     = NULL;

        GPTLstart("LocalPrepare");
        CHKPTR(sbytes = (int *)calloc(ncols, sizeof(int)));
        CHKPTR(rbytes = (int *)calloc(ncols, sizeof(int)));
        total_sbytes = 0;
        total_rbytes = 0;
        for (col = 0; col < ncols; col++) {
            int mat_rank = row * ncols + col;
            if (mat_rank >= mat_pkg.nranks)
                break;

            sbytes[col] = 0;
            rbytes[col] = 0;
            if (sendcounts[mat_rank] > 0)
                CHKRET(MPI_Type_size(sendtypes[mat_rank], &sbytes[col]));
            if (recvcounts[mat_rank] > 0)
                CHKRET(MPI_Type_size(recvtypes[mat_rank], &rbytes[col]));
            sbytes[col] = sbytes[col] * sendcounts[mat_rank];
            rbytes[col] = rbytes[col] * recvcounts[mat_rank];
            total_sbytes += sbytes[col];
            total_rbytes += rbytes[col];
        }
        GPTLstop("LocalPrepare");

        GPTLstart("RootPrepare");
        struct root_ctx_t {
            int *sendbytes;
            int *recvbytes;
            int *proxyout_nbytes;
            int *proxyout_displs;
            int proxyout_len;
            int proxyin_len;
            int *proxyin_nbytes;
        };
        struct root_ctx_t ctx0;
        if (row_pkg.isroot) {
            CHKPTR(ctx0.sendbytes       = (int *)calloc(ncols * ncols, sizeof(int)));
            CHKPTR(ctx0.recvbytes       = (int *)calloc(ncols * ncols, sizeof(int)));
            CHKPTR(ctx0.proxyout_nbytes = (int *)calloc(ncols, sizeof(int)));
            CHKPTR(ctx0.proxyin_nbytes  = (int *)calloc(ncols, sizeof(int)));
        }
        CHKRET(MPI_Gather(sbytes, ncols, MPI_INT, ctx0.sendbytes, ncols, MPI_INT, 0, row_pkg.comm));
        CHKRET(MPI_Gather(rbytes, ncols, MPI_INT, ctx0.recvbytes, ncols, MPI_INT, 0, row_pkg.comm));
        CHKRET(MPI_Gather(&total_sbytes, 1, MPI_INT, ctx0.proxyout_nbytes, 1, MPI_INT, 0, row_pkg.comm));
        CHKRET(MPI_Reduce(&total_sbytes, &ctx0.proxyout_len, 1, MPI_INT, MPI_SUM, 0, row_pkg.comm));
        CHKRET(MPI_Reduce(&total_rbytes, &ctx0.proxyin_len, 1, MPI_INT, MPI_SUM, 0, row_pkg.comm));
        CHKRET(MPI_Reduce(rbytes, ctx0.proxyin_nbytes, ncols, MPI_INT, MPI_SUM, 0, row_pkg.comm));

        if (row_pkg.isroot) {
            CHKPTR(ctx0.proxyout_displs = (int *)calloc(ncols, sizeof(int)));
            ctx0.proxyout_displs[0] = 0;
            for (i = 1; i < ncols; i++) {
                ctx0.proxyout_displs[i] = ctx0.proxyout_displs[i - 1] + ctx0.proxyout_nbytes[i - 1];
            }
        }
        GPTLstop("RootPrepare");

        GPTLstart("CreateSharedMemory");
        adjust_ctx_shared(&a2a_shared_ctx, total_sbytes, total_rbytes, &row_pkg);
        GPTLstop("CreateSharedMemory");

        /* [1.0] Intra Row Gather: Pack our data to memory */
        GPTLstart("Internal_Gather");
        position = 0;
        for (col = 0; col < ncols; col++) {
            int mat_rank = row * ncols + col;
            if (mat_rank >= mat_pkg.nranks)
                break;
            if (sendcounts[mat_rank] == 0)
                continue;

            sendbuf_ptr = &((uint8_t *)sendbuf)[sdispls[mat_rank]];
            // TLOG(MYRANK(), "packing for rank %d. displs %d", mat_rank, sdispls[mat_rank]);
            MPI_Pack(
                sendbuf_ptr, sendcounts[mat_rank], sendtypes[mat_rank],
                a2a_shared_ctx.proxyout.buf, a2a_shared_ctx.proxyout.loclen, &position, row_pkg.comm
            );
        }

        // Ensure to wait other processes write
        MPI_Barrier(mat_pkg.comm);
        GPTLstop("Internal_Gather");

        /* [2.0] Inter Row Exchange */
        GPTLstart("External_Exchange");
        if (row_pkg.isroot) {
            CHKRET(MPI_Sendrecv(
                a2a_shared_ctx.proxyout.buf, ctx0.proxyout_len, MPI_UINT8_T, row, 1008864,
                a2a_shared_ctx.proxyin.buf, ctx0.proxyin_len, MPI_UINT8_T, row, 1008864,
                col_pkg.comm, MPI_STATUS_IGNORE
            ));
        }
        GPTLstop("External_Exchange");

        /* [3.0] Intra Row Scatter: Scatter data from processes in opposite row one by one */
        int *displs = NULL;
        GPTLstart("Internal_Scatter");
        CHKPTR(displs = (int *)calloc(ncols, sizeof(int)));
        if (row_pkg.isroot) {
            displs[0] = 0;
            for (col = 1; col < ncols; col++) {
                displs[col] = displs[col - 1] + ctx0.proxyin_nbytes[col - 1];
            }
            for (col = 0; col < ncols; col++) {
                displs[col] += rbytes[col];
            }
            MPI_Exscan(MPI_IN_PLACE, displs, ncols, MPI_INT, MPI_SUM, row_pkg.comm);
            displs[0] = 0;
            for (col = 1; col < ncols; col++) {
                displs[col] = displs[col - 1] + ctx0.proxyin_nbytes[col - 1];
            }
            if (stage == 0) {
                if (mat_pkg.myrank == 0) {
                }
            }
        } else {
            MPI_Exscan(rbytes, displs, ncols, MPI_INT, MPI_SUM, row_pkg.comm);
        }

        for (col = 0; col < ncols; col++) {
            position = 0;
            int mat_rank = row * ncols + col;
            if (mat_rank >= mat_pkg.nranks)
                break;
            if (recvcounts[mat_rank] == 0)
                continue;

            recvbuf_ptr = &((uint8_t *)recvbuf)[rdispls[mat_rank]];
            MPI_Unpack(
                &a2a_shared_ctx.proxyin.buf[displs[col]], a2a_shared_ctx.proxyin.capacity,
                &position, recvbuf_ptr, recvcounts[mat_rank], recvtypes[mat_rank], MPI_COMM_WORLD
            );
        }
        free(displs);

        // Ensure to wait other processes read
        MPI_Barrier(mat_pkg.comm);
        GPTLstop("Internal_Scatter");

        /* [4.0] End of Communication Between Two Rows */
        GPTLstart("ReleaseResource");
        if (row_pkg.isroot) {
            free(ctx0.sendbytes);
            free(ctx0.recvbytes);
            free(ctx0.proxyout_nbytes);
            free(ctx0.proxyin_nbytes);
            free(ctx0.proxyout_displs);
        }
        // _free_ctx_shared(&ctx);
        free(rbytes);
        free(sbytes);
        GPTLstop("ReleaseResource");
    }

#ifdef A2A_VALIDATION
    for (size_t j = 0; j < sendbuf_len; j++) {
        DLOG_ORDERED("sendbuf[%zu]=%d", j, (int)(((uint8_t *)sendbuf)[j]));
    }
    WLOG(0, "Good");

    uint8_t *sendbuf_chk = (uint8_t *)calloc(sizeof(uint8_t), sendbuf_len);
    uint8_t *recvbuf_chk = (uint8_t *)calloc(sizeof(uint8_t), recvbuf_len);
    int ret_chk = MPI_Alltoallw(sendbuf, sendcounts, sdispls, sendtypes, recvbuf_chk, recvcounts, rdispls, recvtypes, comm);
    assert(ret_chk == MPI_SUCCESS);
    int badat = -1;
    for (size_t i = 0; i < recvbuf_len; i++) {
        if (((uint8_t *)recvbuf)[i] != ((uint8_t *)recvbuf_chk)[i]) {
            badat = i;
            break;
        }
    }
    int gbadat = -1;
    MPI_Allreduce(&badat, &gbadat, 1, MPI_INT, MPI_MAX, comm);
    if (gbadat != -1) {
        ELOG_ORDERED("badat %d", badat);
        MPI_Abort(comm, 666);
    }
    free(sendbuf_chk);
    free(recvbuf_chk);
#endif

    GPTLstop("mys_alltoallw");
    return 0;
}
