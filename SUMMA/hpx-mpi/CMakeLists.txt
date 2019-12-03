add_executable(summa_hpx_mpi.exe summa_hpx_mpi.cpp)
target_link_libraries(summa_hpx_mpi.exe PRIVATE gemm_common_with_hpx)

add_executable(summa_hpx_mpi_bb.exe summa_hpx_mpi_bb.cpp)
target_link_libraries(summa_hpx_mpi_bb.exe PRIVATE gemm_common_with_hpx)

