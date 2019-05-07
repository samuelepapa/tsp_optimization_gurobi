#include "tsp_hardfixing.h"
#include "matheuristic_utils.h"

#define  DEFAULT_TIMELIMIT 120

#define max(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

int num_it = 0;

void remove_subtour(Tsp_prob *instance, double *solution, int num_conn_comp, Connected_component *conn_comps, int (*var_pos)(int, int, Tsp_prob *));

void set_warm_start(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

int set_hard_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *));

/**
 * Initializes the hardfixing algorithm by finding solution using warm start + fast run
 * @param instance the tsp prob instance
 * @return error code from Gurobi
 */
int initialize_hardfixing(Tsp_prob *instance, double time_limit);

void tsp_hardfixing_model_create(Tsp_prob *instance) {
    struct timespec start, end;
    double time_elapsed;
    const double initial_sol_timelimit = 10;

    clock_gettime(CLOCK_MONOTONIC, &start);

    double time_limit = instance->time_limit;
    if (time_limit == INFINITY) {
        time_limit = DEFAULT_TIMELIMIT;
    }
    instance->time_limit = max(time_limit / 25, 10);

    initialize_hardfixing(instance, initial_sol_timelimit);

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_elapsed = (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;

    printf("timelimit: %g\n", time_limit);
    //Execute at a minimum 25 times, fixed parameter, at least 10 seconds

    GRBsetdblparam(GRBgetenv(instance->model), GRB_DBL_PAR_TIMELIMIT, instance->time_limit);

    while (time_elapsed < time_limit) {
        switch (instance->black_box) {
            case 9:
                set_hard_constraints(instance, xpos_loop);

                tsp_loop_model_run(instance);
                break;
            case 10: {
                set_hard_constraints(instance, xpos_lazycall);

                tsp_lazycall_model_run(instance);
            }
                break;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_elapsed = (end.tv_sec - start.tv_sec) + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;
    }


}

int initialize_hardfixing(Tsp_prob *instance, double time_limit) {
    int error = 0;
    instance->best_heur_sol_value = INFINITY;
    instance->heuristic_iteration = -1;
    //first call to the selected model
    switch (instance->black_box) {
        case 9: {
            tsp_loop_model_generate(instance);

            set_warm_start(instance, xpos_loop);

            error = GRBsetdblparam(GRBgetenv(instance->model), GRB_DBL_PAR_TIMELIMIT, time_limit);
            quit_on_GRB_error(instance->env, instance->model, error);

            //Run very fast, if nothing is found keep initial solution set by set_warm_start using work_start method
            tsp_loop_model_run(instance);
        }
            break;
        case 10: {
            tsp_lazycall_model_generate(instance);

            set_warm_start(instance, xpos_lazycall);

            error = GRBsetdblparam(GRBgetenv(instance->model), GRB_DBL_PAR_TIMELIMIT, time_limit);
            quit_on_GRB_error(instance->env, instance->model, error);

            //Run very fast, if nothing is found keep initial solution set by set_warm_start using work_start method
            tsp_lazycall_model_run(instance);
        }
            break;
        case 12:
            tsp_usercall_model_create(instance);
            break;

    }
    error = GRBsetintparam(GRBgetenv(instance->model), GRB_INT_PAR_SOLUTIONLIMIT, GRB_MAXINT);
    quit_on_GRB_error(instance->env, instance->model, error);

    instance->heuristic_iteration = 0;
    error = GRBgetdblattr(instance->model, GRB_DBL_ATTR_OBJVAL, &instance->best_heur_sol_value);
    quit_on_GRB_error(instance->env, instance->model, error);

    return error;
}

void set_warm_start(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {
    int error;
    error = GRBsetintparam(GRBgetenv(instance->model), GRB_INT_PAR_SOLUTIONLIMIT, 2);
    quit_on_GRB_error(instance->env, instance->model, error);

    //find the warm solution to feed to algorithm
    int nvariables = (int) (0.5 * (instance->nnode * instance->nnode - instance->nnode));
    double *solution = calloc(nvariables, sizeof(double));
    get_initial_heuristic_sol(instance, solution, var_pos);

    error = GRBsetdblattrarray(instance->model, GRB_DBL_ATTR_START, 0, nvariables, solution);
    quit_on_GRB_error(instance->env, instance->model, error);

    error = GRBupdatemodel(instance->model);
    quit_on_GRB_error(instance->env, instance->model, error);
}

int set_hard_constraints(Tsp_prob *instance, int (*var_pos)(int, int, Tsp_prob *)) {
    int error;
    double cur_solution;
    error = GRBgetdblattr(instance->model, GRB_DBL_ATTR_OBJVAL, &cur_solution);
    quit_on_GRB_error(instance->env, instance->model, error);
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            error = GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 0.0);
            quit_on_GRB_error(instance->env, instance->model, error);

        }
    }

    double percent_decr = (-(cur_solution - instance->best_heur_sol_value) / instance->best_heur_sol_value) * 100;

    if (percent_decr < 5) {
        if (instance->heuristic_iteration < 10) {
            instance->heuristic_iteration++;
        } else {
            instance->heuristic_iteration = 0;
            if (instance->prob >= 0.1) {
                instance->prob = instance->prob * 0.8;
            }
        }
    }
    if (cur_solution < instance->best_heur_sol_value) {
        instance->best_heur_sol_value = cur_solution;
    }

    double x;

    for (int i = 0; i < instance->nnode; i++) {
        for (int j = i + 1; j < instance->nnode; j++) {
            error = GRBgetdblattrelement(instance->model, GRB_DBL_ATTR_X, var_pos(i, j, instance), &x);
            quit_on_GRB_error(instance->env, instance->model, error);
            if (x > 1 - TOLERANCE) {
                double r = ((double) rand() / (RAND_MAX));

                if (r <= instance->prob) {
                    GRBsetdblattrelement(instance->model, GRB_DBL_ATTR_LB, var_pos(i, j, instance), 1.0);
                }
            }
        }
    }

    error = GRBupdatemodel(instance->model);
    quit_on_GRB_error(instance->env, instance->model, error);
}

void remove_subtour(Tsp_prob *instance, double *solution, int num_conn_comp, Connected_component *conn_comps, int (*var_pos)(int, int, Tsp_prob *)) {

    int size = 2 * num_conn_comp;

    int *tour = calloc(size, sizeof(int));

    int *root = calloc(num_conn_comp, sizeof(int));

    get_root(root, num_conn_comp, conn_comps, instance->nnode);

    int h = 0;

    for (int i = 0; i < num_conn_comp; i++) {
        for (int j = 0; j < instance->nnode; j++) {
            if(root[i] != j) {
                if (solution[var_pos(root[i], j, instance)] > 1 - TOLERANCE) {
                    if (find(conn_comps, conn_comps->parent[j]) == root[i]) {
                        if (h == 0) {
                            tour[h] = root[i];
                            tour[size -1] = j;
                            solution[var_pos(root[i], j, instance)] = 0;
                            h++;
                            break;
                        } else {
                            tour[h] = root[i];
                            tour[h + 1] = j;
                            solution[var_pos(root[i], j, instance)] = 0;
                            h += 2;
                            break;
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < size - 1; i += 2) {
        solution[var_pos(i, i + 1, instance)] = 1;
    }

    free(root);
    free(tour);

    /*for (int i = 0; i < instance->nnode; i++) {
        node_deg[i] = 2;
    }

    int root[num_conn_comp];
    get_root(root, num_conn_comp, conn_comps, instance->nnode);

    for (int i = 0; i < num_conn_comp; i++) {
        if (node_deg[i] == 2) {
            for (int j = i + 1; j < instance->nnode; j++) {
                if (root[i] != find(conn_comps, j)) {
                    continue;
                }
                if (solution[var_pos(i, j, instance)] > 1 - TOLERANCE) {
                    node_deg[root[i]]--;
                    node_deg[j]--;
                    solution[var_pos(i, j, instance)] = 0;
                    break;
                }
            }
        }

    }

    for (int i = 0; i < instance->nnode; i++) {
        if (node_deg[i] < 2) {
            int x_comp = find(conn_comps, i);
            for (int j = i + 1; j < instance->nnode; j++) {
                int y_comp = find(conn_comps, j);
                if (x_comp != y_comp && node_deg[j] < 2) {
                    solution[var_pos(i, j, instance)] = 1;
                    node_deg[i]++;
                    node_deg[j]++;
                    continue;
                }
            }
        }
    }*/
}