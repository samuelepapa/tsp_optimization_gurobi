//
// Created by samuele on 04/03/19.
//

#ifndef UNTITLED_COMMON_H
#define UNTITLED_COMMON_H

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "gurobi_c.h"


typedef struct{
    char *name; //TODO free memory after allocation
    char *comment; //TODO free memory after allocation

    int type; //0: TSP | in future add multiple types
    int nnode; //number of nodes
    int weight_type; //type of weight between edges (maybe pointer to some function?)
    /**
     * 0 = EXPLICIT     : weights are listed explicitly in the corresponding section
     * 1 = EUC_2D       : weights are Euclidean distances in 2-D
     * 2 = EUC_3D       : weights are Euclidean distances in 3-D
     * 3 = MAX_2D       : weights are maximum distances in 2-D
     * 4 = MAX_3D       : weights are maximum distances in 3-D
     * 5 = MAN_2D       : weights are Manhattan distances in 2-D
     * 6 = MAN_3D       : weights are Manhattan distances in 3-D
     * 7 = CEIL_2D      : weights are Euclidean distances in 2-D rounded up
     * 8 = GEO          : weights are geographical distances
     * 9 = ATT          : special distance function for problems att48 and att532 (pseudo-Euclidean)
     * 10 = XRAY1       : special distance function for crystallography problems(version1)
     * 11 = XRAY2       : special distance function for crystallography problems (version2)
     * 12 = SPECIAL     : there is a special distance function documented elsewhere
     */

    double *coord_x; //list of x coordinates //TODO free memory after allocation
    double *coord_y; //list of y coordinates //TODO free memory after allocation

    double time_limit;

    char *filename; //TODO free memory after allocation

    double *solution;

} Tsp_prob;



#endif //UNTITLED_COMMON_H
