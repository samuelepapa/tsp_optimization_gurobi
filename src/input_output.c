#include "common.h"
#include "input_output.h"

#define MODEL_TOLERANCE 10E-4

/**
 * Adds an edge to the solution array. If the size is negative an error is triggered, and exits with 1.
 * @param instance The Tsp_prob instance where to add the edge.
 * @param edge The edge to be added, allocate memory before passing the pointer.
 */
void add_edge_to_solution(Solution_list *edges_list, int *edge);

int parse_input(int argc, char **argv, Tsp_prob *instance, Trial *trial_inst) {
    int type = -1;
    struct arg_file *filename;
    struct arg_str *model_name;
    struct arg_lit *is_this_trial, *is_this_job, *help;
    struct arg_int *seed_number, *time_limit;
    struct arg_end *end;
    void *argtable[] = {
            filename = arg_file0("f", "filename", "<filename>",
                                 "path to file where the tsp instance or a trial file is located "
                                 "(this is the interpretation if the -r argument is present)."),
            model_name = arg_str0("m", "model", "<tsp_model>",
                                  "the tsp model used to solve this (this argument is only used if -r is not present)."),
            is_this_trial = arg_lit0("r", "run",
                                     "if this should be interpreted as a test trial."),
            is_this_job = arg_lit0("j", "job", "if this should be a job to be run on a distributed system"),
            seed_number = arg_int0("s", "seed", "<int>", "seed of current run."),
            time_limit = arg_int0("t", "timelimit", "<seconds>", "time limit for the run."),
            help = arg_lit0(NULL, "help",
                            "print this help and exit"),
            end = arg_end(20)
    };
    char progname[] = "tsp_optimize";
    if (arg_nullcheck(argtable) != 0)
        printf("error: insufficient memory\n");
    int nerrors = arg_parse(argc, argv, argtable);

    if (help->count > 0) {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout, argtable, "\n");
        printf("Run tsp optimization models against specified instances.\n\n");
        arg_print_glossary(stdout, argtable, "  %-25s %s\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        exit(0);
    }
    if (nerrors == 0) {
        if (is_this_trial->count > 0) {
            type = 1;
            if (filename->count > 0) {
                //+1 due to termination \0 of strcpy
                if (strncmp(filename->extension[0], ".trl", 4) == 0) {
                    trial_inst->filename = calloc(strlen(filename->filename[0]) + 1, sizeof(char));
                    strcpy(trial_inst->filename, filename->filename[0]);
                } else {
                    printf("Incorrect file extension, it should be a .trl file, it instead was a \"%s\" file.\n",
                           filename->extension[0]);
                    exit(1);
                }
            } else {
                printf("Filename in trials is required.\n");
                exit(1);
            }
        } else if (is_this_job->count > 0) {
            if (filename->count > 0) {
                //+1 due to termination \0 of strcpy
                if (strncmp(filename->extension[0], ".tspjob", 4) == 0) {
                    trial_inst->filename = calloc(strlen(filename->filename[0]) + 1, sizeof(char));
                    strcpy(trial_inst->filename, filename->filename[0]);
                } else {
                    printf("Incorrect file extension, it should be a .trl file, it instead was a \"%s\" file.\n",
                           filename->extension[0]);
                    exit(1);
                }
            } else {
                printf("Filename in jobs is required.\n");
                exit(1);
            }
        } else {
            type = 0;
            if (filename->count > 0) {
                //+1 due to termination \0 of strcpy
                instance->filename = calloc(strlen(filename->filename[0]) + 1, sizeof(char));
                strcpy(instance->filename, filename->filename[0]);
            } else {
                printf("Filename is required.\n");
                exit(1);
            }
            if (seed_number->count > 0) {
                instance->seed = seed_number->ival[0];
            }
            if (model_name->count > 0) {
                instance->model_type = map_model_type((char *) model_name->sval[0]);
            }
            if (time_limit->count > 0) {
                instance->time_limit = time_limit->ival[0];
            }
        }
    } else {
        arg_print_errors(stdout, end, "tsp_optimize");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        exit(1);
    }
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return type;
}

int init_instance(Tsp_prob *instance) {
    //opening file
    FILE *model_file = fopen(instance->filename, "r");
    if (model_file == NULL) {

        DEBUG_PRINT(("File not found or filename: %s not defined.\n", instance->filename));
        exit(1);
    }

    //buffer for line
    char line[180];
    //buffer for param values
    char buffer[180];
    size_t str_len = 0;
    char *pointer_to_line;
    char *param;
    int id_numb = 0;
    int current_mode = 0; //0: scanning parameters. 1: input coordinates. -1: param found

    int valid_instance = 1;
    //flag to check whether nnode was defined before importing coordinates
    //int began_importing_coords = 0;

    int useless_chars = 3;
    //scan entire file
    while (fgets(line, sizeof(line), model_file) != NULL) {

        if (strncmp(line, "EOF", 3) == 0) {
            DEBUG_PRINT(("File ended.\n"));
            current_mode = -1;
        } else {
            pointer_to_line = line;
            param = strsep(&pointer_to_line, ": \t");


            DEBUG_PRINT(("param: |%s|\n", param));

            if (strncmp(param, "NAME", 4) == 0) {
                DEBUG_PRINT(("pointer_line: |%c|\n", pointer_to_line[0]));
                if (pointer_to_line[0] == ':') {
                    useless_chars = 3;
                } else if (pointer_to_line[0] == ' ') {
                    useless_chars = 2;
                }
                // the useless_chars is due to first space and newline characters
                str_len = strlen(pointer_to_line) - useless_chars;
                // +1 due to strcpy
                instance->name = calloc(str_len + 1, 1);
                //remove the first space
                strncpy(instance->name, pointer_to_line + useless_chars - 1, str_len);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                DEBUG_PRINT(("param_content: |%s|\n", instance->name));
            }

            if (strncmp(param, "COMMENT", 7) == 0) {
                if (pointer_to_line[0] == ':') {
                    useless_chars = 3;
                } else if (pointer_to_line[0] == ' ') {
                    useless_chars = 2;
                }
                // the useless_chars is due to first space and newline characters
                str_len = strlen(pointer_to_line) - useless_chars;
                instance->comment = calloc(str_len + 1, 1);
                //remove the first space
                strncpy(instance->comment, pointer_to_line + useless_chars - 1, str_len);

                current_mode = 0;
                DEBUG_PRINT(("param_content: |%s|\n", instance->comment));
            }

            if (strncmp(param, "TYPE", 4) == 0) {
                if (pointer_to_line[0] == ':') {
                    useless_chars = 3;
                } else if (pointer_to_line[0] == ' ') {
                    useless_chars = 2;
                }
                str_len = strlen(pointer_to_line) - useless_chars;
                strncpy(buffer, pointer_to_line + useless_chars + 1, str_len);
                if (strncmp(buffer, "TSP", 3) == 0) {
                    instance->type = 0;
                }

                current_mode = 0;
                DEBUG_PRINT(("param_content: |%d|\n", instance->type));
            }

            if (strncmp(param, "DIMENSION", 9) == 0) {
                if (pointer_to_line[0] == ':') {
                    useless_chars = 3;
                } else if (pointer_to_line[0] == ' ') {
                    useless_chars = 2;
                }
                str_len = strlen(pointer_to_line) - useless_chars;
                //remove the first space and copy in buffer
                strncpy(buffer, pointer_to_line + useless_chars - 1, str_len);
                //convert to int safely (possible error catching in future)
                instance->nnode = (int) strtol(buffer, NULL, 10);

                current_mode = 0;
                DEBUG_PRINT(("param_content: |%d|\n", instance->nnode));
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
                if (pointer_to_line[0] == ':') {
                    useless_chars = 3;
                } else if (pointer_to_line[0] == ' ') {
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
                    printf("%s\n",
                           "Wrong edge weight type, this program resolve only 2D TSP case with coordinate type.");
                    exit(1);
                }

                if (strncmp(buffer, "SPECIAL", 7) == 0 || strncmp(buffer, "EUC_3D", 6) == 0 ||
                    strncmp(buffer, "EUC_3D", 6) == 0 || strncmp(buffer, "MAN_3D", 6) == 0 ||
                    strncmp(buffer, "XRAY1", 3) == 0 ||
                    strncmp(buffer, "XRAY2", 3) == 0) {
                    printf("%s\n", "Wrong edge weight type, this program resolve only 2D TSP case.");
                    exit(1);
                }

                current_mode = 0;
                DEBUG_PRINT(("param_content: |%d|\n", instance->weight_type));
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
                    valid_instance = 0;
                }
                pointer_to_line = line;

                param = strsep(&pointer_to_line, " \t\n");
            }

            if (current_mode == 1 && instance->nnode > 0) {
                while (strlen(param) == 0) {
                    param = strsep(&pointer_to_line, " \t\n");
                    DEBUG_PRINT(("line: |%s|\n", param));
                }
                if (!isdigit(param[0]) && param[0] != '-') {
                    valid_instance = 0;
                    //Something is not right, stop the cycle
                    break;
                }
                //the number of the line
                id_numb = (int) strtol(param, NULL, 10);
                //the first coordinate
                param = strsep(&pointer_to_line, " \t\n");
                while (strlen(param) == 0) {
                    param = strsep(&pointer_to_line, " \t\n");
                    DEBUG_PRINT(("line: |%s|\n", param));
                }
                if (!isdigit(param[0]) && param[0] != '-') {
                    valid_instance = 0;
                    //Something is not right, stop the cycle
                    break;
                }
                instance->coord_x[id_numb - 1] = strtod(param, NULL);
                //the second coordinate
                param = strsep(&pointer_to_line, " \t\n");
                while (strlen(param) == 0) {
                    param = strsep(&pointer_to_line, " \t\n");
                    DEBUG_PRINT(("line: |%s|\n", param));
                }
                if (!isdigit(param[0]) && param[0] != '-') {
                    valid_instance = 0;
                    //Something is not right, stop the cycle
                    break;
                }
                instance->coord_y[id_numb - 1] = strtod(param, NULL);
            }
        }
    }

    /*FILE *fin = fopen(instance->input_file, "r");

	if (fin == NULL) {
		print_error("File not found");
	}

	instance->num_nodes = -1;

	int coordinate_section = 0;

	char text_line[200];
	char *param;
	char *token;

	while(fgets(text_line, sizeof(text_line), fin) != NULL) {

		printf("\nLine read: %s \n", text_line);

		if(strlen(text_line) <= 1) {
			continue;
		}

		param = strtok(text_line, " :");

		printf("Parameter name: %s \n", param);

		if(strncmp(param, "NAME", 4) == 0) {
			coordinate_section = 0;
			continue;
		}

		if(strncmp(param, "COMMENT", 7) == 0) {
			coordinate_section = 0;
			continue;
		}

		if(strncmp(param, "TYPE", 4) == 0) {
			token = strtok(NULL, " :");
			if(strncmp(token, "TSP", 3) == 0) {
				instance->type = 0;
				coordinate_section = 0;
				continue;
			} else {
				print_error("Type is not TSP");
				break;
			}
		}

		if(strncmp(param, "DIMENSION", 9) == 0) {
			if(instance->num_nodes >= 0){
				print_error("File contains another DIMENSION parameter.");
				continue;
			}
			token = strtok(NULL, " :");
			instance->num_nodes = atoi(token);
			printf("---Number of nodes: %d \n", instance->num_nodes);
			coordinate_section = 0;
			continue;

		}

        if (strncmp(param, "EDGE_WEIGHT_TYPE", 16) == 0) {
            if (pointer_to_line[0] == ':') {
                useless_chars = 3;
            } else if (pointer_to_line[0] == ' ') {
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
                printf("%s\n",
                       "Wrong edge weight type, this program resolve only 2D TSP case with coordinate type.");
                exit(1);
            }

            if (strncmp(buffer, "SPECIAL", 7) == 0 || strncmp(buffer, "EUC_3D", 6) == 0 ||
                strncmp(buffer, "EUC_3D", 6) == 0 || strncmp(buffer, "MAN_3D", 6) == 0 ||
                strncmp(buffer, "XRAY1", 3) == 0 ||
                strncmp(buffer, "XRAY2", 3) == 0) {
                printf("%s\n", "Wrong edge weight type, this program resolve only 2D TSP case.");
                exit(1);
            }

            current_mode = 0;
            printf("param_content: |%d|\n", instance->weight_type);
            continue;
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
                valid_instance = 0;
            }
            pointer_to_line = line;

            param = strsep(&pointer_to_line, " ");
            continue;
        }

        if(strncmp(param, "EOF", 3) == 0) {
            coordinate_section = 0;
            printf("End of file \n");
            break;
        }

        if(coordinate_section == 1) {
            int i = atoi(param) - 1;
            if(i < 0 || i >= instance->num_nodes) {
                print_error("--- unknown node in NODE_COORD_SECTION");
            }
            token = strtok(NULL, " :,");
            instance->coord_x[i] = atof(token);
            token = strtok(NULL, " :,");
            instance->coord_y[i] = atof(token);
            printf("---Node %d has coordinate (%f, %f) \n", i+1, instance->coord_x[i], instance->coord_y[i]);
            continue;
        }

        printf(" final coordinate section %d\n", coordinate_section);
        print_error(" ... wrong format for the current parser!");
    }

    fclose(fin);

     */

    if (instance->nnode < 1) {
        printf("Unable to import dimension.\n");
        valid_instance = 0;
    }

    for (int i = 0; i < instance->nnode; i++) {
        DEBUG_PRINT(("i: %d X: %g, Y: %g \n", i + 1, instance->coord_x[i], instance->coord_y[i]));
    }


    fclose(model_file);
    //TODO parse strange files too
    return valid_instance;
}

int init_trial(Trial *trial_inst) {
    //opening file
    FILE *trial_file = fopen(trial_inst->filename, "r");
    if (trial_file == NULL) {
        DEBUG_PRINT(("Trial file not found or filename not defined.\n"));
        exit(1);
    }

    DEBUG_PRINT(("Starting Trial name: %s\n", trial_inst->name));

    //buffer for line
    char line[180];
    //buffer for param values
    char buffer[180];
    size_t str_len = 0;
    int current_mode = 0; //0: scanning parameters. 1: input seeds. -1: param found

    int valid_instance = 1;
    //flag to check whether nnode was defined before importing coordinates
    //int began_importing_coords = 0;

    int cur_element = 0;
    //scan entire file

    while (fgets(line, sizeof(line), trial_file) != NULL) {

        if (strncmp(line, "EOF", 3) == 0) {
            DEBUG_PRINT(("File ended.\n"));
            current_mode = -1;
            break;
        } else {
            /*pointer_to_line = line;
            param = strsep(&pointer_to_line, ": \t");
            */

            DEBUG_PRINT(("Line: |%s|\n", line));

            //MODE CHECK
            //parsing seeds
            if (current_mode == 1) {

                if (strncmp(line, "-1", 2) == 0) {
                    //TODO generate seed randomly
                } else {
                    if (cur_element < trial_inst->n_runs) {
                        if (isdigit(line[0])) {
                            trial_inst->seeds[cur_element] = (int) strtol(line, NULL, 10);
                            cur_element++;
                        } else {
                            valid_instance = 0;
                            break;
                        }
                    } else {
                        current_mode = 0;
                    }

                }
            }
            if (current_mode == 2) {
                if (cur_element < trial_inst->n_instances) {
                    str_len = strlen(line);

                    //remove_tr_newline(line);
                    trial_inst->instances[cur_element] = calloc(str_len, 1);
                    strncpy(trial_inst->instances[cur_element], line, str_len - 1);
                    cur_element++;
                } else {
                    current_mode = 0;
                }
            }
            if (current_mode == 3) {
                if (cur_element < trial_inst->n_models) {
                    str_len = strlen(line);
                    strncpy(buffer, line, str_len);
                    int cur = map_model_type(buffer);
                    trial_inst->models[cur_element] = map_model_type(buffer);
                    cur_element++;
                } else {
                    current_mode = 0;
                }
            }
            if (strncmp(line, "NAME", 4) == 0) {
                if (fgets(line, sizeof(line), trial_file) == NULL) {
                    valid_instance = 0;
                    printf("Name is not defined");
                    break;
                }
                str_len = strlen(line);
                trial_inst->name = calloc(str_len, 1);
                //remove the first space
                strncpy(trial_inst->name, line, str_len - 1);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                DEBUG_PRINT(("param_content: |%s|\n", trial_inst->name));
            }


            if (strncmp(line, "RUNS", 4) == 0) {
                if (fgets(line, sizeof(line), trial_file) == NULL) {
                    valid_instance = 0;
                    printf("Number of runs is not defined");
                    break;
                }
                trial_inst->n_runs = strtol(line, NULL, 10);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                DEBUG_PRINT(("param_content: |%d|\n", trial_inst->n_runs));
            }
            if (strncmp(line, "SEEDS", 5) == 0) {
                if (trial_inst->n_runs > 0) {
                    trial_inst->seeds = calloc(trial_inst->n_runs, sizeof(int));
                    cur_element = 0;
                    current_mode = 1;
                } else {
                    valid_instance = 0;
                    break;
                }
            }
            if (strncmp(line, "TIME_LIMIT", 10) == 0) {
                if (fgets(line, sizeof(line), trial_file) == NULL) {
                    valid_instance = 0;
                    printf("Time limit is not defined");
                    break;
                }
                trial_inst->time_limit = strtod(line, NULL);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                DEBUG_PRINT(("param_content: |%g|\n", trial_inst->time_limit));
            }
            if (strncmp(line, "NUM_INSTANCES", 13) == 0) {
                if (fgets(line, sizeof(line), trial_file) == NULL) {
                    valid_instance = 0;
                    printf("Number of instances is not defined");
                    break;
                }
                trial_inst->n_instances = strtol(line, NULL, 10);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                DEBUG_PRINT(("param_content: |%d|\n", trial_inst->n_instances));
            }
            if (strncmp(line, "INSTANCES", 9) == 0) {
                if (trial_inst->n_instances > 0) {
                    trial_inst->instances = calloc(trial_inst->n_instances, sizeof(char *));
                    cur_element = 0;
                    current_mode = 2;
                } else {
                    valid_instance = 0;
                    break;
                }
            }
            if (strncmp(line, "NUM_MODELS", 10) == 0) {
                if (fgets(line, sizeof(line), trial_file) == NULL) {
                    valid_instance = 0;
                    printf("Number of models is not defined");
                    break;
                }
                trial_inst->n_models = (int) strtol(line, NULL, 10);

                //There are still parameters to look at, if it was in coord input, change mode now
                current_mode = 0;
                DEBUG_PRINT(("param_content: |%d|\n", trial_inst->n_models));
            }
            if (strncmp(line, "MODELS", 6) == 0) {
                if (trial_inst->n_models > 0) {
                    trial_inst->models = calloc(trial_inst->n_models, sizeof(int));
                    cur_element = 0;
                    current_mode = 3;
                } else {
                    valid_instance = 0;
                    break;
                }
            }
        }
    }
    fclose(trial_file);

    return valid_instance;
}

void add_edge_to_solution(Solution_list *edges_list, int *edge) {
    if (edge == NULL) {
        printf("The edge is NULL, allocate it before passing it as argument. \n");
        exit(1);
    }
    if (edges_list->size == 0) {
        edges_list->solution = calloc(1, sizeof(int *));
        edges_list->size = 1;
    } else if (edges_list->size > 0) {
        edges_list->size += 1;
        edges_list->solution = realloc(edges_list->solution, edges_list->size * sizeof(int *));
    } else {
        printf("Error while adding edge to solution, the size is negative. \n");
        exit(1);
    }
    edges_list->solution[edges_list->size - 1] = edge;
}

void free_solution_list(Solution_list *edges_list) {
    if (edges_list->size < 0) {
        printf("Solution array was not initialized (sol size is negative).\n");
        return;
    }
    for (int i = 0; i < edges_list->size; i++) {
        free(edges_list->solution[i]);
    }
    free(edges_list->solution);

    //no free on edge_list because we assume that it was statically allocated
}

int plot_solution(Tsp_prob *instance, GRBmodel *model, GRBenv *env, int (*var_pos)(int, int, Tsp_prob *)) {
    int valid_plot = 1;
    //this function assumes that var_pos converts coordinates into the correct identifier in the GRB model
    Solution_list edges_list = {
            .size = 0
    };

    int index = -1;
    int error = 0;

    double sol;
    for (int i = 0; i < instance->nnode; i++) {
        for (int j = 0; j < instance->nnode; j++) {
            sol = 0;
            index = (*var_pos)(i, j, instance);
            if (index == -1) {
                //this is a tsp where self edges are not defined
                continue;
            }
            error = GRBgetdblattrelement(model, "X", index, &sol);
            quit_on_GRB_error(env, model, error);
            if (sol > 1 - MODEL_TOLERANCE) {
                int *edge = calloc(2, sizeof(int));
                edge[0] = i;
                edge[1] = j;
                add_edge_to_solution(&edges_list, edge);
            }
        }
    }

    plot_edges(&edges_list, instance);

    free_solution_list(&edges_list);

    return valid_plot;
}

