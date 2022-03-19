cmake_minimum_required(VERSION 3.18)
project(Server CXX)

add_executable(server src/main.cpp src/server/server.cpp src/clientSocket/clientSocket.cpp src/table/tableRow.cpp)
target_link_libraries(server shared pthread)
target_include_directories(server PRIVATE src/server src/clientSocket src/table)
target_include_directories(client PRIVATE ${shared_INCLUDE_DIRS})