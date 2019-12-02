# - Find Elemental 
# Find the Elemental libraries
# Elemental: Distributed-memory, arbitrary-precision, dense and sparse-direct 
# linear algebra, conic optimization, and lattice reduction 
# Github: https://github.com/elemental/Elemental 
#   Elemental_FOUND            : True if BLIS_INCUDE_DIR are found
#   Elemental_INCLUDE_DIR      : where to find blis.h, etc.
#   Elemental_INCLUDE_DIRS     : set when BLIS_INCLUDE_DIR found
#   Elemental_LIBRARIES        : the library to link against.
#   PMRRR_LIBRARY	       : PMRRR_LIBRARY to link

set(Elemental_ROOT "/usr/local" CACHE PATH "Folder containing Elemental libraries")

if (NOT Elemental_ROOT AND DEFINED ENV{Elemental_ROOT})
  set(Elemental_ROOT $ENV{BLIS_ROOT} CACHE PATH "Folder containing Elemental")
elseif (NOT Elemental_ROOT AND DEFINED ENV{ElementalROOT})
  set(Elemental_ROOT $ENV{BISLROOT} CACHE PATH "Folder containing Elemental")
endif()

find_file(Elemental_basic_header 
    NAMES
        elemental.hpp
    PATHS
	${Elemental_ROOT}/include
)

if(Elemental_basic_header)

  find_path(Elemental_INCLUDE_DIR
    NAMES
        elemental.hpp
    PATHS
        ${Elemental_ROOT}/include
  )

  find_library(Elemental_LIBRARY libelemental.a
      PATHS ${Elemental_ROOT}/lib
  )

  find_library(PMRRR_LIBRARY libpmrrr.a
     PATHS ${Elemental_ROOT}/lib
  )
  
  set(LOW_VERISON true)

else()

  find_path(Elemental_INCLUDE_DIR
    NAMES
	El.hpp
    PATHS
        ${Elemental_ROOT}/include
  )

  find_library(Elemental_LIBRARY libEl.so 
      PATHS ${Elemental_ROOT}/lib
  )

  find_library(PMRRR_LIBRARY libpmrrr.so
     PATHS ${Elemental_ROOT}/lib
  )

  set(LOW_VERISON false)

endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Elemental
	DEFAULT_MSG
	Elemental_LIBRARY
	Elemental_INCLUDE_DIR
)

if(Elemental_FOUND)
    set(Elemental_INCLUDE_DIRS ${Elemental_INCLUDE_DIR})
    set(Elemental_LIBRARIES ${Elemental_LIBRARY}) 
    set(Elemental_IF_LOW_VERSION ${LOW_VERISON})
endif()



