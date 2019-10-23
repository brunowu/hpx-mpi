#include <hpx/hpx_init.hpp>
#include <hpx/util/high_resolution_timer.hpp>
#include <hpx/include/parallel_transform.hpp>

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

int hpx_main(boost::program_options::variables_map& vm){

    int s = vm["size"].as<int>();
    int rep = vm["repetitions"].as<int>();
    std::string out = vm["debug"].as<std::string>();

    bool debug = false;

    if(out == "yes"){
        debug = true;
    }

    hpx::util::high_resolution_timer t;

    int length = (int) s * s;

    double *A = new double [length];

    int rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int root = 0;

    if(rank == root){

        hpx::util::high_resolution_timer t2;

        for(int i = 0; i < length; i++){
            A[i] = i * i + i;
        }

       double elapsed2 = t2.elapsed();
    //   std::cout << "elapsed2 = " << elapsed2 << std::endl;

       if(debug) showMatrix(A, s, s, rank);

    }

    for(int j = 0; j < rep; j++){

        MPI_Bcast(A, length, MPI_DOUBLE, root, MPI_COMM_WORLD);

        for(int i = 0; i < length; i++){
	    A[i] += j * i + i + j;
        }
    }

    double elapsed = t.elapsed();

//    if(rank == 0) 
	std::cout << "HPX ]> Proc nb = " << world_size <<", size = " << s << ", rep = " << rep <<"; elapsed time: " << elapsed << std::endl;

    if(debug) showMatrix(A, s, s, rank);

    return hpx::finalize();
}

int main(int argc, char** argv) {

    using namespace boost::program_options;

    options_description desc_cmdline;

    desc_cmdline.add_options()
    	("size,s", value<int>()->default_value(-1), "Matrix size (if > 0, overrides m, n, k).")
        ("debug", value<std::string>()->default_value("no"), "(debug) => print matrices")
    	("repetitions,r", value<int>()->default_value(5), "Number of repetitions.");


    return hpx::init(desc_cmdline, argc, argv);

}
