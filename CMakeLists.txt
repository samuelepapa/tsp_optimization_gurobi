cmake_minimum_required(VERSION 3.13)
project(tsp_optimization_gurobi)

set(CMAKE_C_STANDARD  11)

include_directories(/opt/gurobi810/linux64/include)
link_directories(/opt/gurobi810/linux64/lib)

add_library(gnuplot SHARED plotGraph.c)
add_library(gurobi81 STATIC IMPORTED)

add_executable(tsp_optimize main.c common.h)

target_link_libraries(tsp_optimize gurobi81)
target_link_libraries(tsp_optimize gnuplot)