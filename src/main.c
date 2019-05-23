//
// Created by samuele on 05/03/19.
//

#include "common.h"
#include "utils.h"
#include "plot_graph.h"
#include "input_output.h"
#include "tsp_std.h"
#include "tsp_mtz.h"
#include "tsp_bad_compact.h"
#include "tsp_flow1.h"
#include "tsp_flow2.h"
#include "tsp_flow3.h"
#include "tsp_timed_stage1.h"
#include "tsp_timed_stage2.h"
#include "tsp_timed_stage3.h"
#include "tsp_loop.h"
#include "tsp_lazycall.h"
#include "tsp_usercall.h"
#include "tsp_hardfixing.h"
#include "tsp_local_branching.h"
#include "tsp_vns.h"
#include "tsp_grasp.h"

#include "peaceful_queens_optimization.h"
/**
 * Selects the method chosen by the instance and starts it
 * @param instance the tsp instance with the information about the problem to solve
 */
void start_selected_model(Tsp_prob *instance);

void execute_trial(Trial *trial_inst);

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Not enough arguments. Type --help for list of arguments.\n");
        exit(1);
    }

    struct timespec start, end;

    Tsp_prob instance = {
            .env = NULL,
            .nnode = -1,
            .model_type = 0,
            .seed = 0,
            .best_solution = -1,
            .time_limit = INFINITY, //for tsp_loop purposes
            .warm_start = 0,
            .prob = 0.9
    };
    Trial trial_inst = {
            .n_models = -1,
            .n_instances = -1,
            .n_runs = -1
    };
    int valid_instance = 0;

    //0 = single run; 1 = multiple runs (a trial)
    int type = -1;

    type = parse_input(argc, argv, &instance, &trial_inst);

    if (type == 0) {
        valid_instance = init_instance(&instance);

        DEBUG_PRINT(("weight type: %d\n", instance.weight_type));
        DEBUG_PRINT(("nnodes: %d\n", instance.nnode));

        if (valid_instance) {

            plot_instance(&instance);

            //Start the timer
            clock_gettime(CLOCK_MONOTONIC, &start);

            //create file name
            char *model_name = calloc(64, sizeof(char));
            inverse_map_model_type(instance.model_type, model_name);
            strcat(model_name, ".log");
            //create gurobi environment
            GRBloadenv(&instance.env, model_name);

            //set the seed
            int error;
            error = GRBsetintparam(instance.env, "Seed", instance.seed);
            quit_on_GRB_error(instance.env, instance.model, error);
            error = GRBsetdblparam(instance.env, "TimeLimit", instance.time_limit);
            quit_on_GRB_error(instance.env, instance.model, error);

            start_selected_model(&instance);

            clock_gettime(CLOCK_MONOTONIC, &end);
            printf("STAT, %s,%s,%d,%g,%g\n\n", instance.filename, model_name, instance.seed, instance.best_solution,
                   end.tv_sec - start.tv_sec + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0);

            free(model_name);
            GRBfreeenv(instance.env);

        } else {
            printf("Error in parsing file");
        }
        close_instance(&instance);
    } else if (type == 1) {
        init_trial(&trial_inst);

        execute_trial(&trial_inst);

        close_trial(&trial_inst);
    } else if (type == 2) {
        init_trial(&trial_inst);


    } else {
        printf("Type not recognized, exiting.\n");
        exit(1);
    }
    return 0;
}

void start_selected_model(Tsp_prob *instance) {
    switch (instance->model_type) {
        case 0:
            tsp_model_create(instance); //std
            break;
        case 1:
            mtz_model_create(instance); //mtz
            break;
        case 2:
            bad_compact_model_create(instance); //badcompact
            break;
        case 3:
            flow1_model_create(instance); //flow1
            break;
        case 4:
            flow2_model_create(instance); //flow2
            break;
        case 5:
            flow3_model_create(instance); //flow3
            break;
        case 6:
            timed_stage1_model_create(instance); //ts1
            break;
        case 7:
            timed_stage2_model_create(instance); //ts2
            break;
        case 8:
            timed_stage3_model_create(instance); //ts3
            break;
        case 9:
            tsp_loop_model_create(instance); //loop additional SEC
            break;
        case 10:
            tsp_lazycall_model_create(instance); //lazy callback SEC
            break;
        case 11:
            tsp_hardfixing_model_create(instance); //hardfixing
            break;
        case 12:
            tsp_usercall_model_create(instance); //usercall
            break;
        case 13:
            tsp_local_branching_create(instance); //localbranching
            break;
        case 14:
            tsp_vns_create(instance); //variable neighborhood search
            break;
        case 15:
            tsp_grasp_create(instance); //greedy randomized adaptive search procedure
            break;
        default:
            tsp_model_create(instance);
    }
}

void execute_trial(Trial *trial_inst) {
    struct timespec start, end;
    double time_elapsed;
    //create environment
    GRBenv *env = NULL;
    int error = GRBloadenv(&env, NULL);
    if (error || env == NULL) {
        printf("Unable to initialize environment in trial_file. error: %d \n", error);
        exit(1);
    }
    //set timelimit
    error = GRBsetdblparam(env, "TimeLimit", trial_inst->time_limit);
    if (error) {
        printf("Error while setting the TimeLimit on trial_file: %d\n", error);
        exit(1);
    }

    char model_name[120];
    char *output_log = calloc(128000, sizeof(char));
    char *pointer_to_output_log = output_log;
    sprintf(pointer_to_output_log, "INSTANCE,MODEL FORMULATION,SEED,TIME ELAPSED (in seconds)\n");
    pointer_to_output_log = output_log + strlen(output_log);

    //Initialize problem instances
    trial_inst->problems = calloc(trial_inst->n_instances, sizeof(Tsp_prob *));
    for (int i = 0; i < trial_inst->n_instances; i++) {
        trial_inst->problems[i] = calloc(1, sizeof(Tsp_prob));
        trial_inst->problems[i]->filename = trial_inst->instances[i];
        trial_inst->problems[i]->env = env;
        trial_inst->problems[i]->time_limit = trial_inst->time_limit;
        printf("file: %s\n", trial_inst->problems[i]->filename);
        init_instance(trial_inst->problems[i]);
    }
    //Starting trial of the blade
    Tsp_prob *instance_pointer;
    for (int cur_instance = 0; cur_instance < trial_inst->n_instances; cur_instance++) {
        instance_pointer = trial_inst->problems[cur_instance];
        for (int cur_run = 0; cur_run < trial_inst->n_runs; cur_run++) {
            error = GRBsetintparam(instance_pointer->env, "Seed", trial_inst->seeds[cur_run]);
            if (error) {
                printf("Something went wrong while setting the seed of the env in the trial run %d\n", error);
            }
            for (int cur_model = 0; cur_model < trial_inst->n_models; cur_model++) {
                //start clock
                clock_gettime(CLOCK_MONOTONIC, &start);
                instance_pointer->model_type = trial_inst->models[cur_model];
                //output purposes
                printf("Instance: %s | Model: %d | Seed: %d\n", trial_inst->instances[cur_instance],
                       trial_inst->models[cur_model], trial_inst->seeds[cur_run]);

                start_selected_model(instance_pointer);

                //calculate time elapsed
                clock_gettime(CLOCK_MONOTONIC, &end);
                time_elapsed = end.tv_sec - start.tv_sec + (double) (end.tv_nsec - start.tv_sec) / 1000000000.0;
                if (instance_pointer->status == GRB_TIME_LIMIT) {
                    time_elapsed = -1;
                }
                //Output purposes
                inverse_map_model_type(trial_inst->models[cur_model], model_name);
                //fprintf(trial_file, "INSTANCE,MODEL FORMULATION,SEED,TIME ELAPSED (in seconds)\n");
                sprintf(pointer_to_output_log, "%s,%s,%d,%g\n", trial_inst->instances[cur_instance], model_name,
                        trial_inst->seeds[cur_run], time_elapsed);
                pointer_to_output_log = output_log + strlen(output_log);
                fprintf(stdout, "STAT, %s,%s,%d,%g\n\n", trial_inst->instances[cur_instance], model_name,
                        trial_inst->seeds[cur_run], time_elapsed);
            }
        }
    }

    printf("OUTPUT LOG: \n%s", output_log);

    GRBfreeenv(env);
    free(output_log);
}