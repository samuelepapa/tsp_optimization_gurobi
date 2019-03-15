#include "common.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "inputOutput.h"


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
            case 'v':
                //verbosity level
                instance->verbosity = atoi(optarg);
                break;
            case 'tlim':
                //time limit
                instance->time_limit = atof(optarg);
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

                current_mode = 0;
                printf("param_content: |%d|\n", instance->nnode);
            }


            /**
            * for symmetric travelling salesman problems:
            *  0 = EUC_2D       : weights are Euclidean distances in 2-D
            *  1 = MAX_2D       : weights are maximum distances in 2-D
            *  2 = MAN_2D       : weights are Manhattan distances in 2-D
            *  3 = CEIL_2D      : weights are Euclidean distances in 2-D rounded up
            *  4 = GEO          : weights are geographical distances
            *  5 = ATT          : special distance function for problems att48 and att532 (pseudo-Euclidean)
            */
            if (strncmp(param, "EDGE_WEIGHT_TYPE", 16) == 0) {
                str_len = strlen(pointer_to_line) - 3;
                strncpy(buffer, pointer_to_line + 2, str_len);

                if (strncmp(buffer, "EUC_2D", 6) == 0) {
                    instance->weight_type = 0;
                }

                if (strncmp(buffer, "MAX_2D", 6) == 0) {
                    instance->weight_type = 1;
                }

                if (strncmp(buffer, "MAN_2D", 6) == 0) {
                    instance->weight_type = 2;
                }

                if (strncmp(buffer, "CEIL_2D", 7) == 0) {
                    instance->weight_type = 3;
                }

                if (strncmp(buffer, "GEO", 3) == 0) {
                    instance->weight_type = 4;
                }

                if (strncmp(buffer, "ATT", 3) == 0) {
                    instance->weight_type = 5;
                }

                if (strncmp(buffer, "EXPLICIT", 8) == 0) {
                    printf("%s\n", "Wrong edge weight type, this program resolve only 2D TSP case with coordinate type.");
                    exit(1);
                }

                if (strncmp(buffer, "SPECIAL", 7) == 0 || strncmp(buffer, "EUC_3D", 6) == 0 ||
                    strncmp(buffer, "EUC_3D", 6) == 0 || strncmp(buffer, "MAN_3D", 6) == 0 || strncmp(buffer, "XRAY1", 3) == 0 ||
                    strncmp(buffer, "XRAY2", 3) == 0) {
                    printf("%s\n", "Wrong edge weight type, this program resolve only 2D TSP case.");
                    exit(1);
                }

                current_mode = 0;
                printf("param_content: |%d|\n", instance->weight_type);
            }

            if (strncmp(param, "NODE_COORD_SECTION", 18) == 0) {
                //initialize the the array of coordinates
                instance->coord_x = calloc((size_t) instance->nnode, sizeof(double));
                instance->coord_y = calloc((size_t) instance->nnode, sizeof(double));

                current_mode = 1;
                //This is done because I start reading the next line immediately
                //without waiting for the next cycle
                if (fgets(line, sizeof(line), model_file) == NULL) {
                    printf("After the coordinate section delimiter the file ended.");
                }
                pointer_to_line = line;
                param = strsep(&pointer_to_line, ": ");
            }

            /*if (strncmp(param, "EDGE_WEIGHT_SECTION", 19) == 0) {

                int n_node = instance->nnode;
                int **arr = (int **)malloc(n_node * sizeof(int *));
                for (int i=0; i < n_node; i++) {
                    arr[i] = (int *)malloc(n_node * sizeof(int));
                }

                current_mode = 2;
                //This is done because I start reading the next line immediately
                //without waiting for the next cycle
                if (fgets(line, sizeof(line), model_file) == NULL) {
                    printf("After the coordinate section delimiter the file ended.");
                }
                pointer_to_line = line;
                param = strsep(&pointer_to_line, ": ");

            }*/

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
    //For the moment this is useless
    /*
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
     */

    /*if (began_importing_coords == 2 || instance->nnode < 1) {
        printf("Unable to import coordinates, they or the dimension might be missing.\n");
        valid_instance = 0;
    }*/

    /*for (int i = 0; i < instance->nnode; i++) {
        printf("i: %d X: %g, Y: %g \n", i + 1, instance->coord_x[i], instance->coord_y[i]);
    }*/


    fclose(model_file);
    //TODO parse strange files too
    return valid_instance;
}
