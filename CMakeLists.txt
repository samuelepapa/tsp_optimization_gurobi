cmake_minimum_required(VERSION 3.13)
project(tsp_optimization_gurobi)

set(CMAKE_C_STANDARD 11)

#set new path cmake output files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)

#Bring the headers (.h) into the project
include_directories(include)
#Bring the source files (.c) into the project
include_directories(src)
#Bring the gurobi header into the project
include_directories(/opt/gurobi810/linux64/include)
#link_directories(/opt/gurobi810/linux64/lib)

find_library(C_GUROBI_LIB NAMES libgurobi81.so PATHS /opt/gurobi810/linux64/lib)

add_library(gnuplot STATIC src/plot_graph.c)
add_library(utils STATIC src/utils.c)
add_library(tspStd STATIC src/tsp_std.c)
add_library(inputOutput STATIC src/input_output.c)
add_library(tspMTZ STATIC src/tsp_mtz.c)
add_library(tspFischetti STATIC src/tsp_fischetti.c)
add_library(tspFlow1 STATIC src/tsp_flow1.c)
add_library(tspFlow2 STATIC src/tsp_flow2.c)
add_library(tspFlow3 STATIC src/tsp_flow3.c)
add_library(tspTS3 STATIC src/tsp_timed_stage3.c)

add_executable(tsp_optimize src/main.c include/common.h)

# for future use: m is the math.h library, it is not automatically linked by cmake
target_link_libraries(tsp_optimize utils tspStd inputOutput gnuplot tspMTZ tspFischetti tspFlow1 tspFlow2 tspFlow3 tspTS3 m)
# for future reference, link gurobi separately from other libraries, no idea why
target_link_libraries(tsp_optimize ${C_GUROBI_LIB})
