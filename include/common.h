//
// Created by samuele on 04/03/19.
//

#ifndef UNTITLED_COMMON_H
#define UNTITLED_COMMON_H

#define DEBUG

#include "gurobi_c.h"

#ifdef DEBUG
# define DEBUG_PRINT(x) printf x
#else
# define DEBUG_PRINT(x) do {} while (0)
#endif

typedef struct{
    int verbosity;
    double time_limit;

    char *name; //TODO free memory after allocation
    char *comment; //TODO free memory after allocation

    int type; //0: TSP | in future add multiple types
    int nnode; //number of nodes
    int weight_type; //type of weight between edges (maybe pointer to some function?)
    /**for symmetric travelling salesman problems:
     *  0 = EUC_2D       : weights are Euclidean distances in 2-D
     *  1 = MAX_2D       : weights are maximum distances in 2-D
     *  2 = MAN_2D       : weights are Manhattan distances in 2-D
     *  3 = CEIL_2D      : weights are Euclidean distances in 2-D rounded up
     *  4 = GEO          : weights are geographical distances
     *  5 = ATT          : special distance function for problems att48 and att532 (pseudo-Euclidean)
     */

    int model_type;

    double *coord_x; //list of x coordinates //TODO free memory after allocation
    double *coord_y; //list of y coordinates //TODO free memory after allocation

    // **weight_matrix; //weight value matrix

    char *filename; //TODO free memory after allocation

    int **solution; //TODO free memory after allocation
    int solution_size;

    GRBenv *env;
    GRBmodel *model;

} Tsp_prob;

typedef struct{
  int **solution;
  int size;
}Solution_list;

#endif //UNTITLED_COMMON_H
