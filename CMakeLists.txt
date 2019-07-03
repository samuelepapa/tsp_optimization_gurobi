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
#Bring the concorde header into the project
include_directories(concorde/concorde_build)

find_library(C_GUROBI_LIB NAMES libgurobi81.so PATHS /opt/gurobi810/linux64/lib)

add_library(argtable STATIC src/argtable3.c)
add_library(mt19937 STATIC src/mt19937-64.c)
add_library(gnuplot STATIC src/plot_graph.c)
add_library(utils STATIC src/utils.c)
add_library(inputOutput STATIC src/input_output.c)
add_library(unionFind STATIC src/union_find.c)
add_library(tspStd STATIC src/tsp_std.c)
add_library(tspMTZ STATIC src/tsp_mtz.c)
add_library(tspBadCompact STATIC src/tsp_bad_compact.c)
add_library(tspFlow1 STATIC src/tsp_flow1.c)
add_library(tspFlow2 STATIC src/tsp_flow2.c)
add_library(tspFlow3 STATIC src/tsp_flow3.c)
add_library(tspTS1 STATIC src/tsp_timed_stage1.c)
add_library(tspTS2 STATIC src/tsp_timed_stage2.c)
add_library(tspTS3 STATIC src/tsp_timed_stage3.c)
add_library(tspLoop STATIC src/tsp_loop.c)
add_library(tspLazycall STATIC src/tsp_lazycall.c)
add_library(tspHardfixing STATIC src/tsp_hardfixing.c)
add_library(tspUsercall STATIC src/tsp_usercall.c)
add_library(matheuristicUtils STATIC src/matheuristic_utils.c)
add_library(tspLocalBranching STATIC src/tsp_local_branching.c)
add_library(metaheuristicUtils STATIC src/meta_heuristic_utils.c)
add_library(tspVNS STATIC src/tsp_vns.c)
add_library(tspGRASP STATIC src/tsp_grasp.c)
add_library(tspSA STATIC src/tsp_simulated_annealing.c)
add_library(tspMetaheuristics STATIC src/tsp_metaheuristics.c)

add_executable(tsp_optimize src/main.c include/common.h)

# for future use: m is the math.h library, it is not automatically linked by cmake
target_link_libraries(tsp_optimize tspMetaheuristics tspSA tspGRASP tspVNS metaheuristicUtils tspLocalBranching tspHardfixing tspUsercall
        tspLazycall matheuristicUtils tspLoop utils unionFind tspStd inputOutput gnuplot tspMTZ tspBadCompact tspFlow1
        tspFlow2 tspFlow3 tspTS1 tspTS2 tspTS3 mt19937 argtable m)

target_link_libraries(tsp_optimize ${PROJECT_SOURCE_DIR}/concorde/concorde_build/concorde.a)

# for future reference, link gurobi separately from other libraries, no idea why
target_link_libraries(tsp_optimize ${C_GUROBI_LIB})