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




void close_instance(Tsp_prob *instance) {
    free(instance->name);
    free(instance->comment);
    free(instance->filename);
    free(instance->coord_y);
    free(instance->coord_x);
    //free(instance->weight_matrix);
    //free(instance->solution);

}