cmake_minimum_required(VERSION 3.13)
project(tsp_optimization_gurobi)

set(CMAKE_C_STANDARD 11)

include_directories(/opt/gurobi810/linux64/include)
#link_directories(/opt/gurobi810/linux64/lib)

find_library(C_GUROBI_LIB NAMES libgurobi81.so PATHS /opt/gurobi810/linux64/lib)

add_library(gnuplot STATIC plotGraph.c)
add_library(utils STATIC utils.c)
add_library(tsp STATIC tsp.c)
add_library(inputOutput STATIC inputOutput.c)
add_library(tspMTZ STATIC tspMTZ.c)

add_executable(tsp_optimize main.c)

# for future use: m is the math.h library, it is not automatically linked by cmake
target_link_libraries(tsp_optimize utils gnuplot tsp inputOutput tspMTZ m)
# for future reference, link gurobi separately from other libraries, no idea why
target_link_libraries(tsp_optimize ${C_GUROBI_LIB})
