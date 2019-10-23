#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <cmath>
#include <time.h>
#include <iomanip>
#include <chrono>

#include <boost/program_options.hpp>


void showMatrix(double *mat, int m, int n){

    std::cout << std::endl << "Show " << m << "x" << n << " matrix:" << std::endl;

    std::cout << std::setprecision(8);
    std::cout << std::scientific;

    for(std::size_t i = 0; i < m; i++){
    	 for(std::size_t j = 0; j < n; j++){
	 	std::cout << std::right << std::setw(12) << mat[i * n + j] <<" ";
	 }
	std::cout << std::endl;
    }
}

void showMatrix(double *mat, int m, int n, int rank){

    std::cout << std::endl << rank << "]> " << "Show " << m << " x " << n << " matrix:" << std::endl;

    std::cout << std::setprecision(8);
    std::cout << std::scientific;

    for(std::size_t i = 0; i < m; i++){
         for(std::size_t j = 0; j < n; j++){
                std::cout << std::right << std::setw(12) << mat[i * n + j] <<" ";
         }
        std::cout << std::endl;
    }

}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);

    using namespace boost::program_options;

    options_description desc_cmdline;

    desc_cmdline.add_options()
    	("size,s", value<int>()->default_value(-1), "Matrix size (if > 0, overrides m, n, k).")
        ("debug", value<std::string>()->default_value("no"), "(debug) => print matrices")
    	("repetitions,r", value<int>()->default_value(5), "Number of repetitions.");

    variables_map vm;

    store(parse_command_line(argc, argv, desc_cmdline),vm);

    int s = vm["size"].as<int>();
    int rep = vm["repetitions"].as<int>();
    std::string out = vm["debug"].as<std::string>();
 
    bool debug = false;

    if(out == "yes"){
    	debug = true;
    }

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    int length = (int) s * s;

    double *A = new double [length];

    int rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int root = 0;

    if(rank == root){

       std::chrono::high_resolution_clock::time_point start2 = std::chrono::high_resolution_clock::now();

	for(int i = 0; i < length; i++){
	    A[i] = i * i + i;
	}

       std::chrono::high_resolution_clock::time_point end2 = std::chrono::high_resolution_clock::now();
       std::chrono::duration<double> elapsed2 = std::chrono::duration_cast<std::chrono::duration<double>>(end2 - start2);

//       std::cout << "elapsed2 = " << elapsed2.count() << std::endl;

       if(debug) showMatrix(A, s, s, rank);
	
    }

    for(int j = 0; j < rep; j++){
    
	MPI_Bcast(A, length, MPI_DOUBLE, root, MPI_COMM_WORLD);

    	for(int i = 0; i < length; i++){
	    A[i] += j * i + i + j;
    	}
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    MPI_Barrier(MPI_COMM_WORLD);

//    if(rank == 0) 
	std::cout << "MPI ]> Proc nb = " << world_size <<", size = " << s << ", rep = " << rep <<"; elapsed time: " << elapsed.count() << std::endl;

    if(debug) showMatrix(A, s, s, rank);

    MPI_Finalize();
}
