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
#include "Utils.h"
#include "Tsp.h"
#include "Plot_graph.h"
#include "Input_output.h"
#include "Tsp_MTZ.h"

int main(int argc, char **argv) {

    DEBUG_PRINT(("prova %s\n", "prova"));

    if (argc < 2) {
        printf("Not enough arguments.\n");
        exit(1);
    }

    Tsp_prob instance = {
            .nnode = -1
    };

    int valid_instance = 0;

    parse_input(argc, argv, &instance);

    valid_instance = init_instance(&instance);

    printf("weight type: %d", instance.weight_type);

    if (valid_instance) {

        plot_instance(&instance);

        //preprocessing_model_create(&instance);
        preprocessing_MTZ_model_create(&instance);

        //plot_solution(&instance);

        close_instance(&instance);

    }else{
        printf("Error in parsing file");
        exit(1);
    }
    return 0;
}