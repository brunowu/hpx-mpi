# - Find Intel MKL
# Find the MKL libraries
#
# Options:
#
#   MKL_ROOT              : place to search for MKL
#   $ENV{MKL_ROOT}        :   "  "  "  "  "  "  "
#   $ENV{MKLROOT}         :   "  "  "  "  "  "  "
#   MKL_STATIC            : use static linking
#   MKL_MULTI_THREADED    : use multi-threading
#   MKL_SDL               : Single Dynamic Library interface
#   MKL_32                : search for ia32 instead of intel64
#   MKL_VERBOSE           : display info about found heders/libs/etc
#
# This module defines the following variables:
#
#   MKL_FOUND            : True if MKL_INCLUDE_DIR are found
#   MKL_INCLUDE_DIR      : where to find mkl.h, etc.
#   MKL_INCLUDE_DIRS     : set when MKL_INCLUDE_DIR found
#   MKL_LIB_DIR          : the dir where the mkl libs are found
#   MKL_LIBRARIES        : the library to link against.
#   MKL_LIBRARIES_MT     : the library to link against.

macro(MKL_MESSAGE string)
  if (MKL_VERBOSE)
    message("MKL: " ${string})
  endif()
endmacro()

include(FindPackageHandleStandardArgs)

set(INTEL_ROOT "/opt/intel" CACHE PATH "Folder containing intel libs")
if (NOT MKL_ROOT AND DEFINED ENV{MKL_ROOT})
  set(MKL_ROOT $ENV{MKL_ROOT} CACHE PATH "Folder containing MKL")
elseif (NOT MKL_ROOT AND DEFINED ENV{MKLROOT})
  set(MKL_ROOT $ENV{MKLROOT} CACHE PATH "Folder containing MKL")
elseif (NOT MKL_ROOT)
  set(MKL_ROOT ${INTEL_ROOT}/mkl CACHE PATH "Folder containing MKL")
endif()

MKL_MESSAGE("Using MKL_ROOT ${MKL_ROOT}")

# Find include dir
find_path(MKL_INCLUDE_DIR mkl.h
  PATHS ${MKL_ROOT}/include)

MKL_MESSAGE("MKL_INCLUDE_DIR is ${MKL_INCLUDE_DIR}")

# Find include directory
#  There is no include folder under linux
if(WIN32)
    find_path(INTEL_INCLUDE_DIR omp.h
        PATHS ${INTEL_ROOT}/include)
    set(MKL_INCLUDE_DIR ${MKL_INCLUDE_DIR} ${INTEL_INCLUDE_DIR})
endif()

# Find libraries

# Handle suffix
set(_MKL_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

if(WIN32)
    if(MKL_STATIC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES _dll.lib)
    endif()
else()
    if(MKL_STATIC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    else()
        if(APPLE)
            set(CMAKE_FIND_LIBRARY_SUFFIXES .dylib)
        else()
            set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
        endif()
    endif()
endif()

if(UNIX AND NOT APPLE)
    if(MKL_32)
        set(MKL_SUBDIR ia32)
    else()
        set(MKL_SUBDIR intel64)
        MKL_MESSAGE("MKL sub dir is ${MKL_SUBDIR}")
    endif()
endif()

set(MKL_LIB_DIR "${MKL_ROOT}/lib/${MKL_SUBDIR}" CACHE INTERNAL "lib path")
MKL_MESSAGE("MKL_LIB_DIR sub dir is ${MKL_LIB_DIR}")

# MKL is composed by four layers: Interface, Threading, Computational and RTL

if(MKL_SDL)
    find_library(MKL_LIBRARY mkl_rt
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)

    set(MKL_MINIMAL_LIBRARY ${MKL_LIBRARY})
else()
    ######################### Interface layer #######################
    if(WIN32)
        set(MKL_INTERFACE_LIBNAME mkl_intel_c)
    else()
        set(MKL_INTERFACE_LIBNAME mkl_intel_ilp64)
    endif()

    find_library(MKL_INTERFACE_LIBRARY ${MKL_INTERFACE_LIBNAME}
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)
    MKL_MESSAGE("MKL_INTERFACE_LIBRARY is ${MKL_INTERFACE_LIBRARY}")

    ######################## Threading layer ########################
    find_library(MKL_THREADING_LIBRARY mkl_sequential
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)
    find_library(MKL_THREADING_LIBRARY_MT mkl_gnu_thread
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)

    ####################### Computational layer #####################
    find_library(MKL_CORE_LIBRARY mkl_core
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)
    find_library(MKL_FFT_LIBRARY mkl_cdft_core
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)
    find_library(MKL_SCALAPACK_LIBRARY mkl_scalapack_ilp64
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)
    find_library(MKL_BLACS_LIBRARY mkl_blacs_intelmpi_ilp64
        PATHS ${MKL_ROOT}/lib/${MKL_SUBDIR}/)


    MKL_MESSAGE("MKL_CORE_LIBRARY is ${MKL_CORE_LIBRARY}")
    MKL_MESSAGE("MKL_FFT_LIBRARY is ${MKL_FFT_LIBRARY}")
    MKL_MESSAGE("MKL_SCALAPACK_LIBRARY is ${MKL_SCALAPACK_LIBRARY}")
    MKL_MESSAGE("MKL_BLACS_LIBRARY is ${MKL_BLACS_LIBRARY}")

    ############################ RTL layer ##########################
    if(WIN32)
        set(MKL_RTL_LIBNAME libiomp5md)
    else()
        set(MKL_RTL_LIBNAME libiomp5)
    endif()
    find_library(MKL_RTL_LIBRARY ${MKL_RTL_LIBNAME}
        PATHS ${INTEL_RTL_ROOT}/lib)
    MKL_MESSAGE("MKL_RTL_LIBRARY is ${MKL_RTL_LIBRARY}")

    set(MKL_LIBRARY
        ${MKL_INTERFACE_LIBRARY}
        ${MKL_CORE_LIBRARY}
        ${MKL_THREADING_LIBRARY}
        ${MKL_FFT_LIBRARY}
        ${MKL_SCALAPACK_LIBRARY}
    )
    set(MKL_MINIMAL_LIBRARY ${MKL_INTERFACE_LIBRARY} ${MKL_THREADING_LIBRARY} ${MKL_CORE_LIBRARY})
    if (MKL_RTL_LIBRARY)
      set(MKL_LIBRARY ${MKL_LIBRARY} ${MKL_RTL_LIBRARY})
      set(MKL_MINIMAL_LIBRARY ${MKL_MINIMAL_LIBRARY} ${MKL_RTL_LIBRARY})
    endif()
endif()

set(CMAKE_FIND_LIBRARY_SUFFIXES ${_MKL_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})

find_package_handle_standard_args(MKL DEFAULT_MSG
    MKL_INCLUDE_DIR MKL_LIBRARY MKL_MINIMAL_LIBRARY)

if(MKL_FOUND)
    set(MKL_INCLUDE_DIRS ${MKL_INCLUDE_DIR})
    set(MKL_LIBRARIES ${MKL_LIBRARY})
    set(MKL_MINIMAL_LIBRARIES ${MKL_LIBRARY})
    set(MKL_LIBRARIES_MT
        ${MKL_INTERFACE_LIBRARY}
        ${MKL_CORE_LIBRARY}
        ${MKL_THREADING_LIBRARY_MT}
        ${MKL_FFT_LIBRARY}
        ${MKL_SCALAPACK_LIBRARY}
        ${MKL_BLACS_LIBRARY}
    )
    MKL_MESSAGE("MKL_LIBRARIES_MT are ${MKL_LIBRARIES_MT}")
endif()


