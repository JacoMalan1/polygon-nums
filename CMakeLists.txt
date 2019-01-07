cmake_minimum_required(VERSION 3.12)
project(ShapeNums)

set(CMAKE_CXX_STANDARD 17)

add_executable(shape_nums main.cpp)

target_link_libraries(shape_nums pthread)
target_link_libraries(shape_nums gmp)
target_link_libraries(shape_nums gmpxx)
