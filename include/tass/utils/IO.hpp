#ifndef GRAPES_IO_HPP
#define GRAPES_IO_HPP

#include "par_struct_mat.hpp"
#include "string"

template<typename idx_t, typename data_t>
void read_initial(const std::string pathname, par_structMatrix<idx_t, data_t> & A, 
        par_structVector<idx_t, data_t> & x, par_structVector<idx_t, data_t> & b)
{
    seq_structMatrix<idx_t, data_t> & A_local = *(A.local_matrix);
    seq_structVector<idx_t, data_t> & x_local = *(x.local_vector), & b_local = *(b.local_vector);

    assert( A_local.local_x == x_local.local_x && x_local.local_x == b_local.local_x &&
            A_local.local_y == x_local.local_y && x_local.local_y == b_local.local_y &&
            A_local.local_z == x_local.local_z && x_local.local_z == b_local.local_z  );
    assert( A.comm_pkg->mpi_scalar_type == x.comm_pkg->mpi_scalar_type 
            && x.comm_pkg->mpi_scalar_type == b.comm_pkg->mpi_scalar_type  );
    assert( (sizeof(data_t) == 4 && A.comm_pkg->mpi_scalar_type == MPI_FLOAT)  ||
            (sizeof(data_t) == 8 && A.comm_pkg->mpi_scalar_type == MPI_DOUBLE) ||
            (sizeof(data_t) == 2 && A.comm_pkg->mpi_scalar_type == MPI_SHORT )    );
    assert( A.input_dim[0] == x.global_size_x && A.output_dim[0] == b.global_size_x &&
            A.input_dim[1] == x.global_size_y && A.output_dim[1] == b.global_size_y &&
            A.input_dim[2] == x.global_size_z && A.output_dim[2] == b.global_size_z  );
    assert( A.offset_x == x.offset_x && x.offset_x == b.offset_x &&
            A.offset_y == x.offset_y && x.offset_y == b.offset_y &&
            A.offset_z == x.offset_z && x.offset_z == b.offset_z);
    
    idx_t local_x = A_local.local_x, local_y = A_local.local_y, local_z = A_local.local_z;
    idx_t tot_len = local_x * local_y * local_z;
    data_t * buf = new data_t[tot_len];// 读入缓冲区
    MPI_File fh = MPI_FILE_NULL;// 文件句柄
    MPI_Datatype etype = A.comm_pkg->mpi_scalar_type;
    MPI_Datatype read_type = MPI_DATATYPE_NULL;// 读取时的类型
    
    // 三者可以共用一套向量数据类型
    MPI_Type_vector(b_local.local_y * b_local.local_z, b_local.local_x, b.global_size_x, 
        b.comm_pkg->mpi_scalar_type, &read_type);
    MPI_Type_commit(&read_type);

    MPI_Offset displacement = b.offset_x + b.global_size_x * (b.offset_z + b.global_size_z * b.offset_y);
    displacement *= sizeof(data_t);// 位移要以字节为单位！
    MPI_Status status;

    // FILE * fp = fopen(("b."+std::to_string(b.my_pid)).c_str(), "w+");

    // 读入右端向量
    MPI_File_open(b.comm_pkg->cart_comm, (pathname + "/array_b.new71").c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, displacement, etype, read_type, "native", MPI_INFO_NULL);
    MPI_File_read_all(fh, buf, tot_len, etype, &status);
    // data_t loc_prod = 0.0;
    for (idx_t j = 0; j < local_y; j++)
        for (idx_t k = 0; k < local_z; k++)
            for (idx_t i = 0; i < local_x; i++)
                b_local.data[k + b_local.halo_z + 
                            (i + b_local.halo_x) * b_local.slice_k_size +
                            (j + b_local.halo_y) * b_local.slice_ki_size] 
                = buf[i + local_x * (k + local_z * j)];
    MPI_File_close(&fh);
    
    // 读入初始解向量
    MPI_File_open(x.comm_pkg->cart_comm, (pathname + "/array_x.new71").c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
    MPI_File_set_view(fh, displacement, etype, read_type, "native", MPI_INFO_NULL);
    MPI_File_read_all(fh, buf, tot_len, etype, &status);
    for (idx_t j = 0; j < local_y; j++)
        for (idx_t k = 0; k < local_z; k++)
            for (idx_t i = 0; i < local_x; i++)
                x_local.data[k + x_local.halo_z + 
                            (i + x_local.halo_x) * x_local.slice_k_size +
                            (j + x_local.halo_y) * x_local.slice_ki_size] 
                = buf[i + local_x * (k + local_z * j)];
    MPI_File_close(&fh);

    // 依次读入A的各条对角线
    for (idx_t idiag = 0; idiag < A.num_diag; idiag++) {
        // 注意对角线数据的存储文件命名是从1开始的
        MPI_File_open(A.comm_pkg->cart_comm, (pathname + "/array_a.new" + std::to_string(idiag + 1)).c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
        MPI_File_set_view(fh, displacement, etype, read_type, "native", MPI_INFO_NULL);
        MPI_File_read_all(fh, buf, tot_len, etype, &status);
        for (idx_t j = 0; j < local_y; j++)
            for (idx_t k = 0; k < local_z; k++)
                for (idx_t i = 0; i < local_x; i++)
                    A_local.data[idiag +
                                (k + A_local.halo_z) * A_local.num_diag + 
                                (i + A_local.halo_x) * A_local.slice_dk_size +
                                (j + A_local.halo_y) * A_local.slice_dki_size]
                    = buf[i + local_x * (k + local_z * j)];
        MPI_File_close(&fh);
    }
    // 矩阵需要填充halo区（目前仅此一次）
    A.update_halo();

    delete buf;
}


#endif