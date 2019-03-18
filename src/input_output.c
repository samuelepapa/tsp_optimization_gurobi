#include "common.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "input_output.h"
#include "utils.h"
#include "plot_graph.h"

#define MODEL_TOLERANCE 10E-4

/**
 * Adds an edge to the solution array. If the size is negative an error is triggered, and exits with 1.
 * @param instance The Tsp_prob instance where to add the edge.
 * @param edge The edge to be added, allocate memory before passing the pointer.
 */
void add_edge_to_solution(Solution_list *edges_list, int *edge);

void parse_input(int argc, char **argv, Tsp_prob *instance) {
    int c;
    size_t filename_length;
    while ((c = getopt(argc, argv, "f:v::t::")) != -1) {
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
            case 't':
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

    int useless_chars = 3;
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
                printf("pointer_line: |%c|\n", pointer_to_line[0]);
                if(pointer_to_line[0] == ':'){
                    useless_chars = 3;
                }else if(pointer_to_line[0] == ' '){
                    useless_chars = 2;
                }
                // the useless_chars is due to first space and newline characters
                str_len = strlen(pointer_to_line) - useless_chars;
                instance->name = calloc(str_len, 1);
                //remove the first space
                strncpy(instance->name, pointer_to_line + useless_chars - 1, str_len);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                printf("param_content: |%s|\n", instance->name);
            }

            if (strncmp(param, "COMMENT", 7) == 0) {
                if(pointer_to_line[0] == ':'){
                    useless_chars = 3;
                }else if(pointer_to_line[0] == ' '){
                    useless_chars = 2;
                }
                // the useless_chars is due to first space and newline characters
                str_len = strlen(pointer_to_line) - useless_chars;
                instance->comment = calloc(str_len, 1);
                //remove the first space
                strncpy(instance->comment, pointer_to_line + useless_chars - 1, str_len);

                current_mode = 0;
                printf("param_content: |%s|\n", instance->comment);
            }

            if (strncmp(param, "TYPE", 4) == 0) {
                if(pointer_to_line[0] == ':'){
                    useless_chars = 3;
                }else if(pointer_to_line[0] == ' '){
                    useless_chars = 2;
                }
                str_len = strlen(pointer_to_line) - useless_chars;
                strncpy(buffer, pointer_to_line + useless_chars + 1, str_len);
                if (strncmp(buffer, "TSP", 3) == 0) {
                    instance->type = 0;
                }

                current_mode = 0;
                printf("param_content: |%d|\n", instance->type);
            }

            if (strncmp(param, "DIMENSION", 9) == 0) {
                if(pointer_to_line[0] == ':'){
                    useless_chars = 3;
                }else if(pointer_to_line[0] == ' '){
                    useless_chars = 2;
                }
                str_len = strlen(pointer_to_line) - useless_chars;
                //remove the first space and copy in buffer
                strncpy(buffer, pointer_to_line + useless_chars - 1, str_len);
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
                if(pointer_to_line[0] == ':'){
                    useless_chars = 3;
                }else if(pointer_to_line[0] == ' '){
                    useless_chars = 2;
                }
                str_len = strlen(pointer_to_line) - useless_chars;
                strncpy(buffer, pointer_to_line + useless_chars - 1, str_len);

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
    //For the moment this is useless, it tries to interpret different files
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

    if (instance->nnode < 1) {
        printf("Unable to import dimension.\n");
        valid_instance = 0;
    }

    /*for (int i = 0; i < instance->nnode; i++) {
        printf("i: %d X: %g, Y: %g \n", i + 1, instance->coord_x[i], instance->coord_y[i]);
    }*/


    fclose(model_file);
    //TODO parse strange files too
    return valid_instance;
}
void add_edge_to_solution(Solution_list *edges_list, int *edge) {
    if(edge == NULL){
        printf("The edge is NULL, allocate it before passing it as argument. \n");
        exit(1);
    }
    if(edges_list->size == 0){
        edges_list->solution = calloc(1, sizeof(int *));
        edges_list->size = 1;
    }else if(edges_list->size > 0) {
        edges_list->size += 1;
        edges_list->solution = realloc(edges_list->solution, edges_list->size * sizeof(int *));
    }else{
        printf("Error while adding edge to solution, the size is negative. \n");
        exit(1);
    }
    edges_list->solution[edges_list->size - 1] = edge;
}

void free_solution_list(Solution_list *edges_list){
    if(edges_list->size < 0){
        printf("Solution array was not initialized (sol size is negative).\n");
        return;
    }
    for(int i = 0; i< edges_list->size; i++){
        free(edges_list->solution[i]);
    }

    //no free on edge_list because we assume that it was statically allocated
}
int plot_solution(Tsp_prob *instance, GRBmodel *model, GRBenv *env, int (*var_pos)(int, int, Tsp_prob*)){
    //this function assumes that var_pos converts coordinates into the correct identifier in the GRB model
    Solution_list edges_list={
            .size = 0
    };

    int index = -1;
    int error = 0;

    double sol;
    for(int i = 0; i<instance->nnode; i++){
        for( int j = 0; j<instance->nnode; j++){
            sol = 0;
            index = (*var_pos)(i,j,instance);
            if(index == -1){
                //this is a tsp where self edges are not defined
                continue;
            }
            error = GRBgetdblattrelement(model, "X",index, &sol);
            quit_on_GRB_error(env, model, error);
            if(sol>1-MODEL_TOLERANCE){
                int *edge = calloc(2, sizeof(int));
                edge[0] = i;
                edge[1] = j;
                add_edge_to_solution(&edges_list, edge);
            }
        }
    }

    plot_edges(&edges_list, instance);

    free_solution_list(&edges_list);

}