//
// Created by samuele on 26/03/19.
//
#include "tsp_loop.h"

#define MAX_VARNAME_SIZE 100
#define LAZY_LEVEL 2
/*LAZY_LEVEL
 * With a value of 1, the constraint can be used to cut off a feasible solution, but it won’t necessarily be pulled in if another lazy constraint also cuts off the solution.
 * With a value of 2, all lazy constraints that are violated by a feasible solution will be pulled into the model.
 * With a value of 3, lazy constraints that cut off the relaxation solution at the root node are also pulled in.
 */
/**
 * Add subtour elimination constraints to the model
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param instance The pointer to the problem instance
 * @param comp The pointer to the connected component structure
 * @param iteration Index of the added constraints
 */
void add_sec_constraints(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp, int iteration);

void tsp_loop_model_generate(Tsp_prob *instance) {
    GRBenv *env = instance->env;
    GRBmodel *loop_model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_variables = (int) (0.5 * (n_node * n_node - n_node)); //this number is always even
    DEBUG_PRINT(("%d", n_variables));
    double upper_bounds[n_variables];
    double lower_bounds[n_variables];
    char variable_type[n_variables];
    double objective_coeffs[n_variables];
    char **variables_names = (char **) calloc((size_t) n_variables, sizeof(char *));
    int size_variable_names = 0;

    int coord = 0;
    //Create variables
    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            coord = xpos_loop(i, j, instance);
            upper_bounds[coord] = 1.0;
            lower_bounds[coord] = 0.0;
            variable_type[coord] = GRB_BINARY;
            objective_coeffs[coord] = distance(i, j, instance);
            variables_names[coord] = (char *) calloc(MAX_VARNAME_SIZE, sizeof(char)); //TODO dealloc after
            sprintf(variables_names[coord], "x(%d,%d)", i + 1, j + 1);
            DEBUG_PRINT(("i: %d, ; j: %d\n", i + 1, j + 1));
            size_variable_names++;
        }
    }

    if (env == NULL) {
        printf("Env was NULL\n");
        //Env creation and starting
        error = GRBloadenv(&env, "tsp_loop.log");
        if (error || env == NULL) {
            printf("Error: couldn't create empty environment.\n");
            exit(1);
        }
        instance->env = env;
    }

    double curtimelimit = -1;
    error = GRBgetdblparam(env, "TimeLimit", &curtimelimit);
    printf("erros: %d\n", error);
    printf("Time limit is: %g\n", curtimelimit);

    error = GRBnewmodel(env, &loop_model, instance->name, 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, loop_model, error);
    instance->model = loop_model;

    //Add variables to loop_model
    error = GRBaddvars(loop_model, n_variables, 0, NULL, NULL, NULL,
                       objective_coeffs, lower_bounds, upper_bounds, variable_type, variables_names);
    quit_on_GRB_error(env, loop_model, error);


    //Define constraints
    int constr_index[n_node - 1];
    double constr_value[n_node - 1];
    int k = 0;
    double rhs = 2.0;
    char *constr_name = (char *) calloc(100, sizeof(char));
    int index_cur_constr = 0; //count number of constraints, useful when I add lazy constraints

    for (int i = 0; i < n_node; i++) {
        k = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                constr_index[k] = xpos_loop(i, j, instance);
                constr_value[k] = 1.0;
                k = k + 1;
            }
        }
        sprintf(constr_name, "deg(%d)", i + 1);
        error = GRBaddconstr(loop_model, n_node - 1, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, loop_model, error);
        index_cur_constr++;
    }

    //Freeing memory
    free(constr_name);

    for (int i = 0; i < size_variable_names; i++) {
        free(variables_names[i]);
    }

    free(variables_names);

}

void tsp_loop_model_run(Tsp_prob *instance) {
    int error;
    //setup clock, timelimit doesn't work because of repeated runs
    struct timespec start, cur;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int time_limit_reached = 0;
    int n_node = instance->nnode;
    GRBmodel *loop_model = instance->model;
    GRBenv *env = instance->env;

    Connected_comp comp = {.comps = calloc(n_node, sizeof(int)),
            .number_of_comps = 0,
            .number_of_items = calloc(n_node, sizeof(int)),
            .list_of_comps = NULL/*,
                           .visit_flag = calloc(n_node, sizeof(int))*/
    };

    int done = 0;
    double solution;
    /*
     * Add SEC constraints and cycle
     */
    //Update the model using new constraints
    error = GRBupdatemodel(loop_model);
    quit_on_GRB_error(env, loop_model, error);
    int numnoz = 0;
    error = GRBgetintattr(loop_model, "NumNZs", &numnoz);
    quit_on_GRB_error(env, loop_model, error);
    printf("IterationLimit: %g\n", ((double) numnoz) / 10);
    double node_limit = (double) numnoz / 10;
    int max_increments = 2;
    int max_iterations = 10;
    int cur_iter = 0;
    int cur_incr = 0;
    int count_nolimit = 0;

    int fast_phase = 1;

    int status_code = 0;

    int current_iteration = 0;
    error = GRBsetdblparam(GRBgetenv(loop_model), "IterationLimit", node_limit);
    quit_on_GRB_error(GRBgetenv(loop_model), loop_model, error);

    //error = GRBsetintparam(GRBgetenv(loop_model), "OutputFlag", 0);
    //quit_on_GRB_error(GRBgetenv(loop_model), loop_model, error);

    int seed = -1;
    error = GRBgetintparam(GRBgetenv(loop_model), "Seed", &seed);
    quit_on_GRB_error(GRBgetenv(loop_model), loop_model, error);

    printf("Current seed: %d\n", seed);

    //error = GRBsetintparam(env, GRB_INT_PAR_RINS, 10);
    //quit_on_GRB_error(env, loop_model, error);

    while (!done) {
        //Update the model using new constraints
        error = GRBupdatemodel(loop_model);
        quit_on_GRB_error(env, loop_model, error);

        //write to output file
        //error = GRBwrite(loop_model, "output_loop_model.lp");
        //quit_on_GRB_error(env, loop_model, error);

        //Run optimization
        error = GRBoptimize(loop_model);
        quit_on_GRB_error(env, loop_model, error);

        //get the current solution
        error = GRBgetdblattr(loop_model, GRB_DBL_ATTR_OBJVAL, &solution);
        quit_on_GRB_error(env, loop_model, error);
        DEBUG_PRINT(("Solution: %g\n", solution));

        //write solution to file for inspection
        //error = GRBwrite(loop_model, "solution.sol");
        //quit_on_GRB_error(env, loop_model, error);

        //Get termination condition
        error = GRBgetintattr(loop_model, "Status", &status_code);
        quit_on_GRB_error(env, loop_model, error);

        DEBUG_PRINT(("status: %d\n", status_code));

        plot_solution(instance, loop_model, env, &xpos_loop);

        //Find the connected components
        find_connected_comps(env, loop_model, instance, &comp, &xpos_loop);

        DEBUG_PRINT(("Found %d connected components\n", comp.number_of_comps));

        //I have stopped because of the limit imposed, so increment the counter
        if (status_code == GRB_ITERATION_LIMIT) {
            DEBUG_PRINT(("IterationLimit REACHED\n"));
            cur_iter++;
        } else {
            count_nolimit++;
        }
        //the number of iterations with this node limit is enough, try and give more time
        if ((cur_incr < max_increments) && (cur_iter >= max_iterations)) {
            node_limit = node_limit * 3;
            //I have incremented once
            cur_incr++;
            //I need to start counting again
            cur_iter = 0;
            //Set the new node limit
            error = GRBsetdblparam(GRBgetenv(loop_model), "IterationLimit", node_limit);
            quit_on_GRB_error(GRBgetenv(loop_model), loop_model, error);
        }
        //I have reached maximum iterations
        if (cur_incr > max_increments) {
            node_limit = INFINITY;
            //Reactivate RINS
            //error = GRBsetintparam(env, GRB_INT_PAR_RINS, -1);
            //quit_on_GRB_error(env, loop_model, error);
        }
        //I have gone 10 iterations without reaching the time limit, start applying it again
        if (count_nolimit > 10) {
            cur_incr = 0;
            cur_iter = 0;
            node_limit = 10;
            count_nolimit = 0;
        }

        //I have found connected components, add sec
        if (comp.number_of_comps >= 2) {
            add_sec_constraints(env, loop_model, instance, &comp, current_iteration);
        } else if (status_code == GRB_ITERATION_LIMIT) {
            //I have reached the node limit in this iteration but only 1 connected component.
            //Let it run again with no limit
            node_limit = INFINITY;
            error = GRBsetdblparam(GRBgetenv(loop_model), "IterationLimit", node_limit);
            quit_on_GRB_error(GRBgetenv(loop_model), loop_model, error);
        } else if (status_code == GRB_OPTIMAL) {
            //I have found no connected components and the solution was found without limits
            done = 1;
        } else if (status_code == GRB_SOLUTION_LIMIT) {
            //I have reached the maximum amount of solutions allowed, this means that I am in a warm start
            DEBUG_PRINT(("Solution limit reached in loop method. \n"));
            done = 1;
        }
        current_iteration++;
        free(comp.list_of_comps);
        //check if timelimit has been reached this is something for testing performance
        clock_gettime(CLOCK_MONOTONIC, &cur);
        if ((cur.tv_sec - start.tv_sec) > instance->time_limit) {
            time_limit_reached = 1;
            printf("TIME LIMIT REACHED, no more computing, this is taking too long.\n");
            break;
        }

    }

    plot_solution(instance, loop_model, env, &xpos_loop);

    if (time_limit_reached) {
        instance->status = GRB_TIME_LIMIT;
    } else {
        error = GRBgetintattr(loop_model, "Status", &instance->status);
        quit_on_GRB_error(env, loop_model, error);
        if (instance->status == GRB_OPTIMAL) {
            error = GRBgetdblattr(loop_model, GRB_DBL_ATTR_OBJVAL, &instance->best_solution);
        }
    }

    free_comp_struc(&comp);
}

void tsp_loop_model_create(Tsp_prob *instance){

    tsp_loop_model_generate(instance);
    tsp_loop_model_run(instance);

    GRBfreemodel(instance->model);
    //free_gurobi(env, loop_model);

}

int xpos_loop(int i, int j, Tsp_prob *instance){
    if(i==j) {
        return -1;
    }
    if(i>j){
        return xpos_loop(j, i, instance);
    }
    return i*instance->nnode + j - ((i+1)*(i+2))/2;
}

void add_sec_constraints(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp, int iteration) {
    int error;
    int nnz = 0; //number of non-zero value
    double rhs;
    int nnode = instance->nnode;
    int n_comps = comp->number_of_comps;
    int selected_comp = -1;

    char *constr_name = (char *) calloc(100, sizeof(char));

    for (int c = 0; c < n_comps; c++) {
        selected_comp = comp->list_of_comps[c];
        int n_item = comp->number_of_items[selected_comp];
        rhs = n_item - 1;
        int total_item = (n_item * (n_item- 1)) / 2;
        int constr_index[total_item];
        double constr_value[total_item];
        nnz = 0;
        for (int i = 0; i < nnode; i++) {
            if (comp->comps[i] != selected_comp) {
                continue;
            }

            for (int j = i + 1; j < nnode; j++) {
                if (comp->comps[j] == selected_comp) {
                    constr_index[nnz] = xpos_loop(i, j, instance);
                    constr_value[nnz] = 1.0;
                    nnz++;
                }

            }
        }

        sprintf(constr_name, "add_constr_subtour_%d_%d", iteration, selected_comp);

        error = GRBaddconstr(model, nnz, constr_index, constr_value, GRB_LESS_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, model, error);

        //error = GRBsetintattrelement(model, "Lazy", index_cur_constr, LAZY_LEVEL);
        //quit_on_GRB_error(env, model, error);
        //index_cur_constr++;
    }

    free(constr_name);
}