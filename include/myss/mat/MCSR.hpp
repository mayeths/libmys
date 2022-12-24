#pragma once

#include <set>
#include <mpi.h>
#include "MBase.hpp"
#include "../vec/VCSR.hpp"
#include "mys.hpp"

class MCSR : public MBase<VCSR, int, double>
{
public:

    using BASE = MBase<VCSR, int, double>;
    using VType = BASE::VType;
    using IType = BASE::IType;
    using DType = BASE::DType;

    MPI_Comm comm = MPI_COMM_NULL;
    int nranks = -1, myrank = -1;
    int global_size;
    int local_begin;
    int local_end;
    int local_nnz;
    std::vector<int> rank_begins;
    std::vector<int> rank_ends;
    std::vector<int> I;
    std::vector<int> J;
    std::vector<double> V;
    std::map<int, int> external_indexs;
    std::vector<std::vector<int>> send_others;
    guard_t guard;

    // communication package: for exchange halo
    int halo_total_size = 0;
    std::vector<MPI_Datatype> send_types;
    std::vector<MPI_Datatype> recv_types;
    std::vector<std::vector<int>> halo_indexs;

    MCSR() { }

    MCSR(const MCSR &source) {
        source.guard.ensure();
        this->nranks = source.nranks;
        this->myrank = source.myrank;
        this->global_size = source.global_size;
        this->local_begin = source.local_begin;
        this->local_end = source.local_end;
        this->local_nnz = source.local_nnz;
        this->I.clear();
        this->J.clear();
        this->V.clear();
        std::copy(source.I.begin(), source.I.end(), this->I.begin());
        std::copy(source.J.begin(), source.J.end(), this->J.begin());
        std::copy(source.V.begin(), source.V.end(), this->V.begin());
        this->guard = source.guard;
        this->halo_total_size = source.halo_total_size;
        this->send_types = source.send_types;
        this->recv_types = source.recv_types;
        this->halo_indexs = source.halo_indexs;
    }

    static MCSR FromGlobalMatrix(const MPI_Comm comm, const int *Ap, const int *Aj, const double *Av, const int global_size, const std::vector<int> &rank_begins, const std::vector<int> &rank_ends)
    {
        MCSR res;
        res.comm = comm;
        MPI_Comm_rank(MPI_COMM_WORLD, &res.myrank);
        MPI_Comm_size(MPI_COMM_WORLD, &res.nranks);
        res.global_size = global_size;
        ASSERT_EQ(rank_begins.size(), res.nranks);
        ASSERT_EQ(rank_ends.size(), res.nranks);
        res.rank_begins.resize(res.nranks);
        res.rank_ends.resize(res.nranks);
        std::copy(rank_begins.begin(), rank_begins.end(), res.rank_begins.begin());
        std::copy(rank_ends.begin(), rank_ends.end(), res.rank_ends.begin());
        res.local_begin = rank_begins[res.myrank];
        res.local_end = rank_ends[res.myrank];

        std::vector<std::set<int>> halo_indexs(res.nranks);
        res.I.resize(res.local_end - res.local_begin + 1, 0);
        int test_icount = 0;
        int test_ecount = 0;
        for (int gi = res.local_begin; gi < res.local_end; gi++) {
            const int rowstart = Ap[gi];
            const int rowstop = Ap[gi + 1];
            res.I[gi + 1 - res.local_begin] = res.I[gi - res.local_begin];
            for (int jj = rowstart; jj < rowstop; jj++) {
                const int gj = Aj[jj];
                const double v = Av[jj];
                ASSERT_BETWEEN_IE(0, gj, global_size);
                res.I[gi + 1 - res.local_begin] += 1;
                if (gj >= res.local_begin && gj < res.local_end) {
                    res.J.push_back(gj);
                    res.V.push_back(v);
                    test_icount += 1;
                } else {
                    // find who maintain this index
                    auto upper = std::upper_bound(rank_begins.begin(), rank_begins.end(), gj);
                    // if (lower == rank_begins.end()) {
                    //     DEBUG(res.myrank, "ERROR: %d lower %p end %p", gj, lower, rank_begins.end());
                    // }
                    // ASSERT_NE(lower, rank_begins.end());
                    int maintainer = (upper - 1) - rank_begins.begin();
                    ASSERT_BETWEEN_IE(0, maintainer, res.nranks);
                    halo_indexs[maintainer].insert(gj);
                    res.J.push_back(gj);
                    res.V.push_back(v);
                    test_ecount += 1;
                }
            }
        }
        // for (int i = 1; i < res.I.size(); i++) {
        //     res.I[i] += res.I[i - 1];
        // }
        ASSERT_EQ(res.I.back(), Ap[res.local_end] - Ap[res.local_begin]);
        res.halo_indexs.resize(res.nranks);
        for (int rank = 0; rank < res.nranks; rank++) {
            res.halo_indexs[rank].resize(0);
            for (auto &gj : halo_indexs[rank]) {
                res.halo_indexs[rank].push_back(gj);
                res.external_indexs[gj] = res.local_end - res.local_begin + res.halo_total_size;
                res.halo_total_size += 1;
            }
        }
        std::vector<int> send_count(res.nranks);
        for (int rank = 0; rank < res.nranks; rank++) {
            int need = res.halo_indexs[rank].size();
            MPI_Gather(&need, 1, MPI_TYPE<int>(), send_count.data(), 1, MPI_TYPE<int>(), rank, res.comm);
            // for (int i = 0; i < res.nranks; i++)
            //     DEBUG(rank, "%d will send to %d %d values", this->myrank, i, send_count[i]);
        }

        res.send_others.resize(res.nranks);
        std::vector<MPI_Request> requests(res.nranks, MPI_REQUEST_NULL);
        for (int rank = 0; rank < res.nranks; rank++) {
            std::vector<int> &send_indexs = res.send_others[rank];
            send_indexs.resize(send_count[rank], 0);
            MPI_Irecv(send_indexs.data(), send_indexs.size(), MPI_TYPE<int>(), rank, RMIDX(rank, res.myrank, res.nranks, res.nranks), res.comm, &requests[rank]);
            MPI_Send(res.halo_indexs[rank].data(), res.halo_indexs[rank].size(), MPI_TYPE<int>(), rank, RMIDX(res.myrank, rank, res.nranks, res.nranks), res.comm);
        }
        MPI_Waitall(res.nranks, requests.data(), MPI_STATUSES_IGNORE);

        for (int rank = 0; rank < res.nranks; rank++) {
            for (int i = 0; i < res.nranks; i++) {
                // DEBUG(rank, "%d rely on %d for %d values", this->myrank, i, res.halo_indexs[i].size());
                for (int j = 0; j < res.halo_indexs[i].size(); j++) {
                    // DEBUG(rank, "    for example, rely on global index %d", res.halo_indexs[i][j]);
                }
            }
            MPI_Barrier(res.comm);
        }

        for (int rank = 0; rank < res.nranks; rank++) {
            for (int i = 0; i < res.nranks; i++) {
                std::vector<int> &send_indexs = res.send_others[i];
                // DEBUG(rank, "hhh %d will send to %d %d values", this->myrank, i, send_indexs.size());
                // for (int j = 0; j < send_indexs.size(); j++) {
                //     DEBUG(rank, "    for example, send global index %d", (int)send_indexs[j]);
                // }
            }
            MPI_Barrier(res.comm);
        }

        int halostart = res.local_end - res.local_begin;
        int localsize = halostart + res.halo_total_size;
        res.send_types.resize(res.nranks, MPI_DATATYPE_NULL);
        res.recv_types.resize(res.nranks, MPI_DATATYPE_NULL);
        for (int rank = 0; rank < res.nranks; rank++) {
            std::vector<int> &send_indexs = res.send_others[rank];
            for (int i = 0; i < send_indexs.size(); i++) {
                send_indexs[i] -= res.local_begin;
                ASSERT_BETWEEN_IE(0, send_indexs[i], res.local_end - res.local_begin);
            }
            std::vector<int> array_of_blocklengths(send_indexs.size(), 1);
            CHKRET(MPI_Type_indexed(send_indexs.size(), array_of_blocklengths.data(), send_indexs.data(), MPI_TYPE<double>(), &res.send_types[rank]));
            int halosize = res.halo_indexs[rank].size();
            CHKRET(MPI_Type_create_subarray(1, &localsize, &halosize, &halostart, MPI_ORDER_C, MPI_TYPE<double>(), &res.recv_types[rank]));
            // DEBUG_ORDERED("%d recv from %d -> x[%d:%d](%d)", res.myrank, rank, halostart, halostart + halosize, localsize);
            // DEBUG_ORDERED("(%d) <-> %d, send %d recv %d", res.myrank, rank, send_indexs.size(), halosize);
            halostart += halosize;
        }

        int local_size = res.local_end - res.local_begin; // without halo
        int icount = 0;
        int ecount = 0;
        for (int i = 0; i < local_size; i++) {
            const int rowstart = res.I[i];
            const int rowstop = res.I[i + 1];
            for (int jj = rowstart; jj < rowstop; jj++) {
                const int gj = res.J[jj];
                const double v = res.V[jj];
                // DEBUG(0, "@@ %d %f", gj, v);
                if (gj >= res.local_begin && gj < res.local_end) {
                    res.J[jj] = gj - res.local_begin;
                    ASSERT_BETWEEN_IE(0, res.J[jj], local_size);
                    icount += 1;
                } else {
                    res.J[jj] = res.external_indexs[gj];
                    ASSERT_BETWEEN_IE(local_size, res.J[jj], local_size + res.halo_total_size);
                    ecount += 1;
                }
            }
        }
        // ASSERT_EQ(icount, local_size);
        // ASSERT_EQ(ecount, res.halo_total_size);

        // DEBUG_ORDERED("local: %d %d halo: %d", res.local_begin, res.local_end, res.halo_total_size);

        res.guard.set();
        return res;
    }

    void ExchangeHalo(VCSR &x)
    {
        // DEBUG_ORDERED("local: %d %d halo: %d", this->local_begin, this->local_end, this->halo_total_size);
        ASSERT_EQ(this->local_end - this->local_begin + this->halo_total_size, x.values.size());
        this->guard.ensure();
        x.guard.ensure();
        ASSERT_EQ(this->send_types.size(), this->nranks);
        ASSERT_EQ(this->recv_types.size(), this->nranks);
        std::vector<MPI_Request> send_requests(this->nranks, MPI_REQUEST_NULL);
        std::vector<MPI_Request> recv_requests(this->nranks, MPI_REQUEST_NULL);
        int halostart = this->local_end - this->local_begin;
        int localsize = halostart + this->halo_total_size;
        for (int rank = 0; rank < this->nranks; rank++) {
            int halosize = this->halo_indexs[rank].size();
            // DEBUG_ORDERED("%d recv from %d: 1 block, x size %d halo size %d halo start %d (%p)", this->myrank, rank, localsize, halosize, halostart, this->recv_types[rank]);
            const MPI_Datatype &send_type = this->send_types[rank];
            const MPI_Datatype &recv_type = this->recv_types[rank];
            int send_tag = RMIDX(this->myrank, rank, this->nranks, this->nranks);
            int recv_tag = RMIDX(rank, this->myrank, this->nranks, this->nranks);
            // DEBUG_ORDERED("%d send to %d with tag %d", this->myrank, rank, send_tag);
            // DEBUG_ORDERED("%d recv from %d with tag %d", this->myrank, rank, recv_tag);
            int ret = MPI_Isend(x.values.data(), 1, send_type, rank, send_tag, this->comm, &send_requests[rank]);
            if (ret != 0) {
                printf("xxxxxxxxxx-------xxxxx---x-x-x--x-x-x-x\n");
            }
            CHKRET(MPI_Irecv(x.values.data(), 1, recv_type, rank, recv_tag, this->comm, &recv_requests[rank]));
            // CHKRET(MPI_Isend(x.values.data(), 1, send_type, rank, send_tag, this->comm, &requests[rank]));
            // CHKRET(MPI_Recv(x.values.data(), 1, recv_type, rank, recv_tag, this->comm, MPI_STATUS_IGNORE));
            // MPI_Wait(&requests[rank], MPI_STATUS_IGNORE);
            // DEBUG_ORDERED("OK");
        }
        MPI_Waitall(send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);
        MPI_Waitall(recv_requests.size(), recv_requests.data(), MPI_STATUSES_IGNORE);

        


    }

    // MCSR(const MPI_Comm comm, const int global_size, const int local_begin, const int local_size, const std::vector<int> &coo_rows, const std::vector<int> &coo_cols, const std::vector<double> &coo_data)
    // {
    //     ASSERT_EQ(coo_rows.size(), coo_cols.size());
    //     ASSERT_EQ(coo_rows.size(), coo_data.size());
    //     this->comm = comm;
    //     MPI_Comm_rank(MPI_COMM_WORLD, &this->myrank);
    //     MPI_Comm_size(MPI_COMM_WORLD, &this->nranks);
    //     this->global_size = global_size;
    //     this->local_begin = local_begin;
    //     this->local_size = local_size;
    //     this->local_nnz = coo_data.size();
    //     this->rank_begins.resize(this->nranks + 1);
    //     this->rank_ends.resize(this->nranks);
    //     this->rank_nnzs.resize(this->nranks);
    //     MPI_Allgather(&this->local_begin, 1, MPI_TYPE<int>(), this->rank_begins.data(), 1, MPI_TYPE<int>(), this->comm);
    //     MPI_Allgather(&this->local_size, 1, MPI_TYPE<int>(), this->rank_ends.data(), 1, MPI_TYPE<int>(), this->comm);
    //     MPI_Allgather(&this->local_nnz, 1, MPI_TYPE<int>(), this->rank_nnzs.data(), 1, MPI_TYPE<int>(), this->comm);
    //     for (int rank = 0; rank < nranks; rank++) {
    //         int disp = this->rank_begins[rank];
    //         int size = this->rank_ends[rank];
    //         ASSERT_LE(disp, 0);
    //         ASSERT_LE(size, 0);
    //         ASSERT_BETWEEN_IE(0, disp + size, this->global_size);
    //         if (rank < nranks - 1)
    //             ASSERT_EQ(disp + size, this->rank_begins[rank + 1]);
    //     }
    //     this->rank_begins.back() = this->global_size;
    //     int nrows, ncols, Istart, Jstart;
    //     int *Ap = NULL;
    //     int *Aj = NULL;
    //     double *Av = NULL;
    //     matconvert(this->local_nnz, coo_rows.data(), coo_cols.data(), coo_data.data(), &nrows, &ncols, &Istart, &Jstart, &Ap, &Aj, &Av, MatrixType::COO, MatrixType::CSR);
    //     ASSERT_EQ(nrows, this->local_size);
    //     ASSERT_EQ(nrows, ncols);
    //     ASSERT_EQ(Istart, local_begin);
    //     // rebase
    //     this->I.resize(this->local_size);
    //     this->J.resize(this->local_nnz);
    //     this->V.resize(this->local_nnz);
    //     std::copy(Ap, Ap + this->local_size, this->I.begin());
    //     std::copy(Aj, Aj + this->local_nnz, this->J.begin());
    //     std::copy(Av, Av + this->local_nnz, this->V.begin());
    //     free(Ap);
    //     free(Aj);
    //     free(Av);
    //     // 构造this->external_indexs，它装的是向量后面那段external的数据，各自在全局里是什么下标
    //     std::vector<int> recv_count(this->nranks); // how much I have to recv from these other ranks
    //     std::vector<int> send_count(this->nranks); // how much I have to send to these other ranks
    //     MPI_Alltoall(recv_count.data(), 1, MPI_TYPE<int>(), send_count.data(), 1, MPI_TYPE<int>(), this->comm);
    //     {
    //         this->send_others.resize(this->nranks);
    //         size_t begin = 0;
    //         std::vector<MPI_Request> requests(this->nranks, MPI_REQUEST_NULL);
    //         for (int rank = 0; rank < this->nranks; rank++) {
    //             size_t end = recv_count[rank];
    //             size_t size = end - begin;
    //             MPI_Isend(&this->external_indexs[begin], size, MPI_TYPE<int>(), rank, 0, this->comm, &requests[rank]);
    //         }
    //         for (int rank = 0; rank < this->nranks; rank++) {
    //             std::vector<int> &sendto = send_others[rank];
    //             sendto.resize(send_count[rank]);
    //             MPI_Recv(sendto.data(), sendto.size(), MPI_TYPE<int>(), rank, 0, this->comm, MPI_STATUS_IGNORE);
    //         }
    //         MPI_Waitall(this->nranks, requests.data(), MPI_STATUSES_IGNORE);
    //         for (int i = 0; i < send_others.size(); i++) {
    //             for (int j = 0; j < send_others[i].size(); j++)
    //                 send_others[i][j] -= this->local_begin;
    //         }
    //     }
    //     // FIXME: Get the index list to send each rank.
    //     std::vector<std::vector<int>> recv_index(this->nranks);
    //     std::vector<MPI_Request> init_requests(this->nranks, MPI_REQUEST_NULL);
    //     std::vector<MPI_Request> isend_requests(this->nranks, MPI_REQUEST_NULL);
    //     for (size_t rank = 0; rank < this->nranks; rank++) {
    //         std::vector<int> &rf = recv_index[rank];
    //         MPI_Isend(rf.data(), rf.size(), MPI_TYPE<int>(), rank, 0, this->comm, &isend_requests[rank]);
    //         MPI_Irecv(rf.data(), rf.size(), MPI_TYPE<int>(), rank, 0, this->comm, &isend_requests[rank]);
    //     }
    //     MPI_Waitall(this->nranks, isend_requests.data(), MPI_STATUS_IGNORE);
    //     this->guard.set();
    // }

    ~MCSR() {
        this->comm = MPI_COMM_NULL;
        this->global_size = -1;
        this->local_begin = -1;
        this->local_end = -1;
        this->local_nnz = -1;
        this->rank_begins.clear();
        this->rank_ends.clear();
        this->I.clear();
        this->J.clear();
        this->V.clear();
        this->guard.reset();
    }

    void ResizeVectorForHalo(VCSR *x, VCSR *b) const
    {
        this->guard.ensure();
        if (x != NULL) {
            x->guard.ensure();
            int old = x->values.size();
            x->values.resize(x->values.size() + halo_total_size, 0);
            // DEBUG(0, "Resized from %d to %d", old, this->values.size());
        }
        if (b != NULL) {
            b->guard.ensure();
            int old = b->values.size();
            b->values.resize(b->values.size() + halo_total_size, 0);
            // DEBUG(0, "Resized from %d to %d", old, this->values.size());
        }
    }

    // void CreateVecs(VCSR &x, VCSR &b) const {
    //     this->CreateSol(x);
    //     this->CreateRHS(b);
    // }
    // void CreateSol(VCSR &x) const {
    //     // FIXME: the external part, see MiniFE
    //     x.global_size = this->global_size;
    //     x.local_size = this->local_size;
    //     x.local_begin = this->local_begin;
    //     x.values.resize(this->local_size);
    //     x.guard.set();
    // }
    // void CreateRHS(VCSR &b) const {
    //     b.global_size = this->global_size;
    //     b.local_size = this->local_size;
    //     b.local_begin = this->local_begin;
    //     b.values.resize(this->local_size);
    //     b.guard.set();
    // }

    double Norm() {
        double norm = 0;
        for (size_t i = 0; i < this->V.size(); i++) {
            norm += this->V[i] * this->V[i];
        }
        return std::sqrt(norm);
    }

    virtual void Apply(const VCSR &x, VCSR &y, bool xzero = false) const {
        this->guard.ensure();
        x.guard.ensure();
        y.guard.ensure();
        ASSERT_EQ(x.values.size(), y.values.size());

        int local_size = this->local_end - this->local_begin; // without halo
        for (int i = 0; i < local_size; i++) {
            const int rowstart = this->I[i];
            const int rowstop = this->I[i + 1];
            y.values[i] = 0;
            for (int jj = rowstart; jj < rowstop; jj++) {
                const int j = this->J[jj];
                const double v = this->V[jj];
                y.values[i] += v * x.values[j];
            }
        }
    }
    virtual VCSR GetDiagonals() const {
        int local_size = this->local_end - this->local_begin; // without halo
        std::vector<double> values;
        int count = 0;
        int dc = 0;
        ASSERT_NE(this->I.size(), 0);
        for (int i = 0; i < local_size; i++) {
            const int rowstart = this->I[i];
            const int rowstop = this->I[i + 1];
            for (int jj = rowstart; jj < rowstop; jj++) {
                const int j = this->J[jj];
                const double v = this->V[jj];
                if (j == i) {
                    if (dc < 27) {
                        // DEBUG(1, ">> i %d j %d %f", i, j, v);
                        dc += 1;
                    }
                    values.push_back(v);
                    count += 1;
                }
            }
        }
        ASSERT_EQ(count, local_size);
        VCSR diag(this->global_size, local_size, this->local_begin, values.data());
        this->ResizeVectorForHalo(&diag, NULL);
        return diag;
    }
    virtual const char *GetName() const {
        return "MCSR";
    }

};
