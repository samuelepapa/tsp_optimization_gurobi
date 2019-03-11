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

void print_GRB_error(int error, GRBenv *env, char *msg);

int in_solution(Tsp_prob *instance, int node);

void populate_solution(GRBmodel *model, Tsp_prob *instance, int node);

int parse_solution_file(Tsp_prob *instance, char *filename);

int string_to_varnumb(Tsp_prob *instance, char *varname);


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
            variables_names[coord] = (char *) calloc(MAX_VARNAME_SIZE, sizeof(char));
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
        sprintf(constr_name, "deg(%d)", i);
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

    /* double opt_value;
     instance->solution_size = 0;
     populate_solution(model, instance, -1);
     double value = 0;
     for(int i = 1; i< instance->nnode; i++){
         error = GRBgetdblattrelement(model, GRB_DBL_ATTR_X, xpos(i,0, instance), &value);
         if(error){
             printf("ERROR: %d", error);
             exit(1);
         }
         printf("node(%d,%d) sol: %g\n", i, 0, value);
         populate_solution(model, instance, i);
     }
     instance->solution_size = 54;
     printf("SIZE: %d\n", instance->solution_size);

     for(int j = 0; j< instance->solution_size; j++){
         printf("SOL %d = %g\n", j, instance->solution[j]);
     }*/

}

void print_GRB_error(int error, GRBenv *env, char *msg) {
    if (error) {
        printf("%s\n", msg);
        printf("%s\n", GRBgeterrormsg(env));
        exit(1);
    }
}

void populate_solution(GRBmodel *model, Tsp_prob *instance, int node) {
    double *temp_add;
    if (instance->solution_size == 0) {
        instance->solution = (double *) calloc(1, sizeof(double));
        instance->solution_size = 1;
    } else {
        instance->solution_size += 1;
        temp_add = (double *) realloc(instance->solution, sizeof(double) * instance->solution_size);
        if (temp_add == NULL) {
            exit(1);
        }
        instance->solution = temp_add;

    }
    if (node == -1) {
        instance->solution[0] = 0;
        return populate_solution(model, instance, 0);
    }

    double value = 0;
    int error = 0;
    int next_node = -1;
    int found = 0;
    for (int i = 0; i < instance->nnode; i++) {
        if (i != node) {

            error = GRBgetdblattrelement(model, GRB_DBL_ATTR_X, xpos(i, node, instance), &value);
            if (error) {
                printf("ERROR: %d", error);
                exit(1);
            }

            if ((int) value && !(in_solution(instance, i))) {
                found = 1;
                printf("i: %d, node: %d\n", i, node);
                next_node = i;
                printf("Next node: %d\n", next_node);
            }
        }
    }
    if (found) {
        instance->solution[instance->solution_size - 1] = next_node;
        return populate_solution(model, instance, next_node);
    } else {
        return;
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
    //buffer for param values
    char buffer[180];
    size_t str_len;
    int variable_number = 0;
    char *pointer_to_next;
    char *param;
    int id_numb = -1;//-1: yet to start/nothing found;  0:first line found; 1: second line found; 2: started saving sols
    int is_sol = 0;

    int valid_instance = 1;
    //flag to check whether nnode was defined before importing coordinates
    int began_importing_coords = 0;
    //scan entire file
    while (fgets(line, sizeof(line), solution_file) != NULL) {
        if (id_numb == -1) {
            if (strncmp(line, "# Solution for model ", 21) == 0) {
                id_numb = 0;
            } else {
                printf("Something is wrong in the solution file.\n");
            }
        } else if (id_numb == 0) {
            if (strncmp(line, "# Objective value = ", 20) == 0) {
                id_numb = 1;
            } else {
                printf("Something is wrong in the solution file.\n");
            }
        } else if (id_numb == 1) {
            if (strncmp(line, "x(", 2) == 0) {
                id_numb = 2;
                pointer_to_next = line;
                param = strsep(&pointer_to_next, ")");

                is_sol = atoi(pointer_to_next);
                if (is_sol) {
                    strcpy(buffer, param);
                    printf("Current variable name: %s \n", pointer_to_next);

                }


            } else {
                printf("Something is wrong in the solution file.\n");
            }
        } else if (id_numb == 2) {
            pointer_to_next = line;
            param = strsep(&pointer_to_next, " ");
            strcpy(buffer, param);
            printf("Current variable name: %s \n", buffer);
            variable_number = string_to_varnumb(instance, buffer);
            printf("Current variable number: %d \n", variable_number);
        }
    }
    return 1;
}

int string_to_varnumb(Tsp_prob *instance, char *varname) {
    char *number;
    char *buffer = (char *) calloc(30, sizeof(char));
    //used to keep pointer clean to free later
    char *current_token = buffer;
    char *pointer_to_next;
    int i = 0;
    int j = 0;
    strcpy(current_token, varname);
    pointer_to_next = current_token;
    current_token = strsep(&pointer_to_next, ",");
    //This is why there is a number pointer, I need to move to the right by 2, x(1
    number = current_token + 2;
    i = atoi(number);


    current_token = strsep(&pointer_to_next, ")");

    number = current_token;
    j = atoi(number);
    printf("x(%d,%d)\n", i, j);

    free(buffer);
    return xpos(i, j, instance);
}

int string_to_coords(Tsp_prob* instance,char *varname, int * coords){
char *number;
char *buffer = (char *) calloc(30, sizeof(char));
//used to keep pointer clean to free later
char *current_token = buffer;
char *pointer_to_next;
int i = 0;
int j = 0;
strcpy(current_token, varname
);
pointer_to_next = current_token;
current_token = strsep(&pointer_to_next, ",");
//This is why there is a number pointer, I need to move to the right by 2, x(1
number = current_token + 2;
i = atoi(number);


current_token = strsep(&pointer_to_next, ")");

number = current_token;
j = atoi(number);
printf("x(%d,%d)\n", i, j);

free(buffer);

coords =
return 1;
}