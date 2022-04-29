cmake_minimum_required(VERSION 3.16)
project(Server CXX)

add_library(shared src/command/command.cpp src/connection/connection.cpp)
target_include_directories(shared PUBLIC src)