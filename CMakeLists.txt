cmake_minimum_required(VERSION 3.20)
project(chess-engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)

add_executable(chess-engine main.cpp)

target_link_libraries(chess-engine PRIVATE SDL3::SDL3)