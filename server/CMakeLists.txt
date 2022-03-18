cmake_minimum_required(VERSION 3.18)
project(Server CXX)

add_executable(server src/main.cpp src/server/server.cpp src/clientSocket/clientSocket.cpp)
target_link_libraries(server shared)
target_include_directories(server PRIVATE src/server src/clientSocket)
target_include_directories(client PRIVATE ${shared_INCLUDE_DIRS})