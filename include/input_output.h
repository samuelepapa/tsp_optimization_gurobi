//
// Created by samuele on 10/03/19.
//

#ifndef TSP_OPTIMIZATION_GUROBI_INPUTOUTPUT_H
#define TSP_OPTIMIZATION_GUROBI_INPUTOUTPUT_H
/**
 * Parse the input from command line
 * @param argc The number of arguments
 * @param argv The arguments themselves
 * @param instance The pointer to the problem instance we are using
 */
void parse_input(int argc, char **argv, Tsp_prob *instance);

/**
 * Initialize the problem instance
 * @param instance The pointer to the problem instance
 * @return 0 if not a valid instance, 1 otherwise
 */
int init_instance(Tsp_prob *instance);


int plot_solution(Tsp_prob *instance, GRBmodel *model, GRBenv *env, int (*var_pos)(int, int, Tsp_prob*));

// print the help text for command line
void print_help();
#endif //TSP_OPTIMIZATION_GUROBI_INPUTOUTPUT_H
