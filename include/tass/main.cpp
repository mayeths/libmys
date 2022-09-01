#include "utils/par_struct_mat.hpp"
#include "utils/IO.hpp"
#include "Solver_ls.hpp"

int main(int argc, char ** argv)
{
    MPI_Init(&argc, &argv);

    int num_procs, my_pid;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_pid);
    // 二维划分
    int num_proc_x = atoi(argv[1]);
    int num_proc_y = atoi(argv[2]);

    {
        // par_structVector<IDX_TYPE, KSP_TYPE> x (MPI_COMM_WORLD,     360, 180, 62, num_proc_x, num_proc_y);
        // par_structMatrix<IDX_TYPE, KSP_TYPE> A (MPI_COMM_WORLD, 19, 360, 180, 62, num_proc_x, num_proc_y);
        // const char * pathname = "/storage/hpcauser/huanghp/project/GRAPES-bluegene/data/100km";

        // par_structVector<IDX_TYPE, KSP_TYPE> x (MPI_COMM_WORLD,     720, 360, 62, num_proc_x, num_proc_y);
        // par_structMatrix<IDX_TYPE, KSP_TYPE> A (MPI_COMM_WORLD, 19, 720, 360, 62, num_proc_x, num_proc_y);
        // const char * pathname = "/storage/hpcauser/huanghp/project/GRAPES-bluegene/data/50km";

        par_structVector<IDX_TYPE, KSP_TYPE> x (MPI_COMM_WORLD,     1440, 720, 62, num_proc_x, num_proc_y);
        par_structMatrix<IDX_TYPE, KSP_TYPE> A (MPI_COMM_WORLD, 19, 1440, 720, 62, num_proc_x, num_proc_y);
        const char * pathname = "/storage/hpcauser/huanghp/project/GRAPES-bluegene/data/25km";

        par_structVector<IDX_TYPE, KSP_TYPE> b(x);

        // read_initial("./data", A, x, b);
        read_initial(pathname, A, x, b);

        // 打印一柱出来看看
        // IDX_TYPE i = 45, j = 40;
        // if (my_pid == 0) {
        //     printf("14         0          9\n");
        //     const seq_structMatrix<IDX_TYPE, KSP_TYPE> & mat = *(A.local_matrix);
        //     for (int k = 0; k < 62; k++) 
        //         printf("%12.7e %12.7e %12.7e\n",A.local_matrix->data[14 + k * mat.num_diag + i * mat.slice_dk_size + j * mat.slice_dki_size],
        //                                         A.local_matrix->data[ 0 + k * mat.num_diag + i * mat.slice_dk_size + j * mat.slice_dki_size],
        //                                         A.local_matrix->data[ 9 + k * mat.num_diag + i * mat.slice_dk_size + j * mat.slice_dki_size]);
        // }
        
        // PointJacobi<IDX_TYPE, PC_TYPE> pr; 

        // PointGS<IDX_TYPE, PC_TYPE> pr; pr.SetScanType(SYMMETRIC);

        // LineJacobi<IDX_TYPE, PC_TYPE> pr;

        LineGS<IDX_TYPE, PC_TYPE> pr;

        GCRSolver<IDX_TYPE, KSP_TYPE, PC_TYPE> GCR;
        GCR.SetMaxIter(200);
        GCR.SetInnerIterMax(9);
        GCR.SetAbsTol(1e-10);
        GCR.SetPreconditioner(pr);// 对于GCR，
        GCR.SetOperator(A);
        GCR.Mult(b, x);

        // CGSolver<IDX_TYPE, KSP_TYPE, PC_TYPE> CG;
        // CG.SetMaxIter(1000);
        // CG.SetAbsTol(1e-5);
        // CG.SetPreconditioner(GS);
        // CG.SetOperator(A);
        // CG.Mult(b, x);

        // GMRESSolver<IDX_TYPE, KSP_TYPE, PC_TYPE> GMRES;
        // GMRES.SetMaxIter(200);
        // GMRES.SetRestartlen(30);
        // GMRES.SetAbsTol(1e-5);
        // GMRES.SetPreconditioner(Jacobi);
        // GMRES.SetOperator(A);
        // GMRES.Mult(b, x);

        // FGMRESSolver<IDX_TYPE, KSP_TYPE, PC_TYPE> FGMRES;
        // FGMRES.SetMaxIter(200);
        // FGMRES.SetRestartlen(30);
        // FGMRES.SetAbsTol(1e-5);
        // FGMRES.SetPreconditioner(Jacobi);
        // FGMRES.SetOperator(A);
        // FGMRES.Mult(b, x);

        par_structVector<IDX_TYPE, KSP_TYPE> y(x);
        A.Mult(x, y);
        vec_add(b, -1.0, y, y);
        double res = vec_dot<IDX_TYPE, KSP_TYPE, double>(y, y);
        if (my_pid == 0) printf("true resi: %20.16e\n", res);
    }

    MPI_Finalize();
    return 0;
}