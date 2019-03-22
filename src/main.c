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
#include "utils.h"
#include "plot_graph.h"
#include "input_output.h"
#include "tsp_std.h"
#include "tsp_mtz.h"
#include "tsp_fischetti.h"
#include "tsp_flow1.h"
#include "tsp_flow2.h"
#include "tsp_flow3.h"
#include "tsp_timed_stage3.h"

int main(int argc, char **argv) {

    DEBUG_PRINT(("prova %s\n", "prova"));

    if (argc < 2) {
        printf("Not enough arguments.\n");
        exit(1);
    }

    Tsp_prob instance = {
            .nnode = -1,
            .model_type = 0
    };

    int valid_instance = 0;

    parse_input(argc, argv, &instance);

    valid_instance = init_instance(&instance);

    printf("weight type: %d\n", instance.weight_type);
    printf("nnodes: %d\n", instance.nnode);

   if (valid_instance) {

        plot_instance(&instance);

        switch(instance.model_type){
            case 0:
                tsp_model_create(&instance);
                break;
            case 1:
                mtz_model_create(&instance);
                break;
            case 2:
                fischetti_model_create(&instance);
                break;
            case 3:
                flow1_model_create(&instance);
                break;
            case 4:
                flow2_model_create(&instance);
                break;
            case 5:
                flow3_model_create(&instance);
            case 6:
                timed_stage3_model_create(&instance);
            default:
                tsp_model_create(&instance);
        }
        //tsp_model_create(&instance);
        //mtz_model_create(&instance);
        //fischetti_model_create(&instance);
        //flow1_model_create(&instance);
        //flow2_model_create(&instance);
        //flow3_model_create(&instance);
        //timed_stage3_model_create(&instance);

        //plot_edges(&instance);

    }else{

        printf("Error in parsing file");
    }
    close_instance(&instance);
    return 0;
}