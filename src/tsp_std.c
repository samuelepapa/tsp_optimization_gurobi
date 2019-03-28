//
// Created by samuele on 07/03/19.
//

#include "common.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "tsp_std.h"
#include "utils.h"
#include "input_output.h"

#define MAX_VARNAME_SIZE 100

int varname_to_varnum(Tsp_prob *instance, char *varname);

/**
 * Mapping between points of an edge and position in GRB model
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos(int i, int j, Tsp_prob *instance);


void tsp_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *model = NULL;
    int error = 0;
    int n_node = instance->nnode;
    int n_variables = (int) (0.5 * (n_node * n_node - n_node)); //this number is always even
    DEBUG_PRINT(("%d", n_variables));
    double upper_bounds[n_variables];
    double lower_bounds[n_variables];
    char variable_type[n_variables];
    double objective_coeffs[n_variables];
    char **variables_names = (char **) calloc(n_variables, sizeof(char *));

    int coord = 0;
    //Create variables
    for (int i = 0; i < n_node; i++) {
        for (int j = i + 1; j < n_node; j++) {
            coord = xpos(i, j, instance);
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
    quit_on_GRB_error(env, model, error);

    error = GRBstartenv(env);
    quit_on_GRB_error(env, model, error);

    error = GRBnewmodel(env, &model, instance->name, 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, model, error);

    //Add variables to model
    error = GRBaddvars(model, n_variables, 0, NULL, NULL, NULL,
                       objective_coeffs, lower_bounds, upper_bounds, variable_type, variables_names);
    quit_on_GRB_error(env, model, error);


    //Define constraints
    int indexes[n_node - 1];
    double coefficients[n_node - 1];
    int k = 0;
    double rhs = 2.0;
    char *constr_name = (char *) calloc(100, sizeof(char));

    for (int i = 0; i < n_node; i++) {
        k = 0;
        for (int j = 0; j < n_node; j++) {
            if (i != j) {
                indexes[k] = xpos(i, j, instance);
                coefficients[k] = 1.0;
                k = k + 1;
            }
        }
        sprintf(constr_name, "deg(%d)", i+1);
        error = GRBaddconstr(model, n_node - 1, indexes, coefficients, GRB_EQUAL, rhs, constr_name);
        quit_on_GRB_error(env, model, error);
    }

    error = GRBupdatemodel(model);
    quit_on_GRB_error(env, model, error);

    error = GRBwrite(model, "output_model.lp");
    quit_on_GRB_error(env, model, error);

    error = GRBoptimize(model);
    quit_on_GRB_error(env, model, error);

    double solution;

    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &solution);
    quit_on_GRB_error(env, model, error);

    printf("Solution: %g\n", solution);

    error = GRBwrite(model, "solution.sol");
    quit_on_GRB_error(env, model, error);

    //parse_solution_file(instance, "solution.sol");

    /*for(int j = 0; j< instance->solution_size; j++){
        printf("SOL %d = (%d, %d)\n", j, instance->solution[j][0],instance->solution[j][1] );
    }*/

    plot_solution(instance,model, env, &xpos);

    //Freeing memory
    free(constr_name);
    for(int i = 0 ; i < sizeof(variables_names);i++){
        free(variables_names[i]);
    }
    free(variables_names);

    free_gurobi(env, model);

}
/*
int parse_solution_file(Tsp_prob *instance, char *filename) {
    //opening file
    FILE *solution_file = fopen(filename, "r");
    if (solution_file == NULL) {
        printf("Solution file not found or filename not defined.\n");
        exit(1);
    }
    //buffer for line
    char line[180];
    char *pointer_to_next;
    char *param;
    int file_position = -1;//-1: yet to start/nothing found;  0:first line found; 1: second line found, start saving sols
    int is_sol = 0;

    int valid_file = 1;

    instance->solution_size = 0;


    //scan entire file
    while ((fgets(line, sizeof(line), solution_file) != NULL) && valid_file) {
        if (file_position == -1) {
            if (strncmp(line, "# Solution for model ", 21) == 0) {
                file_position = 0;
            } else {
                printf("Something is wrong in the solution file.\n");
                valid_file = 0;
            }
        } else if (file_position == 0) {
            if (strncmp(line, "# Objective value = ", 20) == 0) {
                file_position = 1;
            } else {
                printf("Something is wrong in the solution file.\n");
                valid_file = 0;
            }
        } else if (file_position == 1) {
            if (strncmp(line, "x(", 2) == 0) {
                pointer_to_next = line;
                param = strsep(&pointer_to_next, ")");
                is_sol = atoi(pointer_to_next);
                if (is_sol) {
                    int * edge = calloc(2, sizeof(int)); //TODO free this
                    string_to_coords(param, edge);
                    printf("Current edge: (%d, %d) \n", edge[0], edge[1]);
                    add_edge_to_solution(instance, edge);
                }


            } else {
                printf("Something is wrong in the solution file.\n");
                valid_file = 0;
            }
        }
    }
    return valid_file;
}
*/
int varname_to_varnum(Tsp_prob *instance, char *varname) {
    char *number;
    char *buffer = (char *) calloc(30, sizeof(char));
    //used to keep pointer clean to free later
    char *current_token = buffer;
    char *pointer_to_next;
    int i = 0;
    int j = 0;

    //copy locally
    strcpy(current_token, varname);
    pointer_to_next = current_token;
    current_token = strsep(&pointer_to_next, ",");
    //This is why there is a number pointer, I need to move to the right by 2, x(1
    number = current_token + 2;
    i = atoi(number);

    current_token = strsep(&pointer_to_next, ")");
    number = current_token;
    j = atoi(number);

    free(buffer);

    return xpos(i, j, instance);
}

int string_to_coords(char *varname, int *edge) {
    char *number;
    char *buffer = (char *) calloc(30, sizeof(char));
    //used to keep pointer clean to free later
    char *current_token = buffer;
    char *pointer_to_next;
    //init coordinates, if they are 0 something is wrong, it starts counting from 1.
    int i = 0;
    int j = 0;

    //local copy
    strcpy(current_token, varname);
    pointer_to_next = current_token;
    current_token = strsep(&pointer_to_next, ",");
    //This is why there is a number pointer, I need to move to the right by 2, x(1
    number = current_token + 2;
    i = atoi(number);

    //Next number
    current_token = strsep(&pointer_to_next, ")");
    number = current_token;
    j = atoi(number);


    //temporary array
    int *p = calloc(2, sizeof(int));
    p[0] = i;
    p[1] = j;
    //copy it inside the edge pointer
    memcpy(edge, p, 2 * sizeof(int));

    free(p);
    free(buffer);

    return 1;
}

int xpos(int i, int j, Tsp_prob *instance){
    if(i==j) {
        //printf("Index i=j\n");
        //exit(1);
        return -1;
    }
    if(i>j){
        return xpos(j, i, instance);
    }
    return i*instance->nnode + j - ((i+1)*(i+2))/2;
}