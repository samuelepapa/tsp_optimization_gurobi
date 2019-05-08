//
// Created by samuele on 06/05/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_TSP_USERCALL_H
#define TSP_OPTIMIZATION_GUROBI_TSP_USERCALL_H

#include "common.h"
#include "utils.h"
#include "input_output.h"
#include "union_find.h"

void tsp_usercall_model_generate(Tsp_prob *instance);

void tsp_usercall_model_run(Tsp_prob *instance);

void tsp_usercall_model_create(Tsp_prob *instance);


#endif //TSP_OPTIMIZATION_GUROBI_TSP_USERCALL_H
