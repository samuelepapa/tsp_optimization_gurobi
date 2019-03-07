//
// Created by samuele on 05/03/19.
//

#include "common.h"
#include "utils.h"
#include "tsp.h"
#include "plotGraph.h"

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

/**
 * Plot the instance passed in the argument
 * @param instance The pointer to the problem instance
 */
void plot_instance(Tsp_prob *instance);

/**
 * Free memory to avoid leaks, assumes instance is initialized as variable, not dinamically allocated
 * @param instance The pointer to the problem instance
 */
void close_instance(Tsp_prob *instance);

// print the help text for command line
void print_help();

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Not enough arguments.\n");
        exit(1);
    }

    Tsp_prob instance = {
            .nnode = -1
    };
    //GRBerror message.
    int valid_instance = 0;

    parse_input(argc, argv, &instance);

    valid_instance = init_instance(&instance);

    if (valid_instance) {

        plot_instance(&instance);

        close_instance(&instance);

    }else{
        printf("Error in parsing file");
        exit(1);
    }
    return 0;
}

void parse_input(int argc, char **argv, Tsp_prob *instance) {
    int c;
    size_t filename_length;
    while ((c = getopt(argc, argv, "f:")) != -1) {
        switch (c) {
            case 'f':
                filename_length = strlen(optarg);
                instance->filename = calloc(filename_length, 1);
                strcpy(instance->filename, optarg);
                printf("Filename: %s \n", instance->filename);
                break;
            case '?':
                printf("This is the guide.\n");
                break;
            default:
                printf("Something went wrong in input.\n");
        }
    }
}

int init_instance(Tsp_prob *instance) {
    //opening file
    FILE *model_file = fopen(instance->filename, "r");
    if (model_file == NULL) {
        printf("File not found or filename not defined.\n");
        exit(1);
    }

    //buffer for line
    char line[180];
    //buffer for param values
    char buffer[180];
    size_t str_len;
    char *pointer_to_line;
    char *param;
    int id_numb = 0;
    int current_mode = 0; //0: scanning parameters. 1: input coordinates. -1: param found

    int valid_instance = 1;
    //flag to check whether nnode was defined before importing coordinates
    int began_importing_coords = 0;
    //scan entire file
    while (fgets(line, sizeof(line), model_file) != NULL) {
        if (strncmp(line, "EOF", 3) == 0) {
            printf("File ended.\n");
            current_mode = -1;
        } else {
            pointer_to_line = line;
            param = strsep(&pointer_to_line, ": ");


            printf("param: %s\n", param);
            if (strncmp(param, "NAME", 4) == 0) {
                // the -2 is due to first space and newline characters
                str_len = strlen(pointer_to_line) - 3;
                instance->name = calloc(str_len, 1);
                //remove the first space
                strncpy(instance->name, pointer_to_line + 2, str_len);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                printf("param_content: |%s|\n", instance->name);
            }
            if (strncmp(param, "COMMENT", 7) == 0) {
                // the -2 is due to first space and newline characters
                str_len = strlen(pointer_to_line) - 3;
                instance->comment = calloc(str_len, 1);
                //remove the first space
                strncpy(instance->comment, pointer_to_line + 2, str_len);

                current_mode = 0;
                printf("param_content: |%s|\n", instance->comment);
            }
            if (strncmp(param, "TYPE", 4) == 0) {
                str_len = strlen(pointer_to_line) - 3;
                strncpy(buffer, pointer_to_line + 2, str_len);
                if (strncmp(buffer, "TSP", 3) == 0) {
                    instance->type = 0;
                }

                current_mode = 0;
                printf("param_content: |%d|\n", instance->type);
            }
            if (strncmp(param, "DIMENSION", 9) == 0) {
                str_len = strlen(pointer_to_line) - 3;
                //remove the first space and copy in buffer
                strncpy(buffer, pointer_to_line + 2, str_len);
                //convert to int safely (possible error catching in future)
                instance->nnode = (int) strtol(buffer, NULL, 10);

                //initialize the the array of coordinates
                instance->coord_x = calloc((size_t) instance->nnode, sizeof(double));
                instance->coord_y = calloc((size_t) instance->nnode, sizeof(double));

                current_mode = 0;
                printf("param_content: |%d|\n", instance->nnode);
            }
            if (strncmp(param, "EDGE_WEIGHT_TYPE", 16) == 0) {
                str_len = strlen(pointer_to_line) - 2;
                strncpy(buffer, pointer_to_line + 1, str_len);

                if (strncmp(buffer, "ATT", 3) == 0) {
                    instance->weight_type = 0;
                }
                current_mode = 0;
                printf("param_content: |%d|\n", instance->weight_type);
            }
            if (strncmp(param, "NODE_COORD_SECTION", 18) == 0) {
                current_mode = 1;
                //This is done because I start reading the next line immediately
                //without waiting for the next cycle
                if (fgets(line, sizeof(line), model_file) == NULL) {
                    printf("After the coordinate section delimiter the file ended.");
                }
                pointer_to_line = line;
                param = strsep(&pointer_to_line, ": ");
            }
            if (current_mode == 1 && instance->nnode > 0) {
                began_importing_coords = 1;
                //the number of the line
                id_numb = (int) strtol(param, NULL, 10);
                //the first coordinate
                param = strsep(&pointer_to_line, " \n");
                instance->coord_x[id_numb - 1] = (double) strtol(param, NULL, 10);
                //the second coordinate
                param = strsep(&pointer_to_line, " \n");
                instance->coord_y[id_numb - 1] = (double) strtol(param, NULL, 10);
            }
        }
    }

    // This means that the dimension was given after the coordinates, or coordinates are missing
    if (instance->nnode > 0 && began_importing_coords == 0) {
        fseek(model_file, 0, SEEK_SET);
        current_mode = 0;
        while (fgets(line, sizeof(line), model_file) != NULL) {
            if (strncmp(line, "EOF", 3) == 0) {
                printf("File ended.\n");
                current_mode = -1;
            } else {
                pointer_to_line = line;
                param = strsep(&pointer_to_line, ": ");
                printf("param: %s\n", param);
                if (strncmp(param, "NODE_COORD_SECTION", 18) == 0) {
                    current_mode = 1;
                    //This is done because I start reading the next line immediately
                    //without waiting for the next cycle
                    if (fgets(line, sizeof(line), model_file) == NULL) {
                        fprintf(stderr, "After the coordinate section delimiter the file ended.");
                    }
                    pointer_to_line = line;
                    param = strsep(&pointer_to_line, ": ");
                }
                if (current_mode == 1) {
                    began_importing_coords = 2;
                    //the number of the line
                    id_numb = (int) strtol(param, NULL, 10);
                    //the first coordinate
                    param = strsep(&pointer_to_line, " \n");
                    instance->coord_x[id_numb - 1] = (double) strtol(param, NULL, 10);
                    //the second coordinate
                    param = strsep(&pointer_to_line, " \n");
                    instance->coord_y[id_numb - 1] = (double) strtol(param, NULL, 10);
                }

            }
        }

    }

    if (began_importing_coords == 2 || instance->nnode < 1) {
        printf("Unable to import coordinates, they or the dimension might be missing.\n");
        valid_instance = 0;
    }

    /*for (int i = 0; i < instance->nnode; i++) {
        printf("i: %d X: %g, Y: %g \n", i + 1, instance->coord_x[i], instance->coord_y[i]);
    }*/


    fclose(model_file);
    //TODO parse strange files too
    return valid_instance;
}

void close_instance(Tsp_prob *instance) {
    free(instance->name);
    free(instance->comment);
    free(instance->filename);
    free(instance->coord_y);
    free(instance->coord_x);
    free(instance->solution);
}

void plot_instance(Tsp_prob *instance) {
    /*Points *points;
    GPC_Plot *plot1;
    points = calloc((size_t) instance->nnode, sizeof(GPC_Plot));

    // to display all points in graph
    double smallest_x = instance->coord_x[0];
    double smallest_y = instance->coord_y[0];
    double biggest_x = instance->coord_x[0];
    double biggest_y = instance->coord_y[0];

    //populate points
    for (int i = 0; i < instance->nnode; i++) {
        points[i].x = instance->coord_x[i];
        points[i].y = instance->coord_y[i];
        // find edges
        if (points[i].x < smallest_x) smallest_x = points[i].x;
        if (points[i].x > biggest_x) biggest_x = points[i].x;
        if (points[i].y < smallest_y) smallest_y = points[i].y;
        if (points[i].y > biggest_y) biggest_y = points[i].y;
    }

    plot1 = gpc_init_xy("Plot of points", smallest_x - 200, smallest_y - 200, biggest_x + 200, biggest_y + 200);
    gpc_plot_xy(plot1, points, instance->nnode, "points", "circles", "blue");
    plot_line(instance, plot1, 0, 1);
    plot_line(instance, plot1, 10, 11);
     */
    plot_nodes(instance);
    //plot_path(instance);
}