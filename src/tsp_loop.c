//
// Created by samuele on 26/03/19.
//

#include "common.h"
#include "tsp_loop.h"

#define TOLERANCE 10E-4

typedef struct{
    int *comps;
    int *number_of_comps;
    int *number_of_items;
}Connected_comp;

void find_connected_comps(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp);


void tsp_loop(Tsp_prob *instance){
    int nnode = instance -> nnode;
    int comp[nnode];
    int comp_count;
    find_connected_comps()
}

void find_connected_comps(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp){
    int nnode = instance -> nnode;
    int
    for(int i = 0; i < nnode; i++){
        comp->comps[i] = i;
    }

    for(int i = 0; i < nnode; i++){
        for(int j = 0; j < nnode; j++){
            if(getsolution(xpos_loop(i, j, instance))>1-TOLERANCE){

            }
        }
    }
}