//
// Created by samuele on 07/03/19.
//
#include "common.h"
#include "utils.h"

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

void close_instance(Tsp_prob *instance) {
    free(instance->name);
    free(instance->comment);
    free(instance->filename);
    free(instance->coord_y);
    free(instance->coord_x);
    free(instance->solution);
}