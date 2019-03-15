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
#include "tsp.h"
#include "utils.h"

#define MAX_VARNAME_SIZE 100

void print_GRB_error(int error, GRBenv *env, char *msg);

int in_solution(Tsp_prob *instance, int node);

int varname_to_varnum(Tsp_prob *instance, char *varname);


void preprocessing_model_create(Tsp_prob *instance) {
    GRBenv *env = NULL;
    GRBmodel *model = NULL;
    int error = 0;
    int nnode = instance->nnode;
    int n_variables = (int) (0.5 * (nnode * nnode - nnode));
    printf("%d", n_variables);
    double upper_bounds[n_variables];
    double lower_bounds[n_variables];
    char variable_type[n_variables];
    double objective_coeffs[n_variables];
    char **variables_names = (char **) calloc(n_variables, sizeof(char *));

    int coord = 0;
    for (int i = 0; i < nnode; i++) {
        for (int j = i + 1; j < nnode; j++) {
            coord = xpos(i, j, instance);
            upper_bounds[coord] = 1.0;
            lower_bounds[coord] = 0.0;
            variable_type[coord] = GRB_BINARY;
            objective_coeffs[coord] = distance(i, j, instance);
            variables_names[coord] = (char *) calloc(MAX_VARNAME_SIZE, sizeof(char)); //TODO dealloc after
            sprintf(variables_names[coord], "x(%d,%d)", i + 1, j + 1);
            printf("i: %d, ; j: %d\n", i + 1, j + 1);
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

    int indexes[nnode - 1];
    double coefficients[nnode - 1];
    int k = 0;
    double rhs = 2.0;
    char *constr_name = (char *) calloc(100, sizeof(char));

    for (int i = 0; i < nnode; i++) {
        k = 0;
        for (int j = 0; j < nnode; j++) {
            if (i != j) {
                indexes[k] = xpos(i, j, instance);
                coefficients[k] = 1.0;
                k = k + 1;
            }
        }
        sprintf(constr_name, "deg(%d)", i+1);
        error = GRBaddconstr(model, nnode - 1, indexes, coefficients, GRB_EQUAL, rhs, constr_name);
        print_GRB_error(error, env, "Error in adding constraint.\n");
    }

    error = GRBupdatemodel(model);
    print_GRB_error(error, env, "Error in updating the model.\n");

    error = GRBwrite(model, "output_model.lp");
    print_GRB_error(error, env, "Error in output");

    error = GRBoptimize(model);
    print_GRB_error(error, env, "Error in optimization.\n");

    double solution;

    error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &solution);
    print_GRB_error(error, env, "Error in getting solution.\n");

    printf("Solution: %g\n", solution);

    error = GRBwrite(model, "solution.sol");
    print_GRB_error(error, env, "Error in printing solution.\n");

    parse_solution_file(instance, "solution.sol");

    for(int j = 0; j< instance->solution_size; j++){
        printf("SOL %d = (%d, %d)\n", j, instance->solution[j][0],instance->solution[j][1] );
    }

}

void print_GRB_error(int error, GRBenv *env, char *msg) {
    if (error) {
        printf("%s\n", msg);
        printf("%s\n", GRBgeterrormsg(env));
        exit(1);
    }
}

int in_solution(Tsp_prob *instance, int node) {
    int found = 0;
    for (int i = 0; i < instance->solution_size; i++) {
        if ((int) (instance->solution[i]) == node) {
            found = 1;
        }
    }
    return found;
}

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