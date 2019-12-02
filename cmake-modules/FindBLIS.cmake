# - Find BLIS
# Find the BLIS libraries
# BLIS: BLAS-like Library Instantiation Software Framework
# Github: https://github.com/flame/blis
#   BLIS_FOUND            : True if BLIS_INCUDE_DIR are found
#   BLIS_INCLUDE_DIR      : where to find blis.h, etc.
#   BLIS_INCLUDE_DIRS     : set when BLIS_INCLUDE_DIR found
#   BLIS_LIBRARIES        : the library to link against.

set(BLIS_ROOT "/usr/local" CACHE PATH "Folder containing BLIS libraries")

if (NOT BLIS_ROOT AND DEFINED ENV{BLIS_ROOT})
  set(BLIS_ROOT $ENV{BLIS_ROOT} CACHE PATH "Folder containing BLIS")
elseif (NOT BLIS_ROOT AND DEFINED ENV{BLISROOT})
  set(BLIS_ROOT $ENV{BISLROOT} CACHE PATH "Folder containing BLIS")
endif()

find_path(BLIS_INCLUDE_DIR
    NAMES
	blis.h
    PATHS
	${BLIS_ROOT}/include/blis
)

set(_BLIS_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

if(WIN32)
    if(BLIS_STATIC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .lib)
    else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES _dll.lib)
    endif()
else()
    if(BLIS_STATIC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
    else()
        if(APPLE)
            set(CMAKE_FIND_LIBRARY_SUFFIXES .dylib)
        else()
            set(CMAKE_FIND_LIBRARY_SUFFIXES .so)
        endif()
    endif()
endif()

find_library(BLIS_LIBRARY blis
   PATHS ${BLIS_ROOT}/lib
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(BLIS
	DEFAULT_MSG
	BLIS_LIBRARY
	BLIS_INCLUDE_DIR
)

if(BLIS_FOUND)
    set(BLIS_INCLUDE_DIRS ${BLIS_INCLUDE_DIR})
    set(BLIS_LIBRARIES ${BLIS_LIBRARY}) 
endif()




