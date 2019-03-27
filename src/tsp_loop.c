//
// Created by samuele on 26/03/19.
//

#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
    int *comps;
    int number_of_comps;
    int *number_of_items;
    int *list_of_comps;
    int *visit_flag;
}Connected_comp;

/**
 * Mapping between points of an edge and position in GRB model
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos_loop(int i, int j, Tsp_prob *instance);

//void tsp_loop(Tsp_prob *instance);

void find_connected_comps(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp);

void add_sec_constraints(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp, int index_cur_constr);

int has_component(const int *comp_list, int curr_comp, int num_comp);

double get_solution(GRBenv *env, GRBmodel *model, int xpos);


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
    Connected_comp comp = {.comps = NULL,
                           .number_of_comps = 0,
                           .number_of_items = NULL,
                           .list_of_comps = NULL,
                           .visit_flag = NULL
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
        sprintf(constr_name, "deg(%d)", i+1);
        error = GRBaddconstr(loop_model, n_node - 1, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, loop_model, error);
        index_cur_constr++;
    }

    int done = 0;
    double solution;

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

        find_connected_comps(env, loop_model, instance, &comp);

        if (comp.number_of_comps >= 2) {
            add_sec_constraints(env, loop_model, instance, &comp, index_cur_constr);
        } else {
            done = 1;
        }
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

/*void tsp_loop(Tsp_prob *instance){
    int nnode = instance -> nnode;
    int comp[nnode];
    int comp_count;
    //find_connected_comps()
}*/

void find_connected_comps(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp){
    int nnode = instance -> nnode;
    int comp_list[nnode];
    int num_item_list[nnode];
    int flag[nnode];

    if (comp->comps == NULL) {

        for (int i = 0; i < nnode; i++) {
            comp_list[i] = i;
            num_item_list[i] = 1;
            flag[i] = 0;
        }
        comp->number_of_comps = nnode;
        comp->visit_flag = flag;
    }

    for (int i = 0; i < nnode; i++) {
        for (int j = 0; j < nnode; j++) {
            if (get_solution(env, model, xpos_loop(i, j, instance))>1-TOLERANCE) {
                if (comp_list[i] != comp_list[j]) {
                    int c1 = comp_list[i];
                    int c2 = comp_list[j];
                    for (int v = 0; v < nnode; v++) {
                        if (comp_list[v] == c2) {
                            comp_list[v] = c1;
                            num_item_list[c1]++;
                            num_item_list[c2]--;
                        }
                    }
                    comp->number_of_comps--;
                }
            }
        }
    }

    int num_comp = comp->number_of_comps;
    int sort_comp[num_comp];
    int new_num_items_list[num_comp];

    for (int i = 0; i < num_comp; i++) {
        sort_comp[i] = -1;
        new_num_items_list[i] = -1;
    }

    for (int i = 0; i < nnode; i++) {
        int cc = comp_list[i];
        if (has_component(sort_comp, cc, num_comp) != 0) {
            continue;
        }
        sort_comp[i] = cc;
        new_num_items_list[i] = num_item_list[cc];
    }

    comp->list_of_comps = sort_comp;
    comp->number_of_items = new_num_items_list;
}

void add_sec_constraints(GRBenv *env, GRBmodel *model, Tsp_prob *instance, Connected_comp *comp, int index_cur_constr) {
    int error;
    int nnz = 0; //number of non-zero value
    double rhs = -1.0;
    int nnodes = instance->nnode;
    int n_comps = comp->number_of_comps;
    int selected_comp = 0;

    char *constr_name = (char *) calloc(100, sizeof(char));

    for (int c = 0; c < n_comps; c++) {
        selected_comp = comp->list_of_comps[c];
        for (int i = 0; (i < nnodes) && !(comp->visit_flag[i]) ; i++) {
            if (comp->comps[i] != selected_comp) {
                continue;
            }
            rhs++;
            int constr_index[comp->number_of_items[c]];
            double constr_value[comp->number_of_items[c]];
            for (int j = i + 1; (j < nnodes) && !(comp->visit_flag[j]); j++) {
                if (comp->comps[j] == selected_comp) {
                    constr_index[nnz] = xpos_loop(i, j, instance);
                    constr_value[nnz] = 1.0;
                    sprintf(constr_name, "lazy_constr_(%d,%d)", i + 1, j + 1);
                    nnz++;
                    comp->visit_flag[j] = 1;
                }
            }

            comp->visit_flag[i] = 1;

            error = GRBaddconstr(model, nnz, constr_index, constr_value, GRB_LESS_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, model, error);

            error = GRBsetintattrelement(model, "Lazy", index_cur_constr, LAZY_LEVEL);
            quit_on_GRB_error(env, model, error);
            index_cur_constr++;

        }
    }



    free(constr_name);
}

int has_component(const int *comp_list, int curr_comp, int num_comp) {

    for (int i = 0; i < num_comp; i++) {
        if (curr_comp == comp_list[i]) {
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