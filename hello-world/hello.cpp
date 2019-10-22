// HPX includes
#include <hpx/hpx_init.hpp>
#include <hpx/hpx_main.hpp>

#include <mpi.h>

#include <iostream>

int main(int argc, char** argv) {

    int initialized;

    MPI_Initialized(&initialized);

    if(!initialized){
        MPI_Init(&argc, &argv);
    }

    int world_size;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::cout << "Rank (" << rank << ") : hello, I'm " << rank << " out of " << world_size << " procs !" << std::endl;

    if(!initialized){
        MPI_Finalize();
    }
}


