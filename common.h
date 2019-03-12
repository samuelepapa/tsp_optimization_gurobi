//
// Created by samuele on 04/03/19.
//

#ifndef UNTITLED_COMMON_H
#define UNTITLED_COMMON_H

#include "gurobi_c.h"


typedef struct{
    char *name; //TODO free memory after allocation
    char *comment; //TODO free memory after allocation

    int type; //0: TSP | in future add multiple types
    int nnode; //number of nodes
    int weight_type; //type of weight between edges (maybe pointer to some function?)
    /**FIRST IDEA
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


    /**SECOND IDEA
     * for symmetric travelling salesman problems:
     *  0 = EUC_2D       : weights are Euclidean distances in 2-D
     *  1 = MAX_2D       : weights are maximum distances in 2-D
     *  2 = MAN_2D       : weights are Manhattan distances in 2-D
     *  3 = CEIL_2D      : weights are Euclidean distances in 2-D rounded up
     *  4 = GEO          : weights are geographical distances
     *  5 = ATT          : special distance function for problems att48 and att532 (pseudo-Euclidean)
     *  6 = EXPLICIT     : weights are listed explicitly in the corresponding section
     *
     *  other weight value:
     *  7 = EUC_3D       : weights are Euclidean distances in 3-D
     *  8 = MAX_3D       : weights are maximum distances in 3-D
     *  9 = MAN_3D       : weights are Manhattan distances in 3-D
     *  10 = XRAY1       : special distance function for crystallography problems(version1)
     *  11 = XRAY2       : special distance function for crystallography problems (version2)
     *  12 = SPECIAL     : there is a special distance function documented elsewhere
     */

    int edge_weight_format; //weight format for explicit weight type

    /** list of edge weight format
     * 0 = FUNCTION Weights are given by a function (see above)
     * 1 = FULL MATRIX Weights are given by a full matrix <-
     * 2 = UPPER ROW Upper triangular matrix (row-wise without diagonal entries) <-
     * 3 = LOWER ROW Lower triangular matrix (row-wise without diagonal entries)
     * 4 = UPPER DIAG ROW Upper triangular matrix (row-wise including diagonal entries) <-
     * 5 = LOWER DIAG ROW Lower triangular matrix (row-wise including diagonal entries) <-
     * 6 = UPPER COL Upper triangular matrix (column-wise without diagonal entries)
     * 7 = LOWER COL Lower triangular matrix (column-wise without diagonal entries)
     * 8 = UPPER DIAG COL Upper triangular matrix (column-wise including diagonal entries)
     * 9 = LOWER DIAG COL Lower triangular matrix (column-wise including diagonal entries)
     */

    double *coord_x; //list of x coordinates //TODO free memory after allocation
    double *coord_y; //list of y coordinates //TODO free memory after allocation

    // **weight_matrix; //weight value matrix

    double time_limit;

    char *filename; //TODO free memory after allocation

    int **solution;
    int solution_size;

} Tsp_prob;



#endif //UNTITLED_COMMON_H
