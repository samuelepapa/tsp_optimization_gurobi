//
// Created by samuele on 15/05/19.
//

#include "common.h"
#include "utils.h"
#include "peaceful_queens_optimization.h"


/**
 * Mapping between points of an edge and position in GRB model
 * @param i First point
 * @param j Second point
 * @param instance The pointer to the problem instance
 * @return The memory position
 */
int xpos_queens(int i, int j, int n_squares);

int ypos_queens(int i, int j, int n_squares);


void tsp_queens_model_create() {
    GRBenv *env = NULL;
    GRBmodel *model = NULL;
    int error = 0;
    int n_squares = 14;
    int n_variables = (int) (2 * (n_squares * n_squares));
    DEBUG_PRINT(("%d\n", n_variables));
    double upper_bounds[n_variables];
    double lower_bounds[n_variables];
    char variable_type[n_variables];
    double objective_coeffs[n_variables];
    char **variables_names = (char **) calloc(n_variables, sizeof(char *));
    int size_variable_names = 0;

    int coordx = 0;
    int coordy = 0;
    //Create variables
    for (int i = 0; i < n_squares; i++) {
        for (int j = 0; j < n_squares; j++) {
            coordx = xpos_queens(i, j, n_squares);
            objective_coeffs[coordx] = 1.0;

            upper_bounds[coordx] = 1.0;
            lower_bounds[coordx] = 0.0;
            variable_type[coordx] = GRB_BINARY;
            variables_names[coordx] = (char *) calloc(180, sizeof(char)); //TODO dealloc after
            size_variable_names++;
            sprintf(variables_names[coordx], "x(%d,%d)", i + 1, j + 1);
            DEBUG_PRINT(("X: i: %d, ; j: %d\n", i + 1, j + 1));

            coordy = ypos_queens(i, j, n_squares);
            objective_coeffs[coordy] = 1.0;

            upper_bounds[coordy] = 1.0;
            lower_bounds[coordy] = 0.0;
            variable_type[coordy] = GRB_BINARY;
            variables_names[coordy] = (char *) calloc(180, sizeof(char)); //TODO dealloc after
            size_variable_names++;
            sprintf(variables_names[coordy], "y(%d,%d)", i + 1, j + 1);
            DEBUG_PRINT(("Y: i: %d, ; j: %d coordy: %d\n", i + 1, j + 1, coordy));
        }
    }

    printf("creating model\n");
    //Env creation and starting
    error = GRBemptyenv(&env);
    if (env == NULL) {
        printf("Error: couldn't create empty environment.\n");
        exit(1);
    }
    quit_on_GRB_error(env, model, error);

    error = GRBstartenv(env);
    quit_on_GRB_error(env, model, error);

    error = GRBnewmodel(env, &model, "peaceful_queens", 0, NULL, NULL, NULL, NULL, NULL);
    quit_on_GRB_error(env, model, error);

    //Add variables to model
    error = GRBaddvars(model, n_variables, 0, NULL, NULL, NULL,
                       objective_coeffs, lower_bounds, upper_bounds, variable_type, variables_names);
    quit_on_GRB_error(env, model, error);

    error = GRBsetintattr(model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE);
    quit_on_GRB_error(env, model, error);

    //Define constraints
    int length_array = n_squares + 1;
    int indexes[length_array];
    double coefficients[length_array];
    int m = 0;
    double rhs = n_squares;
    char *constr_name = (char *) calloc(100, sizeof(char));

    printf("adding rows constr xy\n");
    for (int k = 0; k < n_squares; k++) {
        for (int i = 0; i < n_squares; i++) {
            m = 0;
            for (int j = 0; j < n_squares; j++) {
                indexes[m] = xpos_queens(i, j, n_squares);
                coefficients[m] = 1.0;
                m++;
            }
            indexes[m] = ypos_queens(i, k, n_squares);
            coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "row_xy(%d, %d)", i + 1, k + 1);
            error = GRBaddconstr(model, n_squares + 1, indexes, coefficients, GRB_LESS_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }
    printf("adding rows constr yx\n");
    for (int k = 0; k < n_squares; k++) {
        for (int i = 0; i < n_squares; i++) {
            m = 0;
            for (int j = 0; j < n_squares; j++) {
                indexes[m] = ypos_queens(i, j, n_squares);
                coefficients[m] = 1.0;
                m++;
            }
            indexes[m] = xpos_queens(i, k, n_squares);
            coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "row_yx(%d, %d)", i + 1, k + 1);
            error = GRBaddconstr(model, n_squares + 1, indexes, coefficients, GRB_LESS_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }
    printf("adding column constr xy\n");
    for (int k = 0; k < n_squares; k++) {
        for (int j = 0; j < n_squares; j++) {
            m = 0;
            for (int i = 0; i < n_squares; i++) {
                indexes[m] = xpos_queens(i, j, n_squares);
                coefficients[m] = 1.0;
                m++;
            }
            indexes[m] = ypos_queens(k, j, n_squares);
            coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "column_xy(%d, %d)", j + 1, k + 1);
            error = GRBaddconstr(model, n_squares + 1, indexes, coefficients, GRB_LESS_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }
    printf("adding column constr yx\n");
    for (int k = 0; k < n_squares; k++) {
        for (int j = 0; j < n_squares; j++) {
            m = 0;
            for (int i = 0; i < n_squares; i++) {
                indexes[m] = ypos_queens(i, j, n_squares);
                coefficients[m] = 1.0;
                m++;
            }
            indexes[m] = xpos_queens(k, j, n_squares);
            coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "column_yx(%d, %d)", j + 1, k + 1);
            error = GRBaddconstr(model, n_squares + 1, indexes, coefficients, GRB_LESS_EQUAL, rhs, constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }

    int diagonal_indexes[n_squares + 1];
    double diagonal_coefficients[n_squares + 1];
    rhs = n_squares;

    printf("adding diagonal constr xy\n");

    for (int l = 0; l < n_squares; l++) {
        for (int k = 0; k < n_squares - l; k++) {
            m = 0;
            for (int i = 0; i < n_squares - l; i++) {
                diagonal_indexes[m] = xpos_queens(i, i + l, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = ypos_queens(k, k + l, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "diagonal_xy(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, n_squares - l + 1, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }
    printf("adding diagonal constr yx\n");

    for (int l = 0; l < n_squares; l++) {
        for (int k = 0; k < n_squares - l; k++) {
            m = 0;
            for (int i = 0; i < n_squares - l; i++) {
                diagonal_indexes[m] = ypos_queens(i, i + l, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = xpos_queens(k, k + l, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "diagonal_yx(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, n_squares - l + 1, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }

    printf("adding diagonal j constr xy\n");
    for (int l = 1; l < n_squares; l++) {
        for (int k = 0; k < n_squares - l; k++) {
            m = 0;
            for (int j = 0; j < n_squares - l; j++) {
                diagonal_indexes[m] = xpos_queens(j + l, j, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = ypos_queens(k + l, k, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "diagonal_j_xy(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, n_squares - l + 1, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }
    printf("adding diagonal j constr yx\n");
    for (int l = 1; l < n_squares; l++) {
        for (int k = 0; k < n_squares - l; k++) {
            m = 0;
            for (int j = 0; j < n_squares - l; j++) {
                diagonal_indexes[m] = ypos_queens(j + l, j, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = xpos_queens(k + l, k, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "diagonal_j_yx(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, n_squares - l + 1, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }

    printf("adding opposite diagonal constr xy\n");

    for (int l = 0; l < n_squares - 1; l++) {
        for (int k = 0; k <= l; k++) {
            m = 0;
            for (int i = 0; i <= l; i++) {
                diagonal_indexes[m] = xpos_queens(n_squares - 1 - i, n_squares - 1 + i - l, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = ypos_queens(n_squares - k - 1, n_squares + k - l - 1, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "opposite_diagonal_xy(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, l + 2, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }


    printf("adding opposite diagonal constr yx\n");

    for (int l = 0; l < n_squares - 1; l++) {
        for (int k = 0; k <= l; k++) {
            m = 0;
            for (int i = 0; i <= l; i++) {
                diagonal_indexes[m] = ypos_queens(n_squares - i - 1, n_squares + i - l - 1, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = xpos_queens(n_squares - k - 1, n_squares + k - l - 1, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "opposite_diagonal_yx(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, l + 2, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }

    printf("adding opposite diagonal j constr xy\n");
    for (int l = 0; l < n_squares - 1; l++) {
        for (int k = 0; k <= l; k++) {
            m = 0;
            for (int j = 0; j <= l; j++) {
                diagonal_indexes[m] = xpos_queens(l - j, j, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = ypos_queens(l - k, k, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "diagonal_opposite_j_xy(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, l + 2, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }
    printf("adding opposite diagonal j constr yx\n");
    for (int l = 0; l < n_squares; l++) {
        for (int k = 0; k <= l; k++) {
            m = 0;
            for (int j = 0; j <= l; j++) {
                diagonal_indexes[m] = ypos_queens(l - j, j, n_squares);
                diagonal_coefficients[m] = 1.0;
                m++;
            }
            diagonal_indexes[m] = xpos_queens(l - k, k, n_squares);
            diagonal_coefficients[m] = n_squares;
            m++;
            sprintf(constr_name, "diagonal_opposite_j_yx(%d, %d)", l + 1, k + 1);
            error = GRBaddconstr(model, l + 2, diagonal_indexes, diagonal_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }

    int third_const_indexes[2];
    double third_const_coefficients[2];
    rhs = 1.0;
    printf("adding third constr\n");
    for (int i = 0; i < n_squares; i++) {
        for (int j = 0; j < n_squares; j++) {
            third_const_indexes[0] = xpos_queens(i, j, n_squares);
            third_const_coefficients[0] = 1.0;
            third_const_indexes[1] = ypos_queens(i, j, n_squares);
            third_const_coefficients[1] = 1.0;
            sprintf(constr_name, "same_square(%d,%d)", i + 1, j + 1);
            error = GRBaddconstr(model, 2, third_const_indexes, third_const_coefficients, GRB_LESS_EQUAL, rhs,
                                 constr_name);
            quit_on_GRB_error(env, model, error);
        }
    }

    int fifth_constr_indexes[2 * n_squares * n_squares];
    double fifth_constr_coefficients[2 * n_squares * n_squares];
    rhs = 0.0;
    m = 0;
    printf("adding fifth constr\n");
    for (int i = 0; i < n_squares; i++) {
        for (int j = 0; j < n_squares; j++) {
            fifth_constr_indexes[m] = xpos_queens(i, j, n_squares);
            fifth_constr_coefficients[m] = 1.0;
            m++;
            fifth_constr_indexes[m] = ypos_queens(i, j, n_squares);
            fifth_constr_coefficients[m] = -1.0;
            m++;
        }
    }
    sprintf(constr_name, "equality");
    error = GRBaddconstr(model, 2 * n_squares * n_squares, fifth_constr_indexes, fifth_constr_coefficients, GRB_EQUAL,
                         rhs,
                         constr_name);
    quit_on_GRB_error(env, model, error);


    error = GRBupdatemodel(model);
    quit_on_GRB_error(env, model, error);

    error = GRBwrite(model, "peaceful_queens.lp");
    quit_on_GRB_error(env, model, error);

    error = GRBoptimize(model);
    quit_on_GRB_error(env, model, error);

    double solution;
    int status;

    error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &status);
    quit_on_GRB_error(env, model, error);

    if (status == GRB_OPTIMAL) {
        error = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &solution);
        quit_on_GRB_error(env, model, error);

        DEBUG_PRINT(("Solution: %g\n", solution));
    } else if (status == GRB_TIME_LIMIT) {
        DEBUG_PRINT(("Not enough time\n"));
    }
    error = GRBwrite(model, "solution.sol");
    quit_on_GRB_error(env, model, error);

    //parse_solution_file(instance, "solution.sol\n");

    /*for(int j = 0; j< instance->solution_size; j++){
        printf("SOL %d = (%d, %d)\n", j, instance->solution[j][0],instance->solution[j][1] );
    }*/

    //Freeing memory
    free(constr_name);
    for (int i = 0; i < size_variable_names; i++) {
        free(variables_names[i]);
    }
    free(variables_names);

    free_gurobi(env, model);

}

int xpos_queens(int i, int j, int n_squares) {
    return n_squares * i + j;
}

int ypos_queens(int i, int j, int n_squares) {
    return (n_squares * n_squares) + (n_squares * i + j);
}
