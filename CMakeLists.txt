cmake_minimum_required(VERSION 3.13)
project(tsp_optimization_gurobi)

set(CMAKE_C_STANDARD  11)

add_library(gnuplot SHARED plotGraph.c)
add_executable(tsp_optimize main.c common.h)



target_link_libraries(tsp_optimize gnuplot)