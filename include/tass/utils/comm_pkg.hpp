#ifndef GRAPES_COMM_PKG_HPP
#define GRAPES_COMM_PKG_HPP

#include "common.hpp"

class StructCommPackage {
public:
    MPI_Comm cart_comm = MPI_COMM_NULL;
    int cart_ids[2];
    int my_pid;
    int ngbs_pid[NUM_NEIGHBORS];
    MPI_Datatype send_subarray[NUM_NEIGHBORS], recv_subarray[NUM_NEIGHBORS];
    MPI_Datatype mpi_scalar_type;

    StructCommPackage() {}
    void exec_comm(void * data);
    ~StructCommPackage();
};

void StructCommPackage::exec_comm(void * data) {
    MPI_Status status;
    // 向东发，同时从西收
    MPI_Sendrecv(   data, 1, send_subarray[EAST], ngbs_pid[EAST], my_pid ^ ngbs_pid[EAST],
                    data, 1, recv_subarray[WEST], ngbs_pid[WEST], my_pid ^ ngbs_pid[WEST], cart_comm, &status);
    // 向西发，同时从东收
    MPI_Sendrecv(   data, 1, send_subarray[WEST], ngbs_pid[WEST], my_pid ^ ngbs_pid[WEST],
                    data, 1, recv_subarray[EAST], ngbs_pid[EAST], my_pid ^ ngbs_pid[EAST], cart_comm, &status);

    // 二踢脚的通信方式，必须等东西向传完了，才能传南北向
    // 南北向必须用非阻塞
    MPI_Request send_req[2], recv_req[2];
    MPI_Status  send_status[2], recv_status[2];

    MPI_Isend(data, 1, send_subarray[NORTH], ngbs_pid[NORTH], my_pid ^ ngbs_pid[NORTH], cart_comm, &send_req[0]);
    MPI_Isend(data, 1, send_subarray[SOUTH], ngbs_pid[SOUTH], my_pid ^ ngbs_pid[SOUTH], cart_comm, &send_req[1]);

    MPI_Irecv(data, 1, recv_subarray[NORTH], ngbs_pid[NORTH], my_pid ^ ngbs_pid[NORTH], cart_comm, &recv_req[0]);
    MPI_Irecv(data, 1, recv_subarray[SOUTH], ngbs_pid[SOUTH], my_pid ^ ngbs_pid[SOUTH], cart_comm, &recv_req[1]);

    MPI_Waitall(2, send_req, send_status);
    MPI_Waitall(2, recv_req, recv_status);
}

StructCommPackage::~StructCommPackage()
{
    for (int ingb = 0; ingb < NUM_NEIGHBORS; ingb++) {
        if (send_subarray[ingb] != MPI_DATATYPE_NULL)
            MPI_Type_free(&send_subarray[ingb]);
        if (recv_subarray[ingb] != MPI_DATATYPE_NULL)
            MPI_Type_free(&recv_subarray[ingb]);
    }
}


#endif