#include <hpx/hpx_init.hpp>
#include <hpx/util/high_resolution_timer.hpp>
#include <hpx/include/compute.hpp>
#include <hpx/include/parallel_transform.hpp>
#include <hpx/lcos/split_future.hpp>

#include <hpx/runtime/resource/partitioner.hpp>
#include <hpx/runtime/threads/cpu_mask.hpp>
#include <hpx/include/parallel_executors.hpp>
#include <hpx/runtime/threads/executors/pool_executor.hpp>
#include <hpx/runtime/threads/policies/shared_priority_queue_scheduler.hpp>

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

bool use_pools = true;
bool use_scheduler = true;
int mpi_threads = 1;

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

void SUMMA(int m, int n, int k, int mb, int nb, int kb, const double alpha, std::vector<std::vector<double>> A, int lda, std::vector<std::vector<double>> B, int ldb, const double beta, std::vector<std::vector<double>> *C, int ldc, MPI_Comm comm){

    hpx::threads::scheduled_executor matrix_HP_executor =
        hpx::threads::executors::pool_executor("default", hpx::threads::thread_priority_high);

    hpx::threads::scheduled_executor matrix_normal_executor =
        hpx::threads::executors::pool_executor("default", hpx::threads::thread_priority_default);

    hpx::threads::scheduled_executor mpi_executor;

    int nprocs, rank;

    int rowCommSize, colCommSize, rowCommRank, colCommRank;

    int periodic[] = {0, 0};
    int reorder = 0;
    int free_coords[2];
    int dims[2];
    int coord[2];

    MPI_Comm cartComm, rowComm, colComm;

    MPI_Comm_size(comm, &nprocs);

    if (use_pools && nprocs > 1) {
        hpx::threads::executors::pool_executor mpi_exec("mpi");
        mpi_executor = mpi_exec;
    } else {
        mpi_executor = matrix_HP_executor;
    }

    int rproc = std::sqrt(nprocs);

    if(rproc * rproc - nprocs != 0){
        throw "Support only square 2D grid";
    }

    dims[0] = dims[1] = rproc;

    int s = m;

    MPI_Cart_create(comm, 2, dims, periodic, reorder, &cartComm);

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

    int bsize = mb;

    int sb = (int) (s / rproc);

    int bn = (int)(sb / bsize);


    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<double>> A_save = A; 
    std::vector<std::vector<double>> B_save = B;
    std::vector<std::vector<double>> C_save = *C;;

    std::vector<hpx::shared_future<std::vector<double>>> C_save_fut;

    for(int i = 0; i < C_save.size(); i++){
	C_save_fut.push_back(hpx::make_ready_future(C_save[i]));
    } 
   
    hpx::shared_future<MPI_Comm> comm_fut =  hpx::make_ready_future(comm);


    int bcast_root;
    
    int kk;

    for(kk = 0; kk < s; kk += bsize){
	bcast_root = (int)(kk / sb);

	for(int i = 0; i < bn * bn; i++){
	    std::vector<double> A_loc(bsize * bsize);
	    hpx::shared_future<std::vector<double>> A_loc_fut = hpx::make_ready_future(A_loc);

	    if(coord[1] == bcast_root){
                A_loc_fut = hpx::dataflow(matrix_HP_executor, hpx::util::unwrapping([A_save, i](std::vector<double> A_loc){
			    A_loc = A_save[i];   
                      	    return A_loc;
                        }), A_loc_fut);
            }else{
            	A_loc_fut = hpx::dataflow(matrix_HP_executor, hpx::util::unwrapping([](std::vector<double> A_loc){
			 	std::fill(A_loc.begin(), A_loc.end(), 0.0);
                         	return A_loc;
                        }), A_loc_fut);
            }

	    std::tie(A_loc_fut, comm_fut) = hpx::split_future(
			hpx::dataflow(mpi_executor, hpx::util::unwrapping([bsize, bcast_root, rowComm](std::vector<double> A_loc, MPI_Comm comm){
			        MPI_Bcast(A_loc.data(), bsize * bsize, MPI_DOUBLE, bcast_root, rowComm);
                         	return std::make_pair(A_loc, comm);
                        }), A_loc_fut, comm_fut));

	    std::vector<double> B_loc(bsize * bsize);
            hpx::shared_future<std::vector<double>> B_loc_fut = hpx::make_ready_future(B_loc);


	    if(coord[0] == bcast_root){
                B_loc_fut = hpx::dataflow(matrix_HP_executor, hpx::util::unwrapping([B_save, i](std::vector<double> B_loc){
                            B_loc = B_save[i];
                            return B_loc;
                        }), B_loc_fut);
            }else{
                B_loc_fut = hpx::dataflow(matrix_HP_executor, hpx::util::unwrapping([](std::vector<double> B_loc){
                                std::fill(B_loc.begin(), B_loc.end(), 0.0);
                                return B_loc;
                        }), B_loc_fut);
            }

            std::tie(B_loc_fut, comm_fut) = hpx::split_future(
                        hpx::dataflow(mpi_executor, hpx::util::unwrapping([bsize, bcast_root, colComm](std::vector<double> B_loc, MPI_Comm comm){
                                MPI_Bcast(B_loc.data(), bsize * bsize, MPI_DOUBLE, bcast_root, colComm);
                                return std::make_pair(B_loc, comm);
                        }), B_loc_fut, comm_fut));

	    C_save_fut[i] =  hpx::dataflow(matrix_HP_executor, hpx::util::unwrapping([bsize, alpha, beta](std::vector<double> A_loc, std::vector<double> B_loc, std::vector<double> C){
    		   	        dgemm("N", "N", &bsize, &bsize, &bsize, &alpha, A_loc.data(), &bsize, B_loc.data(), &bsize, &beta, C.data(), &bsize);
                                return C;
                        }), A_loc_fut, B_loc_fut, C_save_fut[i]);	

        }

        for(int i = 0; i < bn * bn; i++){
	    C->at(i) = C_save_fut[i].get();
        }

    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    if(rank == 0){
	std::cout << "| SUMMA HPX | " << nprocs << " | " << m * rproc << " | " << bsize << " | " << elapsed.count() << " | " << std::endl;
    }

}

int hpx_main(boost::program_options::variables_map& vm){

    int s = vm["size"].as<int>();
    int rep = vm["repetitions"].as<int>();
    int bsize = vm["bsize"].as<int>();

    std::string debug = vm["debug"].as<std::string>();

    int nprocs, rank;

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    const double alpha = 1.0;
    const double beta = 1.0;

    int rproc = std::sqrt(nprocs);

    if(rproc * rproc - nprocs != 0){
        throw "Support only square 2D grid";
    }

    int sb = (int) (s / rproc);

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

    SUMMA(s, s, s, bsize, bsize, bsize, alpha, A, bsize, B, bsize, beta, &C, bsize, MPI_COMM_WORLD); 

    if(debug == "yes"){
        showMatrix(C, bsize, bsize, bn, bn, rank, "C");
    }

    return hpx::finalize();

}

int main(int argc, char** argv) {

    using namespace boost::program_options;

    options_description desc_cmdline;

    desc_cmdline.add_options()
        ("size,s", value<int>()->default_value(4), "Matrix size (if > 0, overrides m, n, k).")
	("bsize,b", value<int>()->default_value(2), "Block size.")
        ("debug", value<std::string>()->default_value("no"), "debug mode")
        ("repetitions,r", value<int>()->default_value(1), "Number of repetitions.");


    use_pools = true;

    use_scheduler = true;

    mpi_threads = 2;

    hpx::resource::partitioner rp(desc_cmdline, argc, argv);

    int ntasks = 1;
    if (std::getenv("SLURM_STEP_NUM_TASKS")) {
        ntasks = atoi(std::getenv("SLURM_STEP_NUM_TASKS"));
    }

    if (use_scheduler) {
	using high_priority_sched = hpx::threads::policies::shared_priority_queue_scheduler<>;
    	using namespace hpx::threads::policies;
	rp.create_thread_pool(
        "default",
        [](hpx::threads::policies::callback_notifier& notifier, std::size_t num_threads,
           std::size_t thread_offset, std::size_t pool_index,
           std::string const& pool_name) -> std::unique_ptr<hpx::threads::thread_pool_base> {
           std::unique_ptr<high_priority_sched> scheduler(new high_priority_sched(
                num_threads, {6, 6, 64}, "shared-priority-scheduler"));

          scheduler_mode mode = scheduler_mode(scheduler_mode::do_background_work |
					       scheduler_mode::delay_exit);

          std::unique_ptr<hpx::threads::thread_pool_base> pool(
              new hpx::threads::detail::scheduled_thread_pool<high_priority_sched>(
                  std::move(scheduler), notifier, pool_index, pool_name, mode, thread_offset));
          return pool;
        });
    }
    else {

    }  

    if (use_pools && ntasks > 1) {
	if (use_scheduler) {
	    using high_priority_sched = hpx::threads::policies::shared_priority_queue_scheduler<>;
      	    using namespace hpx::threads::policies;
 	    rp.create_thread_pool(
            "mpi",
            [](hpx::threads::policies::callback_notifier& notifier, std::size_t num_threads,
               std::size_t thread_offset, std::size_t pool_index,
               std::string const& pool_name) -> std::unique_ptr<hpx::threads::thread_pool_base> {
               std::unique_ptr<high_priority_sched> scheduler(new high_priority_sched(
                   num_threads, {6, 6, 64}, "shared-priority-scheduler"));

               scheduler_mode mode =
                   scheduler_mode(scheduler_mode::do_background_work | scheduler_mode::delay_exit);

               std::unique_ptr<hpx::threads::thread_pool_base> pool(
                   new hpx::threads::detail::scheduled_thread_pool<high_priority_sched>(
                       std::move(scheduler), notifier, pool_index, pool_name, mode, thread_offset));
               return pool;
              }
            );
        }
        else {
  	    rp.create_thread_pool("mpi", hpx::resource::scheduling_policy::local_priority_fifo);  
        }

        int count = 0;
        for (const hpx::resource::numa_domain& d : rp.numa_domains()) {
          for (const hpx::resource::core& c : d.cores()) {
            for (const hpx::resource::pu& p : c.pus()) {
              if (count < mpi_threads) {
		count++;
//                std::cout << "Added pu " << count++ << " to mpi pool\n";
                rp.add_resource(p, "mpi");
              }
            }
          }
        }

//        std::cout << "[main] "
//              << "resources added to thread_pools \n";

    }

    int initialized;

    MPI_Initialized(&initialized);

    if(!initialized){
        MPI_Init(&argc, &argv);
    }

    return hpx::init(desc_cmdline, argc, argv);

}

