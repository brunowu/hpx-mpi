#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <time.h>
#include <iomanip>
#include <chrono>
#include <cstdlib>
#include <boost/program_options.hpp>

#include <mkl.h>

void showMatrix(std::vector<double> mat, int m, int n, int rank, std::string name){

    std::cout << std::setprecision(8);
    std::cout << std::scientific;

    for(std::size_t i = 0; i < m; i++){
         std::cout << name << " on [" << rank << "]> row " << i << ":" ;
         for(std::size_t j = 0; j < n; j++){
                std::cout << std::right << std::setw(12) << mat[i * n + j] <<" ";
         }
        std::cout << std::endl;
    }
}


int main(int argc, char** argv) {

    using namespace boost::program_options;

    options_description desc_cmdline;

    desc_cmdline.add_options()
        ("size,s", value<int>()->default_value(4), "Matrix size (if > 0, overrides m, n, k).")
        ("debug", value<std::string>()->default_value("no"), "debug mode")
        ("repetitions,r", value<int>()->default_value(1), "Number of repetitions.");

    variables_map vm;

    store(parse_command_line(argc, argv, desc_cmdline),vm);

    int s = vm["size"].as<int>();
    int rep = vm["repetitions"].as<int>();

    std::string debug = vm["debug"].as<std::string>();

    MPI_Init(&argc,&argv);

    int nprocs, cpoc_init, rank;

    int rowCommSize, colCommSize, rowCommRank, colCommRank;

    int periodic[] = {0, 0};
    int reorder = 0;
    int free_coords[2];
    int dims[2];
    int coord[2];
   
    const double alpha = 1.0;
    const double beta = 1.0;

    MPI_Comm cartComm, rowComm, colComm;
  
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    int rproc = std::sqrt(nprocs);
    
    if(rproc * rproc - nprocs != 0){
	throw "Support only square 2D grid";
    }


    dims[0] = dims[1] = rproc;

    int sb = (int) (s / rproc);

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periodic, reorder, &cartComm);

    MPI_Comm_size(cartComm, &nprocs);

    MPI_Comm_rank(cartComm, &rank);
 
    MPI_Cart_coords(cartComm, rank, 2, coord);

    free_coords[0] = 0;
    free_coords[1] = 1;

    MPI_Cart_sub(cartComm, free_coords, &rowComm);

    free_coords[0] = 1;
    free_coords[1] = 0;

    MPI_Cart_sub(cartComm, free_coords, &colComm);
   
    MPI_Comm_size(rowComm, &rowCommSize);
    MPI_Comm_rank(rowComm, &rowCommRank);

    MPI_Comm_size(colComm, &colCommSize);
    MPI_Comm_rank(colComm, &colCommRank);

    if(debug == "yes"){
        std::cout << "Rank " << rank << " : (" << coord[0] << ", " << coord[1] << "), "  << "rowCommSize = " << rowCommSize << ", rowCommRank = " << rowCommRank  << " | colCommSize = " << colCommSize << ", colCommRank = " << colCommRank << std::endl;
    }

    std::vector<double> A(sb * sb);
    std::vector<double> B(sb * sb); 
    std::vector<double> C(sb * sb);

    std::fill(A.begin(), A.end(), (double)(rank + 1));
    std::fill(B.begin(), B.end(), (double)(rank + sb * sb + 1));
    std::fill(C.begin(), C.end(), 0.0);
    
    if(debug == "yes"){
        showMatrix(A, sb, sb, rank, "A");
	showMatrix(B, sb, sb, rank, "B");
    }
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    std::vector<double> A_save = A;
    std::vector<double> B_save = B;

    int bcast_root;

    for(bcast_root = 0; bcast_root < rproc; bcast_root++){
	std::vector<double> A_loc(sb * sb);
        if(coord[1] == bcast_root){
	    A_loc = A_save;
        }else{
	    std::fill(A_loc.begin(), A_loc.end(), 0.0);
        }
       
        MPI_Bcast(A_loc.data(), sb * sb, MPI_DOUBLE, bcast_root, rowComm);

	std::vector<double> B_loc(sb * sb);
        if(coord[0] == bcast_root){
            B_loc = B_save;
        }else{
            std::fill(B_loc.begin(), B_loc.end(), 0.0);
        }

        MPI_Bcast(B_loc.data(), sb * sb, MPI_DOUBLE, bcast_root, colComm);

        dgemm("N", "N", &sb, &sb, &sb, &alpha, A_loc.data(), &sb, B_loc.data(), &sb, &beta, C.data(), &sb);
    }

    if(debug == "yes"){
        showMatrix(C, sb, sb, rank, "C");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    if(rank == 0){
	std::cout << "| " << nprocs << " | " << s << " | " << elapsed.count() << " | " << std::endl;
    }

    MPI_Finalize();
 
    return 0;
}
