cmake_minimum_required(VERSION 3.20)
project(chess-engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)

add_executable(chess-engine main.cpp chessboard.cpp perft.cpp)

add_custom_command(TARGET chess-engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:chess-engine>/assets)

target_link_libraries(chess-engine PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)