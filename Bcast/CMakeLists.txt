include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Bcast_mpi.exe Bcast_mpi.cpp)
hpx_setup_target(Bcast_mpi.exe COMPONENT_DEPENDENCIES iostreams)

add_executable(Bcast_hpx_mpi.exe Bcast_hpx_mpi.cpp)
hpx_setup_target(Bcast_hpx_mpi.exe COMPONENT_DEPENDENCIES iostreams)

add_executable(Bcast_hpx_mpi_fut.exe Bcast_hpx_mpi_fut.cpp)
hpx_setup_target(Bcast_hpx_mpi_fut.exe COMPONENT_DEPENDENCIES iostreams)

add_executable(Bcast_hpx_mpi_pool.exe Bcast_hpx_mpi_pool.cpp)
hpx_setup_target(Bcast_hpx_mpi_pool.exe COMPONENT_DEPENDENCIES iostreams)

add_executable(for_loop.exe for_loop.cpp)
hpx_setup_target(for_loop.exe COMPONENT_DEPENDENCIES iostreams)

add_executable(for_loop_hpx.exe for_loop_hpx.cpp)
hpx_setup_target(for_loop_hpx.exe COMPONENT_DEPENDENCIES iostreams)

