cmake_minimum_required(VERSION 3.16)
project(Trabalho1 CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )

add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(shared)