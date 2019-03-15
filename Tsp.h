//
// Created by samuele on 07/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_TSP_H
#define TSP_OPTIMIZATION_GUROBI_TSP_H

void preprocessing_model_create(Tsp_prob *instance);

/**
 * Used to parse a solution file in the .sol format
 * @param instance the Tsp_prob instance where the solution should be saved //TODO check that name is the correct one
 * @param filename path to file, file extension need to be included e.g. "solution.sol"
 * @return 0 if errors occurred, 1 otherwise
 */
int parse_solution_file(Tsp_prob *instance, char *filename);

/**
 * convert a string in the x(X,Y) format, where X and Y are numbers into edge coordinates. Other formats are also fine,
 * important is that the comma is present and that it ends with the second number or a closed parentheses.
 * @param varname The variable name in the x(X,Y) format
 * @param edge pointer to where the eedge should be saved
 * @return always 1
 */
int string_to_coords(char *varname, int *edge);

#endif //TSP_OPTIMIZATION_GUROBI_TSP_H
