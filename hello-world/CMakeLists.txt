include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hello.exe hello.cpp)

hpx_setup_target(hello.exe COMPONENT_DEPENDENCIES iostreams)

INCLUDE(CPack)

