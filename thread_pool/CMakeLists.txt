include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(named_pool_executor.exe named_pool_executor.cpp)

hpx_setup_target(named_pool_executor.exe COMPONENT_DEPENDENCIES iostreams)

INCLUDE(CPack)

