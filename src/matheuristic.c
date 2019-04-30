#include "matheuristic.h"

/**
 * Mapping between points of an edge and position in GRB model
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos_matheuristics(int i, int j, Tsp_prob *instance);

void tsp_matheuristic_model_create(Tsp_prob *instance) {

}

int xpos_matheuristics(int i, int j, Tsp_prob *instance){
    if(i==j) {
        return -1;
    }
    if(i>j){
        return xpos_matheuristics(j, i, instance);
    }
    return i*instance->nnode + j - ((i+1)*(i+2))/2;
}
