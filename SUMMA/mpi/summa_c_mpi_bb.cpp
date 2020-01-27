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

void showMatrix(std::vector<std::vector<double>> mat, int m, int n, int mb, int nb, int rank, std::string name){

    std::cout << std::setprecision(8);
    std::cout << std::scientific;

    for(int p = 0; p < mb; p++){
	for(int q = 0; q < nb; q++){
	   for(std::size_t i = 0; i < m; i++){
               std::cout << name << " on [" << rank << "]> " << "(" << p << ", "<< q << ")" << " row " << i << ":" ;
                   for(std::size_t j = 0; j < n; j++){
                       std::cout << std::right << std::setw(12) << mat[p * nb + j][i * n + j] <<" ";
         	   }
        	   std::cout << std::endl;
    	   }
        }
    }
}

int main(int argc, char** argv) {

    using namespace boost::program_options;

    options_description desc_cmdline;

    desc_cmdline.add_options()
        ("size,s", value<int>()->default_value(4), "Matrix size.")
        ("bsize,b", value<int>()->default_value(2), "Block size.")
        ("debug", value<std::string>()->default_value("no"), "debug mode")
        ("repetitions,r", value<int>()->default_value(1), "Number of repetitions.");

    variables_map vm;

    store(parse_command_line(argc, argv, desc_cmdline),vm);

    int s = vm["size"].as<int>();
    int bsize = vm["bsize"].as<int>();
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

    int bn = (int)(sb / bsize);

    std::vector<std::vector<double>> A(bn * bn);
    std::vector<std::vector<double>> B(bn * bn);
    std::vector<std::vector<double>> C(bn * bn);

    for(int i = 0; i < bn * bn; i++){
	A[i].resize(bsize * bsize);
	std::fill(A[i].begin(), A[i].end(), (double)(1));
    }

    for(int i = 0; i < bn * bn; i++){
        B[i].resize(bsize * bsize);
        std::fill(B[i].begin(), B[i].end(), (double)(2));
    }

    for(int i = 0; i < bn * bn; i++){
        C[i].resize(bsize * bsize);
        std::fill(C[i].begin(), C[i].end(), 0.0);
    }

    if(debug == "yes"){
	showMatrix(A, bsize, bsize, bn, bn, rank, "A");
        showMatrix(B, bsize, bsize, bn, bn, rank, "B");
    }

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<double>> A_save = A;
    std::vector<std::vector<double>> B_save = B;

    int bcast_root;
    
    int k;

    for(k = 0; k < s; k += bsize){
	bcast_root = (int)(k / sb);
	for(int i = 0; i < bn * bn; i++){
	    std::vector<double> A_loc(bsize * bsize);
	    if(coord[1] == bcast_root){
                A_loc = A_save[i];
            }else{
                std::fill(A_loc.begin(), A_loc.end(), 0.0);
            }

	    MPI_Bcast(A_loc.data(), bsize * bsize, MPI_DOUBLE, bcast_root, rowComm);

	    std::vector<double> B_loc(bsize * bsize);
            if(coord[0] == bcast_root){
                B_loc = B_save[i];
            }else{
                std::fill(B_loc.begin(), B_loc.end(), 0.0);
            }
            MPI_Bcast(B_loc.data(), bsize * bsize, MPI_DOUBLE, bcast_root, colComm);

	    dgemm("N", "N", &bsize, &bsize, &bsize, &alpha, A_loc.data(), &bsize, B_loc.data(), &bsize, &beta, C[i].data(), &bsize);
        }
    }

    if(debug == "yes"){
        showMatrix(C, bsize, bsize, bn, bn, rank, "C");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    if(rank == 0){
	std::cout << "| SUMMA MPI BB| " << nprocs << " | " << s << " | " << bsize << " | " << elapsed.count() << " | " << std::endl;
    }

    MPI_Finalize();
 
    return 0;
}
