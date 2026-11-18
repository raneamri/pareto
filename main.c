#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#ifdef __APPLE__
#include "/opt/homebrew/opt/libomp/include/omp.h"
#else
#include <omp.h>
#endif

#include "main.h"
#include "util.h"

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <input_file> <output_file> <algorithm> <threads>", argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    const char *arg_algorithm = argv[3];
    int arg_threads = atoi(argv[4]);

    Problem *O = calloc(1, sizeof(*O));
    if (!O) {
        perror("calloc");
        return 1;
    } 
    loadFronts(O, input_file);

    if (arg_threads <= 0) arg_threads = 1;
    if (arg_threads > omp_get_num_procs()) arg_threads = omp_get_num_procs();

    Algorithm A;
    if (strcmp(arg_algorithm, "DPLEM") == 0) {
        A = DPLEM;
    } else if (strcmp(arg_algorithm, "BDPLEM") == 0) {
        A = BDPLEM;
    } else if (strcmp(arg_algorithm, "PDPLEM") == 0) {
        A = PDPLEM;
    } else {
        LOG(("Unknown algorithm %s", arg_algorithm));
        return -1;
    }

    if (VERBOSE) {
        int points = 0;
        for (size_t i = 0; i < O->numFronts; i++) points += O->fronts[i].size;

        LOG(("Fronts loaded: %zu\n", O->numFronts));
        LOG(("Points in problem: %d\n", points));
        LOG(("Minimum memory usage: %.6f GB\n", size_estimate(points, O->numFronts) ));
        LOG(("Using algorithm: %s\n", arg_algorithm));
        LOG(("Threads: %.0f\n", fmin(arg_threads, omp_get_num_procs())));
        LOG(("Starting...\n\n"));
    }

    Solution Pg;
    
    clock_t cpu_start_time, cpu_end_time;
    double wall_start_time, wall_end_time;
    double cpu_elapsed, wall_elapsed;

    wall_start_time = omp_get_wtime(); cpu_start_time = clock();

    switch(A) {
        default:
        case DPLEM:
            Pg = dplem(O);
            break;
        case PDPLEM:
            Pg = pdplem(O, arg_threads);
            break;
        case BDPLEM:
            Pg = bdplem(O, arg_threads);
            break;
    }

    wall_end_time = omp_get_wtime(); cpu_end_time = clock();
    wall_elapsed = (wall_end_time - wall_start_time);
    cpu_elapsed = (double)(cpu_end_time - cpu_start_time) / CLOCKS_PER_SEC;

    printf("CPU time: %fs\n", cpu_elapsed);
    printf("Wall time: %fs\n", wall_elapsed);

    dumpProblem(&Pg, output_file);

    for (size_t i = 0; i < Pg.numFronts; i++) free(Pg.fronts[i].points);
    free(Pg.fronts);
    for (size_t i = 0; i < O->numFronts; i++) free(O->fronts[i].points);
    free(O->fronts);
    free(O);

    return 0;
}
