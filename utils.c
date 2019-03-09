//
// Created by samuele on 07/03/19.
//
#include "common.h"
#include "utils.h"
#include <math.h>
#include <limits.h>

/**
 * Associative function between points of an edge and memory position
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos(int i, int j, Tsp_prob * instance);

/**
 * Compute the distance between two points in two dimensions with the method described in the weight_type value of instance
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The distance value from i to j
 */
int distance(int i, int j, Tsp_prob *instance);

/**
 * Free memory to avoid leaks, assumes instance is initialized as variable, not dinamically allocated
 * @param instance The pointer to the problem instance
 */
void close_instance(Tsp_prob *instance);


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
int distance(int i, int j, Tsp_prob *instance) {

    double xd = instance->coord_x[i-1] - instance->coord_x[j-1]; //x coordinates difference
    double yd = instance->coord_y[i-1] - instance->coord_y[j-1]; //y coordinates difference

    switch (instance->weight_type) {
        case 1: {
            return nint(sqrt(xd*xd + yd*yd));
        }

        case 3: {
            double x = abs(xd);
            double y = abs(yd);
            return max(nint(x), nint(y));
        }

        case 5: {
            double x = abs(xd);
            double y = abs(yd);
            return nint(x + y);
        }

        case 7: {
            return (int) ceil(sqrt(xd*xd + yd*yd));
        }

        case 8: {

            double i_latitude = lat_long(instance->coord_x[i-1]);
            double j_latitude = lat_long(instance->coord_x[j-1]);

            double i_longitude = lat_long(instance->coord_y[i-1]);
            double j_longitude = lat_long(instance->coord_y[j-1]);

            return dist_from_geo(i_latitude, j_latitude, i_longitude, j_longitude);
        }

        case 9: {
            double rij = sqrt((xd*xd + yd*yd) / 10.0);
            int tij = nint(rij);
            if(tij > rij) {
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
    free(instance->solution);
}