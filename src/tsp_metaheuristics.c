//
// Created by samuele on 02/07/19.
//


#include "meta_heuristic_utils.h"
#include "tsp_metaheuristics.h"
#include "tsp_vns.h"
#include "tsp_hardfixing.h"
#include "tsp_simulated_annealing.h"
#include "tsp_grasp.h"

void tsp_metaheuristics(Tsp_prob *instance) {
    int n_node = instance->nnode;
    //here choose which type of metaheuristic and refining
    int metaheuristic = -1; //0: vns, 1: sa, 2: grasp
    int refinement = -1; //0: hard fixing (with usercall), 1: local branching
    switch (instance->model_type) {
        case 17: {
            metaheuristic = 0;
            refinement = 0;
        }
            break;
        case 18: {
            metaheuristic = 1;
            refinement = 0;
        }
            break;
        case 19: {
            metaheuristic = 2;
            refinement = 0;
        }
            break;

        default:
            metaheuristic = 0;
            refinement = 0;
    }
    double time_limit_metaheuristic = instance->time_limit / 4;
    double time_limit_refinement = instance->time_limit - time_limit_metaheuristic;

    //run the metaheuristic
    instance->time_limit = time_limit_metaheuristic;

    if (metaheuristic == 0) {
        tsp_vns_create(instance);
    } else if (metaheuristic == 1) {
        tsp_simulated_annealing_create(instance);
    } else if (metaheuristic == 2) {
        tsp_grasp_create(instance);
    }
    int error = 0;
    error = GRBsetdblparam(GRBgetenv(instance->model), GRB_INT_PAR_OUTPUTFLAG, 0);
    if (error)
        printf("Error while disabling output: %d\n", error);

    instance->time_limit = time_limit_refinement;
    instance->black_box = 10;


    if (refinement == 0) {
        tsp_hardfixing_model_create_wsol(instance);
    }
}
