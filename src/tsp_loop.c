//
// Created by samuele on 26/03/19.
//

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "common.h"
#include "utils.h"
#include "input_output.h"
#include "tsp_loop.h"

#define MAX_VARNAME_SIZE 100
#define TOLERANCE 10E-4
#define LAZY_LEVEL 2
/*LAZY_LEVEL
 * With a value of 1, the constraint can be used to cut off a feasible solution, but it won’t necessarily be pulled in if another lazy constraint also cuts off the solution.
 * With a value of 2, all lazy constraints that are violated by a feasible solution will be pulled into the model.
 * With a value of 3, lazy constraints that cut off the relaxation solution at the root node are also pulled in.
 */

typedef struct{
    int *comps; //list correlation node component
    int number_of_comps; //number of component
    int *number_of_items; //number of node in each component
    int *list_of_comps; //list of component name
    //int *visit_flag; //visited node
}Connected_comp;

/**
 * Mapping between points of an edge and position in GRB model
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos_loop(int i, int j, Tsp_prob *instance);

/**
 * Find connected components returned by the MIP solver
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param instance The pointer to the problem instance
 * @param comp The pointer to the connected component structure
 */
void find_connected_comps(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp);

/**
 * Add subtour elimination constraints to the model
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param instance The pointer to the problem instance
 * @param comp The pointer to the connected component structure
 * @param iteration Index of the added constraints
 */
void add_sec_constraints(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp, int iteration);

/**
 * Verify if the identifier of the connected component is present in the list
 * @param comp The pointer to the connected component structure
 * @param curr_comp The identifier of the current connected component
 * @param num_comp The number of node in the connected component
 * @return
 */
int has_component(Connected_comp *comp, int curr_comp, int num_comp);

/**
 * Return the solution value of the x variables
 * @param env The pointer to the gurobi environment
 * @param model The pointer to the gurobi model
 * @param xpos The memory location of the x variable
 * @return The value of x after the resolution of the model
 */
double get_solution(GRBenv *env, GRBmodel *model, int xpos);

/**
 * Free memory allocated to connected component elements
 * @param comp The pointer to the connected component structure
 */
void free_comp_struc(Connected_comp * comp);


void tsp_loop_model_create(Tsp_prob *instance){
    GRBenv *env = NULL;
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
    Connected_comp comp = {.comps = calloc(n_node, sizeof(int)),
                           .number_of_comps = 0,
                           .number_of_items = calloc(n_node, sizeof(int)),
                           .list_of_comps = NULL/*,
                           .visit_flag = calloc(n_node, sizeof(int))*/
    };

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
        }
    }


    //Env creation and starting
    error = GRBemptyenv(&env);
    if (env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }
    quit_on_GRB_error(env, loop_model, error);

    error = GRBstartenv(env);
    quit_on_GRB_error(env, loop_model, error);

    error = GRBnewmodel(env, &loop_model, instance->name, 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, loop_model, error);

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

    int done = 0;
    double solution;
    /*
     * Add SEC constraints and cicle
     */
    double time_limit = 5;
    int number_of_increments = 2;
    int number_of_iterations = 5;
    int current_number_of_increments = 0;
    int current_number_of_iterations = 0;

    int fast_phase = 1;

    int status_code = 0;

    int current_iteration = 0;
    error = GRBsetintparam(env, "RINS", 10);
    quit_on_GRB_error(env, loop_model, error);
    while (!done) {
        error = GRBupdatemodel(loop_model);
        quit_on_GRB_error(env, loop_model, error);

        error = GRBwrite(loop_model, "output_loop_model.lp");
        quit_on_GRB_error(env, loop_model, error);

        error = GRBoptimize(loop_model);
        quit_on_GRB_error(env, loop_model, error);

        error = GRBgetdblattr(loop_model, GRB_DBL_ATTR_OBJVAL, &solution);
        quit_on_GRB_error(env, loop_model, error);

        printf("Solution: %g\n", solution);

        error = GRBwrite(loop_model, "solution.sol");
        quit_on_GRB_error(env, loop_model, error);

        //parse_solution_file(instance, "solution.sol");

        /*for(int j = 0; j< instance->solution_size; j++){
            printf("SOL %d = (%d, %d)\n", j, instance->solution[j][0],instance->solution[j][1] );
        }*/
        //Get termination condition
        error = GRBgetintattr(loop_model,"Status", &status_code);
        quit_on_GRB_error(env, loop_model, error);
        DEBUG_PRINT(("status: %d\n", status_code));

        //plot_solution(instance,loop_model, env, &xpos_loop);

        find_connected_comps(env, loop_model, instance, &comp);

        DEBUG_PRINT(("Found %d connected components\n", comp.number_of_comps));

        if(status_code == GRB_TIME_LIMIT){
            current_number_of_iterations++;
        }
        //the number of iterations with this time limit is enough
        if((current_number_of_increments < number_of_increments) && (current_number_of_iterations >= number_of_iterations)){
            time_limit = time_limit * 2;
            current_number_of_increments++;
            current_number_of_iterations = 0;
            error = GRBsetdblparam(env, "TimeLimit", time_limit);
            quit_on_GRB_error(env, loop_model, error);
        }
        if(current_number_of_increments>number_of_increments){
            time_limit = INFINITY;
            fast_phase = 0;

            error = GRBsetintparam(env, "RINS", -1);
            quit_on_GRB_error(env, loop_model, error);
        }
        if (comp.number_of_comps >= 2) {
            add_sec_constraints(env, loop_model, instance, &comp, current_iteration);
        } else if(status_code == GRB_TIME_LIMIT) {
            time_limit = INFINITY;
            error = GRBsetdblparam(env, "TimeLimit", time_limit);
            quit_on_GRB_error(env, loop_model, error);
        } else {
            done = 1;
        }
        current_iteration++;
    }

    error = GRBwrite(loop_model, "solution.sol");
    quit_on_GRB_error(env, loop_model, error);

    //parse_solution_file(instance, "solution.sol");

    /*for(int j = 0; j< instance->solution_size; j++){
        printf("SOL %d = (%d, %d)\n", j, instance->solution[j][0],instance->solution[j][1] );
    }*/

    plot_solution(instance,loop_model, env, &xpos_loop);

    //Freeing memory
    free(constr_name);

    for(int i = 0 ; i < sizeof(variables_names);i++){
        free(variables_names[i]);
    }

    free(variables_names);

    free_comp_struc(&comp);

    free_gurobi(env, loop_model);

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



void find_connected_comps(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp){
    int nnode = instance -> nnode;

    for (int i = 0; i < nnode; i++) {
        comp->comps[i] = i;
        comp->number_of_items[i] = 1;
    }

    comp->number_of_comps = nnode;

    int c1, c2;

    for (int i = 0; i < nnode; i++) {
        for (int j = i + 1; j < nnode; j++) {
            if (get_solution(env, model, xpos_loop(i, j, instance)) > 1-TOLERANCE) {
                if (comp->comps[i] != comp->comps[j]) {
                    c1 = comp->comps[i];
                    c2 = comp->comps[j];
                    for (int v = 0; v < nnode; v++) { //update nodes
                        if (comp->comps[v] == c2) {
                            comp->comps[v] = c1;
                            comp->number_of_items[c1]++;
                            comp->number_of_items[c2]--;//TODO this operation can be removed, if everything works correctly, at the end of the cycle it will always be equal to 0.
                        }
                    }
                    comp->number_of_comps--;
                }
            }
        }
    }

    int num_comp = comp->number_of_comps;
    comp->list_of_comps = calloc(num_comp, sizeof(int));

    //int sort_num_items_list[num_comp];

    for (int i = 0; i < num_comp; i++) {
        comp->list_of_comps[i] = -1;
    }
    int t = 0;
    for (int i = 0; i < nnode; i++) {
        int cc = comp->comps[i];
        if (has_component(comp, cc, num_comp) != 0) {
            continue;
        }
        comp->list_of_comps[t] = cc;
        t++;
    }
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

int has_component(Connected_comp *comp, int curr_comp, int num_comp) {

    for (int i = 0; i < num_comp; i++) {
        if (curr_comp == comp->list_of_comps[i]) {
            return 1;
        }
    }

    return 0;
}

double get_solution(GRBenv *env, GRBmodel *model, int xpos) {

    double x_value;
    int error = GRBgetdblattrelement(model, "X", xpos, &x_value);
    quit_on_GRB_error(env, model, error);
    return x_value;
}

void free_comp_struc(Connected_comp * comp) {
    free(comp->comps);
    free(comp->list_of_comps);
    free(comp->number_of_items);
    //free(comp->visit_flag);
}