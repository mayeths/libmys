#ifndef GRAPES_PAR_STRUCT_MV_HPP
#define GRAPES_PAR_STRUCT_MV_HPP

#include "common.hpp"
#include "par_struct_vec.hpp"
#include "operator.hpp"

template<typename idx_t, typename data_t>
class par_structMatrix : public Operator<idx_t, data_t>  {
public:
    idx_t num_diag;

    idx_t offset_x;// 在全球位置中的lon方向偏移
    idx_t offset_y;// 在全球位置中的lat方向偏移
    idx_t offset_z;// 在全球位置中的垂直方向偏移

    seq_structMatrix<idx_t, data_t> * local_matrix;

    // 通信相关的
    StructCommPackage * comm_pkg = nullptr;
    bool own_comm_pkg = false;

    par_structMatrix(MPI_Comm comm, idx_t num_d, idx_t gx, idx_t gy, idx_t gz, idx_t num_proc_x, idx_t num_proc_y);
    // 按照model的规格生成一个结构化向量，浅拷贝通信包
    par_structMatrix(const par_structMatrix & model);
    ~par_structMatrix();

    void update_halo();
    void Mult(const par_structVector<idx_t, data_t> & x, par_structVector<idx_t, data_t> & y) const ;
};


/*
 * * * * * par_structMatrix * * * * *  
 */

template<typename idx_t, typename data_t>
par_structMatrix<idx_t, data_t>::par_structMatrix(MPI_Comm comm, idx_t num_d,
    idx_t global_size_x, idx_t global_size_y, idx_t global_size_z, idx_t num_proc_x, idx_t num_proc_y)
    : Operator<idx_t, data_t>(global_size_x, global_size_y, global_size_z, global_size_x, global_size_y, global_size_z), 
        num_diag(num_d)
{
    // for GMG concern: must be 2^m for periodic domain
    // assert(check_power2(global_size_x));
    // assert(check_power2(global_size_y));
    // for GMG concern: must be fully divided by processors
    assert(global_size_x % num_proc_x == 0);
    assert(global_size_y % num_proc_y == 0);
    // currently only supports even number of processes on lon-direction
    assert(num_proc_x % 2 == 0);

    comm_pkg = new StructCommPackage;
    own_comm_pkg = true;
    // 对comm_pkg内变量的引用，免得写太麻烦了
    MPI_Comm & cart_comm                         = comm_pkg->cart_comm;
    int (&cart_ids)[2]                           = comm_pkg->cart_ids;
    int (&ngbs_pid)[NUM_NEIGHBORS]               = comm_pkg->ngbs_pid;
    int & my_pid                                 = comm_pkg->my_pid;
    MPI_Datatype (&send_subarray)[NUM_NEIGHBORS] = comm_pkg->send_subarray;
    MPI_Datatype (&recv_subarray)[NUM_NEIGHBORS] = comm_pkg->recv_subarray;
    MPI_Datatype & mpi_scalar_type               = comm_pkg->mpi_scalar_type;

    // create 2D distributed grid
    idx_t dims[2] = {num_proc_y, num_proc_x};
    idx_t periods[2] = {0, 1};

    MPI_Cart_create(comm, 2, dims, periods, 0, &cart_comm);
    assert(cart_comm != MPI_COMM_NULL);

    //          north:3                                        ^
    //  west:1   MY location  east:0  <->   (direction #1)     | (direction #0)  笛卡尔坐标(#0, #1)
    //          south:2                                        √
    MPI_Cart_shift(cart_comm, 0, 1, &ngbs_pid[SOUTH], &ngbs_pid[NORTH]);
    MPI_Cart_shift(cart_comm, 1, 1, &ngbs_pid[WEST ], &ngbs_pid[EAST ]);

    MPI_Comm_rank(cart_comm, &my_pid);
    MPI_Cart_coords(cart_comm, my_pid, 2, cart_ids);
    // 特别处理南北极的进程
    if (cart_ids[0] == 0) {// 在南极
        assert(ngbs_pid[SOUTH] == MPI_PROC_NULL);
        IDX_TYPE tmp = cart_ids[1] + num_proc_x / 2;// cart_ids[1] 标记了该进程在东西方向上的相对位置
        tmp = (tmp > num_proc_x - 1) ? (tmp % num_proc_x) : tmp;
        ngbs_pid[SOUTH] = cart_ids[0] * num_proc_x + tmp;
    }
    if (cart_ids[0] == num_proc_y - 1) {// 在北极
        assert(ngbs_pid[NORTH] == MPI_PROC_NULL);
        IDX_TYPE tmp = cart_ids[1] + num_proc_x / 2;// cart_ids[1] 标记了该进程在东西方向上的相对位置
        tmp = (tmp > num_proc_x - 1) ? (tmp % num_proc_x) : tmp;
        ngbs_pid[NORTH] = cart_ids[0] * num_proc_x + tmp;
    }

#ifdef DEBUG
    printf("proc %3d cart_ids (%3d, %3d) east %3d west %3d south %3d north %3d\n", my_pid, cart_ids[0], cart_ids[1],
        ngbs_pid[EAST], ngbs_pid[WEST], ngbs_pid[SOUTH], ngbs_pid[NORTH]);
#endif

    offset_x = cart_ids[1] * global_size_x / num_proc_x;
    offset_y = cart_ids[0] * global_size_y / num_proc_y;
    offset_z = 0;

    // 建立本地数据的内存
    local_matrix = new seq_structMatrix<idx_t, data_t>
        (num_diag, global_size_x / num_proc_x, global_size_y / num_proc_y, global_size_z, 1, 1, 0);
    
    // 建立通信结构：注意data的排布从内到外依次为diag(3)->k(2)->i(1)->j(0)，按照C-order
    if     (sizeof(data_t) == 8)    mpi_scalar_type = MPI_DOUBLE;
    else if (sizeof(data_t) == 4)   mpi_scalar_type = MPI_FLOAT;
    else if (sizeof(data_t) == 2)   mpi_scalar_type = MPI_SHORT;
    else { printf("INVALID data_t when creating subarray, sizeof %d bytes\n", sizeof(data_t)); MPI_Abort(MPI_COMM_WORLD, -2001); }

    idx_t size[4] = {   local_matrix->local_y + 2 * local_matrix->halo_y,
                        local_matrix->local_x + 2 * local_matrix->halo_x,
                        local_matrix->local_z + 2 * local_matrix->halo_z,
                        local_matrix->num_diag  };
    idx_t subsize[4], send_start[4], recv_start[4];
    for (idx_t ingb = 0; ingb < NUM_NEIGHBORS; ingb++) {
        switch (ingb)
        {
        // 东西向传不带对角位置的
        case EAST:
        case WEST:
            subsize[0] = local_matrix->local_y;
            subsize[1] = local_matrix->halo_x;
            break;
        case SOUTH:
        case NORTH:
            subsize[0] = local_matrix->halo_y;
            subsize[1] = local_matrix->local_x + 2 * local_matrix->halo_x;
            break;
        default:
            printf("INVALID NEIGHBOR ID of %d!\n", ingb);
            MPI_Abort(MPI_COMM_WORLD, -2000);
        }
        // 最内维的高度层和矩阵元素层的通信长度不变
        subsize[2] = local_matrix->local_z + 2 * local_matrix->halo_z;
        subsize[3] = local_matrix->num_diag;

        switch (ingb)
        {
        case EAST:
            send_start[0] = local_matrix->halo_y                        ; send_start[1] = local_matrix->local_x                       ;
            recv_start[0] = local_matrix->halo_y                        ; recv_start[1] = local_matrix->local_x + local_matrix->halo_x;
            break;
        case WEST:
            send_start[0] = local_matrix->halo_y                        ; send_start[1] = local_matrix->halo_x;
            recv_start[0] = local_matrix->halo_y                        ; recv_start[1] = 0                   ;
            break;
        case SOUTH:
            send_start[0] = local_matrix->halo_y                        ; send_start[1] = 0;
            recv_start[0] = 0                                           ; recv_start[1] = 0;
            break;
        case NORTH:
            send_start[0] = local_matrix->local_y                       ; send_start[1] = 0;
            recv_start[0] = local_matrix->local_y + local_matrix->halo_y; recv_start[1] = 0;
            break;
        default:
            printf("INVALID NEIGHBOR ID of %d!\n", ingb);
            MPI_Abort(MPI_COMM_WORLD, -2000);
        }
        // 最内维的高度层的通信起始位置不变
        send_start[2] = 0; send_start[3] = 0;
        recv_start[2] = 0; recv_start[3] = 0;

        MPI_Type_create_subarray(4, size, subsize, send_start, MPI_ORDER_C, mpi_scalar_type, &send_subarray[ingb]);
        MPI_Type_commit(&send_subarray[ingb]);
        MPI_Type_create_subarray(4, size, subsize, recv_start, MPI_ORDER_C, mpi_scalar_type, &recv_subarray[ingb]);
        MPI_Type_commit(&recv_subarray[ingb]);
    }
}

template<typename idx_t, typename data_t>
par_structMatrix<idx_t, data_t>::par_structMatrix(const par_structMatrix & model) 
    : Operator<idx_t, data_t>(model.input_dim[0], model.input_dim[1], model.input_dim[2], 
                              model.output_dim[0], model.output_dim[1], model.output_dim[2]),
      offset_x(model.offset_x), offset_y(model.offset_y), offset_z(model.offset_z)
{
    local_matrix = new seq_structMatrix<idx_t, data_t>(*(model.local_matrix));
    // 浅拷贝
    comm_pkg = model.comm_pkg;
    own_comm_pkg = false;
}

template<typename idx_t, typename data_t>
par_structMatrix<idx_t, data_t>::~par_structMatrix()
{
    delete local_matrix;
    if (own_comm_pkg)
        delete comm_pkg;
}

template<typename idx_t, typename data_t>
void par_structMatrix<idx_t, data_t>::update_halo()
{
#ifdef DEBUG
    local_matrix->init_debug(offset_x, offset_y, offset_z);
    if (my_pid == 1) {
        local_matrix->print_level_diag(1, 3);
    }
#endif

    comm_pkg->exec_comm(local_matrix->data);

#ifdef DEBUG
    if (my_pid == 1) {
        local_matrix->print_level_diag(1, 3);
    }
#endif

    // 如果lat方向的宽度比1大，需要做lat向的数据reverse
    assert(local_matrix->halo_y == 1);
}

template<typename idx_t, typename data_t>
void par_structMatrix<idx_t, data_t>::Mult(const par_structVector<idx_t, data_t> & x, par_structVector<idx_t, data_t> & y) const
{
    assert( this->input_dim[0] == x.global_size_x && this->output_dim[0] == y.global_size_x &&
            this->input_dim[1] == x.global_size_y && this->output_dim[1] == y.global_size_y &&
            this->input_dim[2] == x.global_size_z && this->output_dim[2] == y.global_size_z    );

    // lazy halo updated: only done when needed 
    x.update_halo();
    
    // do computation
    local_matrix->Mult(*(x.local_vector), *(y.local_vector));
}

#endif