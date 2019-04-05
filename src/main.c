//
// Created by samuele on 05/03/19.
//

#include "common.h"
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "argtable3.h"
#include "utils.h"
#include "plot_graph.h"
#include "input_output.h"
#include "tsp_std.h"
#include "tsp_mtz.h"
#include "tsp_fischetti.h"
#include "tsp_flow1.h"
#include "tsp_flow2.h"
#include "tsp_flow3.h"
#include "tsp_timed_stage1.h"
#include "tsp_timed_stage2.h"
#include "tsp_timed_stage3.h"
#include "tsp_loop.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Not enough arguments. Type --help for list of arguments.\n");
        exit(1);
    }

    clock_t start, end;
    double cpu_time_used;

    Tsp_prob instance = {
            .nnode = -1,
            .model_type = 0
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
            start = clock();

            switch (instance.model_type) {
                case 0:
                    tsp_model_create(&instance); //std
                    break;
                case 1:
                    mtz_model_create(&instance); //mtz
                    break;
                case 2:
                    fischetti_model_create(&instance); //badcompact
                    break;
                case 3:
                    flow1_model_create(&instance); //flow1
                    break;
                case 4:
                    flow2_model_create(&instance); //flow2
                    break;
                case 5:
                    flow3_model_create(&instance); //flow3
                    break;
                case 6:
                    timed_stage1_model_create(&instance); //ts1
                    break;
                case 7:
                    timed_stage2_model_create(&instance); //ts2
                    break;
                case 8:
                    timed_stage3_model_create(&instance); //ts3
                    break;
                case 9:
                    tsp_loop_model_create(&instance); //loop additional SEC
                    break;
                default:
                    tsp_model_create(&instance);
            }

            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            DEBUG_PRINT(("Time taken in seconds: %g", cpu_time_used));

        } else {
            printf("Error in parsing file");
        }
        close_instance(&instance);
    } else if (type == 1) {
        init_trial(&trial_inst);
        printf("Starting Trial name: %s\n", trial_inst.name);
        int seed;
        char instance[120];
        int model;
        char filename[120];
        char model_name[120];
        strcat(filename, "trials/");
        strcat(filename, trial_inst.name);
        strcat(filename, ".output");
        FILE* trial = fopen(filename, "w");
        fprintf(trial, "INSTANCE,MODEL FORMULATION,SEED,TIME ELAPSED (in seconds)\n");

        trial_inst.problems = calloc(trial_inst.n_instances, sizeof(Tsp_prob*));
        for (int i = 0; i < trial_inst.n_instances; i++) {
            trial_inst.problems[i] = calloc(1, sizeof(Tsp_prob));
            trial_inst.problems[i]->filename = trial_inst.instances[i];
            init_instance(trial_inst.problems[i]);
        }
        Tsp_prob * instance_pointer;
        for(int cur_instance = 0; cur_instance < trial_inst.n_instances; cur_instance++) {
            instance_pointer = trial_inst.problems[cur_instance];
            for (int cur_run = 0; cur_run < trial_inst.n_runs; cur_run++) {
                for (int cur_model = 0; cur_model < trial_inst.n_models; cur_model++) {
                    start = clock();
                    printf("mode: %d\n", instance_pointer->model_type);
                    instance_pointer->model_type = trial_inst.models[cur_model];

                    DEBUG_PRINT(("Instance: %s | Model: %d | Seed: %d\n", trial_inst.instances[cur_instance], trial_inst.models[cur_model], trial_inst.seeds[cur_run]));

                    switch (instance_pointer->model_type) {
                        case 0:
                            tsp_model_create(instance_pointer); //std
                            break;
                        case 1:
                            mtz_model_create(instance_pointer); //mtz
                            break;
                        case 2:
                            fischetti_model_create(instance_pointer); //badcompact
                            break;
                        case 3:
                            flow1_model_create(instance_pointer); //flow1
                            break;
                        case 4:
                            flow2_model_create(instance_pointer); //flow2
                            break;
                        case 5:
                            flow3_model_create(instance_pointer); //flow3
                            break;
                        case 6:
                            timed_stage1_model_create(instance_pointer); //ts1
                            break;
                        case 7:
                            timed_stage2_model_create(instance_pointer); //ts2
                            break;
                        case 8:
                            timed_stage3_model_create(instance_pointer); //ts3
                            break;
                        case 9:
                            tsp_loop_model_create(instance_pointer); //loop additional SEC
                            break;
                        default:
                            tsp_model_create(instance_pointer);
                    }
                    end = clock();
                    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
                    inverse_map_model_type(trial_inst.models[cur_model], model_name);
                    fprintf(trial, "%s,%s,%d,%g\n", trial_inst.instances[cur_instance], model_name, trial_inst.seeds[cur_run], cpu_time_used);
                }
            }
        }
        fclose(trial);
        //TODO CLOSE TRIAL INSTANCE

    } else {
        printf("Type not recognized, exiting.\n");
        exit(1);
    }
    return 0;
}