//
// Created by samuele on 07/03/19.
//
#include "common.h"
#include "utils.h"

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