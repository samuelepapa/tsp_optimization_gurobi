//
// Created by davide on 13/03/19.
//
#include "common.h"
#include <stdio.h>
#include "math.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "tspMTZ.h"

int ypos(int i, int j, Tsp_prob * instance){

    return i*instance->nnode + j;
}

void print_GRB_error(int error, GRBenv *env, char *msg) {
    if (error) {
        printf("%s\n", msg);
        printf("%s\n", GRBgeterrormsg(env));
        exit(1);
    }
}

void quit(GRBenv *env, GRBmodel *model, int error) {

    /*error reporting*/
    if(error) {
        printf("ERROR: %s\n", GRBgeterrormsg(env));
        exit(1);
    }

    /*free model*/
    GRBfreemodel(model);

    /*free environment*/
    GRBfreeenv(env);
}

void preprocessing_MTZ_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *MTZ_model = NULL;
    int error = 0;
    int n_nodes = instance->nnode;
    int n_variables = n_nodes*n_nodes;
    printf("Number of nodes: %d\n Number of variables: %d\n", n_nodes, n_variables);

    char var_type[n_variables];
    double low_bound[n_variables];
    double up_bound[n_variables];
    double obj_coeff[n_variables];
    int optim_status;
    double obj_val;

    char **variables_names = (char **) calloc(n_variables, sizeof(char *));

    int coord = 0;

    for (int i = 0; i < n_nodes; i++) {
        for (int j = 0; j < n_nodes; j++) {
            coord = ypos(i, j, instance);
            var_type[coord] = GRB_BINARY;
            low_bound[coord] = 0;
            if (i == j) {
                up_bound[coord] = 0;
            } else {
                up_bound[coord] = 1;
            }
            obj_coeff[coord] = distance(i, j, instance);
            variables_names[coord] = (char*) calloc(100, sizeof(char));
            sprintf(variables_names[coord], "y(%d,%d)", i + 1, j + 1);
            printf("i:%d, j: %d\n", i + 1, j + 1);

        }
    }


    /*create environment*/
    error = GRBloadenv(&env, "MTZ.log");
    if(error || env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }

    /*Create an empty model*/
    error = GRBnewmodel(env, &MTZ_model, "MTZ", 0, NULL, NULL, NULL, NULL, NULL);

    if(error) {
        quit(env, MTZ_model, error);
    }

    /* Change objective sense to maximization */
    /*error = GRBsetintattr(MTZ_model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
    if (error) {
        quit(env, MTZ_model, error);
    }*/

    /*Add objective function elements*/
    error = GRBaddvars(MTZ_model, n_variables, 0, NULL, NULL, NULL, obj_coeff, low_bound, up_bound, var_type, variables_names);
    if (error) {
        quit(env, MTZ_model, error);
    }

    int constr_index[n_nodes];
    double constr_value[n_nodes];
    double rhs = 1.0;
    char *constr_name = (char *) calloc(100, sizeof(char));

    /*Add constraints for indegree*/
    for (int h = 0; h < n_nodes; h++) {
        for (int j = 0; j < n_nodes; j++) {
            constr_index[j] = ypos(j, h, instance);
            constr_value[j] = 1.0;
        }

        sprintf(constr_name, "indeg(%d)", h + 1);
        error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        if (error) {
            quit(env, MTZ_model, error);
        }
    }

    /*Add constraints for outdegree*/
    for (int h = 0; h < n_nodes; h++) {
        for (int j = 0; j < n_nodes; j++) {
            constr_index[j] = ypos(h, j, instance);
            constr_value[j] = 1.0;
        }

        sprintf(constr_name, "outdeg(%d)", h + 1);
        error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, rhs, constr_name);
        if (error) {
            quit(env, MTZ_model, error);
        }
    }

    /*Add lazy constraints for y(i,j) + y(j, i) <= 1*/
    /*int constr_ind[2];
    double constr_val[2] = {1.0, 1.0};

    for (int i = 0; i < n_nodes; i++) {
        for (int j = i; j < n_nodes; j++) {
            constr_ind[0] = ypos(i, j, instance);
            constr_ind[1] = ypos(j, i, instance);
            sprintf(constr_name, "lazy constr 1 (%d, %d)", i+1, j+1);
            error = GRBaddconstr(MTZ_model, 2, constr_ind, constr_val, GRB_LESS_EQUAL, rhs, constr_name);
            if (error) {
                quit(env, MTZ_model, error);
            }
            error = GRBsetintattrelement(MTZ_model, "Lazy", 2*n_nodes + i + j, 1);
            if (error) {
                quit(env, MTZ_model, error);
            }
            error = GRBupdatemodel(MTZ_model);
            if (error) {
                quit(env, MTZ_model, error);
            }
        }
    }*/

    /*Add constrain y(i, i) = 0 <=> 0 <= y(i,i) <= 0*/

    /*for (int i = 0; i < n_nodes; i++) {
        constr_index[i] = ypos(i, i, instance);
        constr_value[i] = 1.0;
    }

    error = GRBaddconstr(MTZ_model, n_nodes, constr_index, constr_value, GRB_EQUAL, 0.0, NULL);
    if (error) {
        quit(env, MTZ_model, error);
    }*/

    /*consolidate the model parameters*/
    error = GRBupdatemodel(MTZ_model);
    if (error) {
        quit(env, MTZ_model, error);
    }

    /*write model in an  output file*/
    error = GRBwrite(MTZ_model, "output_MTZ_model.lp");
    if (error) {
        quit(env, MTZ_model, error);
    }

    /*launch gurobi solver with the selected model*/
    /*error = GRBoptimize(MTZ_model);
    if (error) {
        quit(env, MTZ_model, error);
    }*/

    /*free memory*/
    free(constr_name);

    for (int k = 0; k < n_variables; k++) {
        free(variables_names[k]);
    }

    free(variables_names);
}
