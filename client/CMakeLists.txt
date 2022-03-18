cmake_minimum_required(VERSION 3.18)
project(Client CXX)

add_executable(client src/main.cpp src/ui/ui.cpp src/client/client.cpp)
target_include_directories(client PRIVATE src/ui src/client)

target_link_libraries(client PRIVATE ncurses shared)