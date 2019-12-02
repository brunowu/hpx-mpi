add_executable(summa_mpi.exe summa_mpi.cpp)
target_link_libraries(summa_mpi.exe PRIVATE gemm_common_with_mpi)


