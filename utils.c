//
// Created by samuele on 07/03/19.
//
#include "common.h"
#include <stdio.h>
#include "math.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include "utils.h"

/**
 * Used to free the solution array, if size is negative returns with no error. This is a "private" function.
 * @param instance the Tsp_prob instance where the solution is stored
 */
void free_solution_array(Tsp_prob * instance);


int xpos(int i, int j, Tsp_prob * instance){
    if(i==j) {
        printf("Index i=j\n");
        exit(1);
    }
    if(i>j){
        return xpos(j,i,instance);
    }
    return i*instance->nnode + j - ((i+1)*(i+2))/2;
}

int nint(double x) {
    return (int) (x+0.5);
}

int max(int i, int j) {
    if(i >= j) {
        return i;
    }

    return j;
}

double lat_long(double coord) {

    const double PI = 3.141592;

    int deg = nint(coord);
    double min = coord - deg;
    return PI * (deg + 5.0 * min / 3.0) /180.0;

}

int dist_from_geo(double i_latitude, double j_latitude, double i_longitude, double j_longitude) {

    const double RRR = 6378.388;

    double q1 = cos(i_longitude - j_longitude);
    double q2 = cos(i_latitude - j_latitude);
    double q3 = cos(i_latitude + j_latitude);
    return (int) (RRR * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
}

/**
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
 *
    * 7 = EUC_3D       : weights are Euclidean distances in 3-D
    * 8 = MAX_3D       : weights are maximum distances in 3-D
    * 9 = MAN_3D       : weights are Manhattan distances in 3-D
    * 10 = XRAY1       : special distance function for crystallography problems(version1)
    * 11 = XRAY2       : special distance function for crystallography problems (version2)
    * 12 = SPECIAL     : there is a special distance function documented elsewhere
    */
int distance(int i, int j, Tsp_prob *instance) {

    double xd = instance->coord_x[i] - instance->coord_x[j]; //x coordinates difference
    double yd = instance->coord_y[i] - instance->coord_y[j]; //y coordinates difference

    switch (instance->weight_type) {
        case 0:{
            return nint(sqrt(xd*xd + yd*yd));
        }

        case 1: {
            double x = abs(xd);
            double y = abs(yd);
            return max(nint(x), nint(y));
        }

        case 2: {
            double x = abs(xd);
            double y = abs(yd);
            return nint(x + y);
        }

        case 3: {
            return (int) ceil(sqrt(xd*xd + yd*yd));
        }

        case 4: {

            double i_latitude = lat_long(instance->coord_x[i]);
            double j_latitude = lat_long(instance->coord_x[j]);

            double i_longitude = lat_long(instance->coord_y[i]);
            double j_longitude = lat_long(instance->coord_y[j]);

            return dist_from_geo(i_latitude, j_latitude, i_longitude, j_longitude);
        }

        case 5: {
            double rij = sqrt((xd*xd + yd*yd) / 10.0);
            int tij = nint(rij);
            if(tij > rij) {
                return tij + 1;
            } else {
                return tij;
            }
        }

        case 6: {
            printf("%s\n", "Weight value is explicit.");
            break;
        }

        default: {
            printf("%s\n", "Wrong weight type!");
            exit(1);
        }
    }
}

void add_edge_to_solution(Tsp_prob * instance, int * edge){
    if(edge == NULL){
        printf("The edge is NULL, allocate it before passing it as argument. \n");
        exit(1);
    }
    if(instance->solution_size == 0){
        instance->solution = calloc(1, sizeof(int *));
        instance->solution_size = 1;
    }else if(instance->solution_size > 0) {
        instance->solution_size += 1;
        instance->solution = realloc(instance->solution, instance->solution_size * sizeof(int *));
    }else{
        printf("Error while adding edge to solution, the size is negative. \n");
        exit(1);
    }
    instance->solution[instance->solution_size - 1] = edge;
}

void free_solution_array(Tsp_prob * instance){
    if(instance->solution_size < 0){
        printf("Solution array was not initialized (sol size is negative).\n");
        return;
    }
    for(int i = 0; i< instance->solution_size; i++){
        free(instance->solution[i]);
    }
    free(instance->solution);
}

void close_instance(Tsp_prob *instance) {
    free(instance->name);
    free(instance->comment);
    free(instance->filename);
    free(instance->coord_y);
    free(instance->coord_x);
    free(instance->weight_matrix);
    free(instance->solution);
    free_solution_array(instance);
}