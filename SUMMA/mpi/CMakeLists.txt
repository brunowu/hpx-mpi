add_executable(summa_mpi_bb.exe summa_mpi_bb.cpp)
target_link_libraries(summa_mpi_bb.exe PRIVATE gemm_common_with_mpi)


