//
// Created by samuele on 07/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_PLOTGRAPH_H
#define TSP_OPTIMIZATION_GUROBI_PLOTGRAPH_H

#include <sys/types.h>
#include <sys/stat.h>
#include "common.h"
#include "utils.h"

/**
 * Plot the points of the instance passed in the argument
 * @param instance The pointer to the problem instance
 */
void plot_instance(Tsp_prob *instance);

/**
 * Plot the solution of the instance passed in the argument
 * @param instance The pointer to the problem instance
 */
void plot_edges(Solution_list *edges_list, Tsp_prob *instance);

#endif //TSP_OPTIMIZATION_GUROBI_PLOTGRAPH_H
