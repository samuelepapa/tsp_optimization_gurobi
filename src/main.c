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
#include "tsp.h"
#include "plot_graph.h"
#include "input_output.h"
#include "tsp_MTZ.h"

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

    printf("weight type: %d\n", instance.weight_type);
    printf("nnodes: %d\n", instance.nnode);

    if (valid_instance) {

        plot_instance(&instance);

        model_create(&instance);
        //MTZ_model_create(&instance);

        //plot_edges(&instance);

        close_instance(&instance);

    }else{
        printf("Error in parsing file");
        exit(1);
    }
    return 0;
}