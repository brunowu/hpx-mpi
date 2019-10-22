cmake_minimum_required(VERSION 3.0)

project(hpx-mpi CXX)

find_package(HPX REQUIRED)
find_Package(MPI REQUIRED)

if(HPX_FOUND AND MPI_FOUND)

  SET(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})

  link_directories(${HPX_LIBRARY_DIR})

  include_directories(${HPX_INCLUDE_DIRS})

  ADD_SUBDIRECTORY(hello-world)
  ADD_SUBDIRECTORY(thread_pool)

  include(Dart)
  include(CPack)

endif()
