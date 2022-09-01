#ifndef GRAPES_PAR_STRUCT_VEC_HPP
#define GRAPES_PAR_STRUCT_VEC_HPP

#include "common.hpp"
#include "comm_pkg.hpp"
#include "seq_struct_mv.hpp"

template<typename idx_t, typename data_t>
class par_structVector {
public:
    idx_t global_size_x;// 全球lon方向的格点数
    idx_t global_size_y;// 全球lat方向的格点数
    idx_t global_size_z;// 全球垂直方向的格点数

    idx_t offset_x;// 该向量在全球位置中的lon方向偏移
    idx_t offset_y;// 该向量在全球位置中的lat方向偏移
    idx_t offset_z;// 该向量在全球位置中的垂直方向偏移

    seq_structVector<idx_t, data_t> * local_vector = nullptr;

    // 通信相关的
    // 真的需要在每个向量里都有一堆通信的东西吗？虽然方便了多重网格的搞法，但能不能多个同样规格的向量实例共用一套？
    // 可以参考hypre等的实现，某个向量own了这些通信的，然后可以在拷贝构造函数中“外借”出去，析构时由拥有者进行销毁
    StructCommPackage * comm_pkg = nullptr;
    bool own_comm_pkg = false;

    par_structVector(MPI_Comm comm, idx_t gx, idx_t gy, idx_t gz, idx_t num_proc_x, idx_t num_proc_y);
    // 按照model的规格生成一个结构化向量，浅拷贝通信包
    par_structVector(const par_structVector & model);
    ~par_structVector();

    void update_halo() const;
    void set_halo(data_t val) const;
    void set_val(data_t val);
};

/*
 * * * * * par_structVector * * * * *  
 */

template<typename idx_t, typename data_t>
par_structVector<idx_t, data_t>::par_structVector(MPI_Comm comm,
    idx_t gx, idx_t gy, idx_t gz, idx_t num_proc_x, idx_t num_proc_y)
    : global_size_x(gx), global_size_y(gy), global_size_z(gz)
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
    local_vector = new seq_structVector<idx_t, data_t>
        (global_size_x / num_proc_x, global_size_y / num_proc_y, global_size_z, 1, 1, 0);
    
    // 建立通信结构：注意data的排布从内到外依次为k(2)->i(1)->j(0)，按照C-order
    if     (sizeof(data_t) == 8)    mpi_scalar_type = MPI_DOUBLE;
    else if (sizeof(data_t) == 4)   mpi_scalar_type = MPI_FLOAT;
    else if (sizeof(data_t) == 2)   mpi_scalar_type = MPI_SHORT;
    else { printf("INVALID data_t when creating subarray, sizeof %d bytes\n", sizeof(data_t)); MPI_Abort(MPI_COMM_WORLD, -2001); }
    
    idx_t size[3] = {   local_vector->local_y + 2 * local_vector->halo_y,
                        local_vector->local_x + 2 * local_vector->halo_x,
                        local_vector->local_z + 2 * local_vector->halo_z  };
    idx_t subsize[3], send_start[3], recv_start[3];
    for (idx_t ingb = 0; ingb < NUM_NEIGHBORS; ingb++) {
        switch (ingb)
        {
        // 东西向传不带对角位置的
        case EAST:
        case WEST:
            subsize[0] = local_vector->local_y;
            subsize[1] = local_vector->halo_x;
            break;
        case SOUTH:
        case NORTH:
            subsize[0] = local_vector->halo_y;
            subsize[1] = local_vector->local_x + 2 * local_vector->halo_x;
            break;
        default:
            printf("INVALID NEIGHBOR ID of %d!\n", ingb);
            MPI_Abort(MPI_COMM_WORLD, -2000);
        }
        // 最内维的高度层的通信长度不变
        subsize[2] = local_vector->local_z + 2 * local_vector->halo_z;

        switch (ingb)
        {
        case EAST:
            send_start[0] = local_vector->halo_y                        ; send_start[1] = local_vector->local_x                       ;
            recv_start[0] = local_vector->halo_y                        ; recv_start[1] = local_vector->local_x + local_vector->halo_x;
            break;
        case WEST:
            send_start[0] = local_vector->halo_y                        ; send_start[1] = local_vector->halo_x;
            recv_start[0] = local_vector->halo_y                        ; recv_start[1] = 0                   ;
            break;
        case SOUTH:
            send_start[0] = local_vector->halo_y                        ; send_start[1] = 0;
            recv_start[0] = 0                                           ; recv_start[1] = 0;
            break;
        case NORTH:
            send_start[0] = local_vector->local_y                       ; send_start[1] = 0;
            recv_start[0] = local_vector->local_y + local_vector->halo_y; recv_start[1] = 0;
            break;
        default:
            printf("INVALID NEIGHBOR ID of %d!\n", ingb);
            MPI_Abort(MPI_COMM_WORLD, -2000);
        }
        // 最内维的高度层的通信起始位置不变
        send_start[2] = 0;
        recv_start[2] = 0;

        MPI_Type_create_subarray(3, size, subsize, send_start, MPI_ORDER_C, mpi_scalar_type, &send_subarray[ingb]);
        MPI_Type_commit(&send_subarray[ingb]);
        MPI_Type_create_subarray(3, size, subsize, recv_start, MPI_ORDER_C, mpi_scalar_type, &recv_subarray[ingb]);
        MPI_Type_commit(&recv_subarray[ingb]);
    }
}

template<typename idx_t, typename data_t>
par_structVector<idx_t, data_t>::par_structVector(const par_structVector & model)
    : global_size_x(model.global_size_x), global_size_y(model.global_size_y), global_size_z(model.global_size_z),
      offset_x(model.offset_x), offset_y(model.offset_y), offset_z(model.offset_z)
{
    local_vector = new seq_structVector<idx_t, data_t>(*(model.local_vector));
    // 浅拷贝
    comm_pkg = model.comm_pkg;
    own_comm_pkg = false;
}

template<typename idx_t, typename data_t>
par_structVector<idx_t, data_t>::~par_structVector()
{
    delete local_vector;
    if (own_comm_pkg)
        delete comm_pkg;
}

template<typename idx_t, typename data_t>
void par_structVector<idx_t, data_t>::update_halo() const
{
#ifdef DEBUG
    local_vector->init_debug(offset_x, offset_y, offset_z);
    if (my_pid == 1) {
        local_vector->print_level(1);
    }
#endif

    comm_pkg->exec_comm(local_vector->data);

#ifdef DEBUG
    if (my_pid == 1) {
        local_vector->print_level(1);
    }
#endif

    // 如果lat方向的宽度比1大，需要做lat向的数据reverse
    assert(local_vector->halo_y == 1);
}

template<typename idx_t, typename data_t>
void par_structVector<idx_t, data_t>::set_halo(data_t val) const
{
    local_vector->set_halo(val);
}

// val -> v[...]
template<typename idx_t, typename data_t>
void par_structVector<idx_t, data_t>::set_val(const data_t val) {
    *(local_vector) = val;
}

/*
 * * * * * Vector Ops * * * * *  
 */
// TODO: 向量点积也可以放在iterative solver里实现
template<typename idx_t, typename data_t, typename res_t>
res_t vec_dot(par_structVector<idx_t, data_t> const & x, par_structVector<idx_t, data_t> const & y)
{   
    assert( x.global_size_x == y.global_size_x && x.global_size_y == y.global_size_y && 
            x.global_size_z == y.global_size_z && 
            x.offset_x == y.offset_x && x.offset_y == y.offset_y && x.offset_z == y.offset_z);
    assert(sizeof(res_t) == 8 || sizeof(res_t) == 4);

    res_t loc_prod, glb_prod;
    seq_vec_dot(*(x.local_vector), *(y.local_vector), &loc_prod, sizeof(loc_prod));

#ifdef DEBUG
    printf("proc %3d halo_x/y/z %d %d %d local_x/y/z %d %d %d loc_proc %10.7e\n", 
        x.my_pid,
        x.local_vector->halo_x, x.local_vector->halo_y, x.local_vector->halo_z,
        x.local_vector->local_x, x.local_vector->local_y, x.local_vector->local_z, loc_prod);
#endif

    if (sizeof(res_t) == 8)
        MPI_Allreduce(&loc_prod, &glb_prod, 1, MPI_DOUBLE, MPI_SUM, x.comm_pkg->cart_comm);
    else 
        MPI_Allreduce(&loc_prod, &glb_prod, 1, MPI_FLOAT , MPI_SUM, x.comm_pkg->cart_comm);
    
    return glb_prod;
}

// v1 + alpha * v2 -> v
template<typename idx_t, typename data_t, typename scalar_t>
void vec_add(const par_structVector<idx_t, data_t> & v1, scalar_t alpha, 
             const par_structVector<idx_t, data_t> & v2, par_structVector<idx_t, data_t> & v)
{
    assert( v1.global_size_x == v2.global_size_x && v2.global_size_x == v.global_size_x &&
            v1.global_size_y == v2.global_size_y && v2.global_size_y == v.global_size_y &&
            v1.global_size_z == v2.global_size_z && v2.global_size_z == v.global_size_z    );

    seq_vec_add(*(v1.local_vector), alpha, *(v2.local_vector), *(v.local_vector));    
}



// src -> dst
template<typename idx_t, typename data_t>
void vec_copy(const par_structVector<idx_t, data_t> & src, par_structVector<idx_t, data_t> & dst) {
    assert( src.global_size_x == dst.global_size_x &&
            src.global_size_y == dst.global_size_y &&
            src.global_size_z == dst.global_size_z    );

    seq_vec_copy(*(src.local_vector), *(dst.local_vector));
}

// coeff * src -> dst
template<typename idx_t, typename data_t, typename scalar_t>
void vec_mul_by_scalar(const scalar_t coeff, const par_structVector<idx_t, data_t> & src, par_structVector<idx_t, data_t> & dst) {
    assert( src.global_size_x == dst.global_size_x &&
            src.global_size_y == dst.global_size_y &&
            src.global_size_z == dst.global_size_z    );

    seq_vec_mul_by_scalar(coeff, *(src.local_vector), *(dst.local_vector));
}

// vec *= coeff
template<typename idx_t, typename data_t, typename scalar_t>
void vec_scale(const scalar_t coeff, par_structVector<idx_t, data_t> & vec) {
    seq_vec_scale(coeff, *(vec.local_vector));
}

#endif