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
 * Print the error message associated by error integer value and free the gurobi model and the gurobi environment
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param error Integer error value returned by the gurobi methods
 */
void quit_on_GRB_error(GRBenv *env, GRBmodel *model, int error) {

    if(error) {
        /*error reporting*/
        printf("ERROR: %s\n", GRBgeterrormsg(env));

        /*free model*/
        GRBfreemodel(model);

        /*free environment*/
        GRBfreeenv(env);
        exit(1);
    }

}

/**
 * Round the distance to the nearest integer value
 * @param x Value of the distance
 * @return Nearest integer value of x
 */

int nint(double x) {
    return (int) (x+0.5);
}

/**
 * Select the maximum value between two number
 * @param i First number
 * @param j Second number
 * @return Maximum value between i and j
 */
int max(int i, int j) {
    if(i >= j) {
        return i;
    }

    return j;
}

/**
 * Convert coordinate value to geographical latitude or longitude
 * @param coord Coordinate of the node
 * @return Latitude or longitude value in radians
 */
double lat_long(double coord) {

    const double PI = 3.141592;

    double deg = (int) coord;
    double min = coord - deg;
    return PI * (deg + 5.0 * min / 3.0) / 180.0;

}

/**
 * Compute distance between two different nodes
 * @param i_latitude Latitude of the first node
 * @param j_latitude Latitude of the second node
 * @param i_longitude Longitude of the first node
 * @param j_longitude Longitude of the second node
 * @return Distance between two different nodes in kilometers
 */
int dist_from_geo(double i_latitude, double j_latitude, double i_longitude, double j_longitude) {

    const double RRR = 6378.388;

    double q1 = cos(i_longitude - j_longitude);
    double q2 = cos(i_latitude - j_latitude);
    double q3 = cos(i_latitude + j_latitude);
    return (int) (RRR * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
}

/**
 * Compute the distance between two points in two dimensions with the method described in the weight_type value of instance
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The distance value from i to j
 */
int distance(int i, int j, Tsp_prob *instance) {

    double xd = instance->coord_x[i] - instance->coord_x[j]; //x coordinates difference
    double yd = instance->coord_y[i] - instance->coord_y[j]; //y coordinates difference

    switch (instance->weight_type) {
        case 0:{ //Euclidean distances in 2-D
            return nint(sqrt(xd*xd + yd*yd));
        }

        case 1: { //Maximum distances in 2-D
            double x = abs(xd);
            double y = abs(yd);
            return max(nint(x), nint(y));
        }

        case 2: { //Manhattan distances in 2-D
            double x = abs(xd);
            double y = abs(yd);
            return nint(x + y);
        }

        case 3: { //Euclidean distances in 2-D rounded up
            return (int) ceil(sqrt(xd*xd + yd*yd));
        }

        case 4: { //Geographical distances

            double i_latitude = lat_long(instance->coord_x[i]);
            double j_latitude = lat_long(instance->coord_x[j]);

            double i_longitude = lat_long(instance->coord_y[i]);
            double j_longitude = lat_long(instance->coord_y[j]);

            return dist_from_geo(i_latitude, j_latitude, i_longitude, j_longitude);
        }

        case 5: { //Special distance function for problems att48 and att532 (pseudo-Euclidean)
            double rij = sqrt((xd*xd + yd*yd) / 10.0);
            int tij = nint(rij);
            if(tij < rij) {
                return tij + 1;
            } else {
                return tij;
            }
        }

        default: {
            printf("%s\n", "Wrong weight type!");
            exit(1);
        }
    }
}

int map_model_type (char *optarg) {

    printf("options: %s", optarg);
    if(strncmp(optarg, "std", 3) == 0) {
        return 0;
    }

    if(strncmp(optarg, "mtz", 3) == 0) {
        return 1;
    }

    if(strncmp(optarg, "fischetti", 9) == 0) {
        return 2;
    }

    if(strncmp(optarg, "flow1", 5) == 0) {
        return 3;
    }

    if(strncmp(optarg, "ts3", 3) == 0) {
        return 4;
    }
}

void inverse_map_model_type (int model_type, char *target_string) {

    switch(model_type){
        case 0:
            strcpy(target_string, "std");
            break;
        case 1:
            strcpy(target_string, "mtz");
            break;
        case 2:
            strcpy(target_string, "fischetti");
            break;
        case 3:
            strcpy(target_string, "flow1");
            break;
        case 4:
            strcpy(target_string, "ts3");
            break;
        default:
            strcpy(target_string, "not a model");
    }

}
/**
 * Free memory to avoid leaks, assumes instance is initialized as variable, not dinamically allocated
 * @param instance The pointer to the problem instance
 */
void close_instance(Tsp_prob *instance) {
    free(instance->name);
    free(instance->comment);
    free(instance->filename);
    free(instance->coord_y);
    free(instance->coord_x);
    //free(instance->weight_matrix);
    //free(instance->solution);
}