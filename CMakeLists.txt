cmake_minimum_required(VERSION 3.0)

project(hpx-mpi CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(HPX REQUIRED)

list (APPEND CMAKE_MODULE_PATH "${hpx-mpi_SOURCE_DIR}/cmake-modules")

option(MKL_MT "MKL with multithreading support" OFF)

if(MKL_MT)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(OPENMP_FLAGS "-fopenmp")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(OPENMP_FLAGS "-fopenmp")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(OPENMP_FLAGS "-openmp")
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(OPENMP_FLAGS "-fopenmp")
  endif()
endif()

find_package(MKL REQUIRED)

if (MKL_FOUND)
    set(LAPACK_INCLUDE_DIRS ${MKL_INCLUDE_DIRS})
    if(MKL_MT)
      set(LAPACK_LIBRARIES
        mkl_intel_lp64 mkl_core mkl_gnu_thread
        gomp pthread m dl mkl_blacs_intelmpi_lp64
        mkl_scalapack_lp64
        CACHE PATH "LAPACK_LIBS"
      )
    else()   
      set(LAPACK_LIBRARIES
        mkl_intel_lp64 mkl_core mkl_sequential
        pthread m dl mkl_blacs_intelmpi_lp64
        mkl_scalapack_lp64
        CACHE PATH "LAPACK_LIBS"
      )
    endif()
    link_directories(${MKL_LIB_DIR})
endif()

find_package(Boost REQUIRED COMPONENTS program_options)

include(CheckCXXSymbolExists)
CHECK_CXX_SYMBOL_EXISTS(MPI_Finalize mpi.h COMPILER_HAS_MPI)

if(COMPILER_HAS_MPI)
  add_library(MPI::MPI_C INTERFACE IMPORTED)
else()
  find_package(MPI REQUIRED)
  if(MPI_FOUND)
    SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
  endif()
endif()

add_library(gemm_common INTERFACE)
target_include_directories(gemm_common
  INTERFACE
    ${LAPACK_INCLUDE_DIRS})

target_link_libraries(gemm_common
  INTERFACE
    ${LAPACK_LIBRARIES}
    Boost::program_options)


add_library(gemm_common_with_mpi INTERFACE)

if(COMPILER_HAS_MPI)
  target_link_libraries(gemm_common_with_mpi
    INTERFACE
    gemm_common
    MPI::MPI_C)
else()
  target_link_libraries(gemm_common_with_mpi
    INTERFACE
    gemm_common
    ${MPI_CXX_LIBRARIES})
endif()


add_library(gemm_common_with_hpx INTERFACE)

target_include_directories(gemm_common_with_hpx
  INTERFACE ${HPX_INCLUDE_DIRS})

target_link_libraries(gemm_common_with_hpx
  INTERFACE
    gemm_common_with_mpi
    ${HPX_LIBRARIES}
)

SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})

link_directories(${HPX_LIBRARY_DIR})

include_directories(${HPX_INCLUDE_DIRS})

ADD_SUBDIRECTORY(hello-world)
ADD_SUBDIRECTORY(SUMMA)

include(Dart)
include(CPack)

