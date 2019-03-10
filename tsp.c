//
// Created by samuele on 07/03/19.
//

#include "common.h"
#include <stdio.h>
#include "math.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "tsp.h"
#include "utils.h"

#define MAX_VARNAME_SIZE 100

void print_GRB_error(int error, GRBenv * env, char * msg);

void preprocessing_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *model = NULL;
    int error = 0;
    int nnode = instance->nnode;
    int n_variables = (int)(0.5 * ( nnode*nnode - nnode));
    printf("%d",n_variables);
    double upper_bounds[n_variables];
    double lower_bounds[n_variables];
    char variable_type[n_variables];
    double objective_coeffs[n_variables];
    char ** variables_names = (char **) calloc(n_variables, sizeof(char*));

    int coord = 0;
    for(int i = 0; i < nnode; i++) {
        for (int j = i + 1; j < nnode; j++) {
            coord = xpos(i, j, instance);
            upper_bounds[coord] = 1.0;
            lower_bounds[coord] = 0.0;
            variable_type[coord] = GRB_BINARY;
            objective_coeffs[coord] = distance(i, j, instance);
            variables_names[coord] = (char * )calloc(MAX_VARNAME_SIZE, sizeof(char));
            sprintf(variables_names[coord], "x(%d,%d)", i+1, j+1);
            printf("i: %d, ; j: %d\n", i+1, j+1);
        }
    }


    //Env creation and starting
    error = GRBemptyenv(&env);
    if (env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }
    print_GRB_error(error, env, "Error in environment creation.\n");
    error = GRBstartenv(env);
    print_GRB_error(error, env, "Error in environment starting.\n");

    error = GRBnewmodel(env, &model, instance->name, 0, NULL, NULL, NULL, NULL, NULL);
    print_GRB_error(error, env, "Error in creation of new model.\n");

    error = GRBaddvars(model, n_variables, 0, NULL, NULL, NULL,
            objective_coeffs, lower_bounds, upper_bounds, variable_type, variables_names);
    print_GRB_error(error, env, "Error in adding variables.\n");

    error = GRBwrite(model, "output_model.lp");
    print_GRB_error(error, env, "Error in output");
}
void print_GRB_error(int error, GRBenv * env, char * msg){
    if(error){
        printf("Error: couldn't start environment.\n");
        printf("%s\n",GRBgeterrormsg(env));
        exit(1);
    }
}