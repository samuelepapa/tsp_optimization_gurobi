//
// Created by samuele on 19/05/19.
//

#include "common.h"
#include "mt64.h"
#include "matheuristic_utils.h"
#include "utils.h"
#include "tsp_grasp.h"

int x_pos_grasp(int i, int j, Tsp_prob *instance);

double grasp_cost_solution(Tsp_prob *instance, double *solution) {
    double cost = 0;
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            int coord = x_pos_grasp(i, j, instance);
            if (solution[coord] > TOLERANCE) {
                cost += distance(i, j, instance);
            }
        }
    }
    return cost;
}

void GRASP(Tsp_prob *instance) {
    //the time limit is interpreted as an iteration count in GRASP
    int iteration_count = (int) (instance->time_limit);
    int cur_iteration = 0;
    int num_var = 0.5 * (instance->nnode * (instance->nnode - 1));
    double *solution = calloc(num_var, sizeof(double));
    while (cur_iteration < iteration_count) {

        printf("GENSOL %d, %g\n", cur_iteration, grasp_cost_solution(instance, solution));
        cur_iteration++;
    }
}

void grasp_randomized_construction(Tsp_prob *instance, double *solution) {
    /*struct timespec start, cur;
    double time_elapsed = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    unsigned long long int seed = (unsigned long long int)start.tv_nsec;
    init_genrand64(seed);


    double breathing = 0.5;

    while (time_elapsed < time_limit) {

        clock_gettime(CLOCK_MONOTONIC, &cur);
        time_elapsed = (cur.tv_sec - start.tv_sec);
        printf("GENSOL %g\n", cost_solution(instance, solution, var_pos));
    }*/
}

void grasp_local_search(Tsp_prob *instance, double *solution, double time_limit, int method,
                           int (*var_pos)(int, int, Tsp_prob *)) {

}

void grasp_get_solution_pool(Tsp_prob *instance, double **solution_pool, double time_limit, int method) {

}

int x_pos_grasp(int i, int j, Tsp_prob *instance) {
    if (i == j) {
        return -1;
    }
    if (i > j) {
        return x_pos_grasp(j, i, instance);
    }
    return i * instance->nnode + j - ((i + 1) * (i + 2)) / 2;
}