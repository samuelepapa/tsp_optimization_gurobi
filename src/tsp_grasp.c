//
// Created by samuele on 19/05/19.
//

#include "common.h"
#include "matheuristic_utils.h"
#include "utils.h"
#include "tsp_grasp.h"

double cost_solution(Tsp_prob *instance, double *solution, int (*var_pos)(int, int, Tsp_prob *)) {
    double cost = 0;
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            int coord = var_pos(i, j, instance);
            if (solution[coord] > TOLERANCE) {
                cost += distance(i, j, instance);
            }
        }
    }
    return cost;
}

void grasp_get_initial_sol(Tsp_prob *instance, double *solution, double time_limit, int method,
                           int (*var_pos)(int, int, Tsp_prob *)) {
    struct timespec start, cur;
    double time_elapsed = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);

    double breathing = 0.5;

    while (time_elapsed < time_limit) {

        clock_gettime(CLOCK_MONOTONIC, &cur);
        time_elapsed = (cur.tv_sec - start.tv_sec);
        printf("GENSOL %g\n", cost_solution(instance, solution, var_pos));
    }
}

void grasp_get_solution_pool(Tsp_prob *instance, double **solution_pool, double time_limit, int method) {

}