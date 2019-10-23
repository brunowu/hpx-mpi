#include <hpx/hpx_init.hpp>
#include <hpx/hpx.hpp>
#include <hpx/util/high_resolution_timer.hpp>
#include <hpx/include/parallel_transform.hpp>

#include <hpx/runtime/threads/cpu_mask.hpp>
#include <hpx/include/parallel_executors.hpp>

#include <hpx/runtime/threads/executors/pool_executor.hpp>

#include <hpx/runtime/threads/threadmanager.hpp>
#include <hpx/runtime/threads/policies/scheduler_mode.hpp>
#include <hpx/runtime/threads/detail/scheduled_thread_pool_impl.hpp>
#include <hpx/runtime/threads/policies/shared_priority_queue_scheduler.hpp>


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

    hpx::threads::scheduled_executor matrix_HP_executor =
          hpx::threads::executors::pool_executor("default", hpx::threads::thread_priority_high);

    hpx::threads::scheduled_executor matrix_normal_executor =
          hpx::threads::executors::pool_executor("default", hpx::threads::thread_priority_default);

    hpx::threads::scheduled_executor mpi_executor;

    hpx::threads::executors::pool_executor mpi_exec("mpi");
    
    mpi_executor = mpi_exec;

    hpx::util::high_resolution_timer t;

    int length = (int) s * s;

    double *A = new double [length];

    hpx::shared_future<double *> A_fut = hpx::make_ready_future(A);

    int rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int root = 0;

    if(rank == root){

	A_fut = hpx::dataflow(
	    matrix_HP_executor,
            hpx::util::unwrapping([length](double *A){

            hpx::util::high_resolution_timer t2;
	      
	        for(int i = 0; i < length; i++){
		    A[i] = i * i + i;
        	}
           double elapsed2 = t2.elapsed();
//           std::cout << "elapsed2 = " << elapsed2 << std::endl;

	    	return A;
	    }),
	    A_fut
    	);     
    }

    for(int j = 0; j < rep; j++){

        A_fut = hpx::dataflow(
            mpi_executor,
            hpx::util::unwrapping([length, root](double *A){
		
		MPI_Bcast(A, length, MPI_DOUBLE, root, MPI_COMM_WORLD);

                return A;
            }),
            A_fut
        );

	if(rank == root){
            A_fut = hpx::dataflow(
	        matrix_HP_executor,
                hpx::util::unwrapping([length, j](double *A){

                    for(int i = 0; i < length; i++){
			A[i] += j * i + i + j;
                    }

                    return A;
                }),
                A_fut
            );
	} else {
	    A_fut = hpx::dataflow(
                matrix_normal_executor,
                hpx::util::unwrapping([length, j](double *A){

                    for(int i = 0; i < length; i++){
		        A[i] += j * i + i + j;
                    }

                    return A;
                }),
                A_fut
            );
	}
    }

    A = A_fut.get();

    double elapsed = t.elapsed();

//    if(rank == 0) 
	std::cout << "HPX v3]> Proc nb = " << world_size <<", size = " << s << ", rep = " << rep <<"; elapsed time: " << elapsed << std::endl;

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


    hpx::resource::partitioner rp(desc_cmdline, argc, argv);

    rp.set_default_pool_name("default");

    std::string mpi_pool_name = "mpi";

    using high_priority_sched = hpx::threads::policies::shared_priority_queue_scheduler<>;
    
    using namespace hpx::threads::policies;
      
    rp.create_thread_pool(
        mpi_pool_name,
        [](hpx::threads::policies::callback_notifier& notifier, std::size_t num_threads,
           std::size_t thread_offset, std::size_t pool_index,
           std::string const& pool_name) -> std::unique_ptr<hpx::threads::thread_pool_base> {
//           std::cout << "User defined scheduler creation callback " << std::endl;
           std::unique_ptr<high_priority_sched> scheduler(new high_priority_sched(
              num_threads, {6, 6, 64}, "shared-priority-scheduler"));

          scheduler_mode mode =
              scheduler_mode(scheduler_mode::do_background_work | scheduler_mode::delay_exit);

          std::unique_ptr<hpx::threads::thread_pool_base> pool(
              new hpx::threads::detail::scheduled_thread_pool<high_priority_sched>(
                  std::move(scheduler), notifier, pool_index, pool_name, mode, thread_offset));
          return pool;
    });

    int thread_count = 0;

    int mpi_threads = 1;

    for (const hpx::resource::numa_domain& d : rp.numa_domains()){
	for (const hpx::resource::core& c : d.cores()){
	    for (const hpx::resource::pu& p : c.pus()){
		if(thread_count < mpi_threads){
//		    std::cout << "Added pu " << thread_count
//                              << " to " << mpi_pool_name << "\n";
		    rp.add_resource(p, mpi_pool_name);
                    thread_count++;
		}
	    }
	}
    }

    int initialized;

    MPI_Initialized(&initialized);

    if(!initialized){
        MPI_Init(&argc, &argv);
    }


    return hpx::init(desc_cmdline, argc, argv);

}
