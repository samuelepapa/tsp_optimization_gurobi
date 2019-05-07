//
// Created by samuele on 08/04/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_TSP_LAZYCALL_H
#define TSP_OPTIMIZATION_GUROBI_TSP_LAZYCALL_H

#include "common.h"
#include "utils.h"
#include "input_output.h"
#include "union_find.h"
#include "tsp_matheuristic.h"

struct callback_data {
    int nvars;
    Tsp_prob *instance;

    Graph *graph;
    Connected_component *conn_comps;
    //Edge list in node-node format (for Concorde)
    int *edge_list;

    int lazyconst_id;
    int userconst_id;
};

int xpos_lazycall(int i, int j, Tsp_prob *instance);

void tsp_lazycall_model_generate(Tsp_prob *instance);

void tsp_lazycall_model_run(Tsp_prob *instance);

void tsp_lazycall_model_create(Tsp_prob *instance);

void
add_lazy_sec(void *cbdata, struct callback_data *user_cbdata, Connected_component *conn_comps, int n_comps, int node);

#endif //TSP_OPTIMIZATION_GUROBI_TSP_LAZYCALL_H
