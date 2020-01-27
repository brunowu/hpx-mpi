add_executable(summa_c_mpi_bb.exe summa_c_mpi_bb.cpp)
target_link_libraries(summa_c_mpi_bb.exe PRIVATE gemm_common_with_mpi)

