//
// Created by samuele on 08/04/19.
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
#include "tsp_lazycall.h"
#include "union_find.h"

#define TOLERANCE 10E-4
#define MAX_VARNAME_SIZE 128

struct callback_data {
    int nvars;
    Tsp_prob *instance;
    int (*var_pos)(int, int, Tsp_prob *);
    Graph *graph;
};

int xpos_lazycall(int i, int j, Tsp_prob *instance);

void find_connected_comps_lazycall(Tsp_prob *instance, Connected_comp *comp, double *solution);

int has_component_lazy(Connected_comp *comp, int curr_comp, int num_comp);

void add_lazy_sec_constraints(void *cbdata, struct callback_data *user_cbdata, Connected_comp *comp, int node);

double get_solution_lazy(double *solution, int xpos);

/* Define my callback function */

int __stdcall mycallback(GRBmodel *model, void *cbdata, int where, void *usrdata) {
    struct callback_data *mydata = (struct callback_data *) usrdata;
    int nvars;
    int nnode = mydata->instance->nnode;

    if(where == GRB_CB_MIPSOL){
        Connected_comp comp = {.comps = calloc(nnode, sizeof(int)),
                .number_of_comps = 0,
                .number_of_items = calloc(nnode, sizeof(int)),
                .list_of_comps = NULL
        };
        double * solution = calloc(mydata->nvars, sizeof(double));
        GRBcbget(cbdata, where, GRB_CB_MIPSOL_SOL, solution);

        find_connected_comps_lazycall(mydata->instance, &comp, solution);

        printf("\nThe current solution has: %d connected components \n", comp.number_of_comps);
        if(comp.number_of_comps >=2) {
            add_lazy_sec_constraints(cbdata, mydata, &comp, 0);
        }
        free(comp.number_of_items);

        free_comp_struc(&comp);
    }

    return 0;
}


/*int __stdcall mycallback(GRBmodel *model, void *cbdata, int where, void *usrdata) {
    struct callback_data *mydata = (struct callback_data *) usrdata;
    int nvars;
    int nnode = mydata->instance->nnode;

    if(where == GRB_CB_MIPSOL){
        Connected_component *conn_comp = calloc(nnode, sizeof(Connected_component));
        double * solution = calloc(mydata->nvars, sizeof(double));
        GRBcbget(cbdata, where, GRB_CB_MIPSOL_SOL, solution);

        int number_of_comps = union_find(mydata->graph, solution, &xpos_lazycall, mydata->instance, conn_comp);

        printf("\nThe current solution has: %d connected components \n", number_of_comps);
        if (number_of_comps >= 2) {
            //add_lazy_sec_constraints(cbdata, mydata, &comp, 0);
        }

        free(conn_comp);
    }

    return 0;
}*/

void tsp_lazycall_model_create(Tsp_prob *instance) {
    //setup clock, timelimit doesn't work because of repeated runs
    struct timespec start, cur;
    double time_elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int time_limit_reached = 0;

    GRBenv *env = instance->env;
    GRBmodel *lazycall_model = NULL;
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

    struct callback_data user_cbdata;

    user_cbdata.instance = instance;
    user_cbdata.var_pos = xpos_lazycall;

    user_cbdata.graph = malloc(sizeof(Graph));
    create_graph_u_f(instance, user_cbdata.graph);

    int coord = 0;
    //Create variables
    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            coord = xpos_lazycall(i, j, instance);
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
        DEBUG_PRINT(("Env was NULL, creating it.\n"));
        //Env creation and starting
        error = GRBloadenv(&env, NULL);
        if (error || env == NULL) {
            printf("Error: couldn't create empty environment.\n");
            exit(1);
        }
    }

    error = GRBnewmodel(env, &lazycall_model, instance->name, 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, lazycall_model, error);

    error = GRBsetintparam(GRBgetenv(lazycall_model), GRB_INT_PAR_LAZYCONSTRAINTS, 1);
    quit_on_GRB_error(env, lazycall_model, error);

    error = GRBsetintparam(GRBgetenv(lazycall_model), GRB_INT_PAR_PRECRUSH, 1);
    quit_on_GRB_error(env, lazycall_model, error);

    //Add variables to lazycall_model
    error = GRBaddvars(lazycall_model, n_variables, 0, NULL, NULL, NULL,
                       objective_coeffs, lower_bounds, upper_bounds, variable_type, variables_names);
    quit_on_GRB_error(env, lazycall_model, error);


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
                constr_index[k] = xpos_lazycall(i, j, instance);
                constr_value[k] = 1.0;
                k = k + 1;
            }
        }
        sprintf(constr_name, "deg(%d)", i + 1);
        error = GRBaddconstr(lazycall_model, n_node - 1, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, lazycall_model, error);
        index_cur_constr++;
    }

    int done = 0;
    double solution;
    /*
     * Add SEC constraints and cycle
     */
    //Update the model using new constraints
    error = GRBupdatemodel(lazycall_model);
    quit_on_GRB_error(env, lazycall_model, error);

    //get number of vars
    error = GRBgetintattr(lazycall_model, GRB_INT_ATTR_NUMVARS, &user_cbdata.nvars);
    quit_on_GRB_error(env, lazycall_model, error);

    error = GRBsetcallbackfunc(lazycall_model, mycallback, (void *) &user_cbdata);
    quit_on_GRB_error(env, lazycall_model, error);

    error = GRBoptimize(lazycall_model);
    quit_on_GRB_error(env, lazycall_model, error);

    //Freeing memory
    free(constr_name);

    for (int i = 0 ; i < size_variable_names;i++) {
        free(variables_names[i]);
    }

    free(variables_names);

    free_graph(user_cbdata.graph);

    free_gurobi(env, lazycall_model);

}

int xpos_lazycall(int i, int j, Tsp_prob *instance) {
    if (i == j) {
        return -1;
    }
    if (i > j) {
        return xpos_lazycall(j, i, instance);
    }
    return i * instance->nnode + j - ((i + 1) * (i + 2)) / 2;
}

void find_connected_comps_lazycall(Tsp_prob *instance, Connected_comp *comp, double *solution) {
    int nnode = instance -> nnode;

    for (int i = 0; i < nnode; i++) {
        comp->comps[i] = i;
        comp->number_of_items[i] = 1;
    }

    comp->number_of_comps = nnode;

    int c1, c2;

    for (int i = 0; i < nnode; i++) {
        for (int j = i + 1; j < nnode; j++) {
            if (solution[xpos_lazycall(i, j, instance)] > 1-TOLERANCE) {
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
        if (has_component_lazy(comp, cc, num_comp) != 0) {
            continue;
        }
        comp->list_of_comps[t] = cc;
        t++;
    }
}



int has_component_lazy(Connected_comp *comp, int curr_comp, int num_comp) {

    for (int i = 0; i < num_comp; i++) {
        if (curr_comp == comp->list_of_comps[i]) {
            return 1;
        }
    }

    return 0;
}
void add_lazy_sec_constraints(void *cbdata, struct callback_data *user_cbdata, Connected_comp *comp, int node) {
    int error;
    int nnz = 0; //number of non-zero value
    double rhs;
    int nnode = user_cbdata->instance->nnode;
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
                    constr_index[nnz] = user_cbdata->var_pos(i, j, user_cbdata->instance);
                    constr_value[nnz] = 1.0;
                    nnz++;
                }

            }
        }

        sprintf(constr_name, "add_constr_subtour_%d_%d", node, selected_comp);
        printf("Adding constraint: add_constr_subtour_%d_%d\n", node, selected_comp);
        error = GRBcblazy(cbdata, nnz, constr_index, constr_value, GRB_LESS_EQUAL, rhs);
        if(error){
            printf("Error on cblazy adding lazy constraints, code: %d \n", error);
        }
        //error = GRBsetintattrelement(model, "Lazy", index_cur_constr, LAZY_LEVEL);
        //quit_on_GRB_error(env, model, error);
        //index_cur_constr++;
    }

    free(constr_name);
}
/*double get_solution_lazy(double *solution, int xpos){
    double x_value;
    int error = GRBgetdblattrelement(model, "X", xpos, &x_value);
    quit_on_GRB_error(env, model, error);
    return x_value;
}*/