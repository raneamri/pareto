#include <stdio.h>

#include "main.h"
#include "util.h"

Solution pdplem(const Problem *problem, int threads) {
    if (threads < 2 || problem->numFronts < 2) return dplem(problem);
    if ((size_t)threads > problem->numFronts) threads = (int)problem->numFronts;

    size_t partitions = (size_t)threads;
    size_t base = problem->numFronts / partitions;
    Solution *partial = calloc(partitions, sizeof(*partial));
    if (!partial) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    #pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < partitions; i++) {
        size_t start = i * base;
        size_t count = i + 1 == partitions ? problem->numFronts - start : base;
        Problem group = {.fronts = problem->fronts + start, .numFronts = count};
        partial[i] = dplem(&group);
    }

    Problem pieces = {0};
    size_t capacity = 0;
    for (size_t i = 0; i < partitions; i++) {
        for (size_t j = 0; j < partial[i].numFronts; j++) {
            ParetoFront piece = partial[i].fronts[j];
            for (size_t k = 0; k < piece.size; k++)
                piece.points[k].front_index += i * base;

            appendFront(&pieces, &capacity, piece);
        }
        free(partial[i].fronts);
    }
    free(partial);

    Solution Pg = dplem(&pieces);
    for (size_t i = 0; i < Pg.numFronts; i++) {
        ParetoFront piece = Pg.fronts[i];
        for (size_t j = 0; j < piece.size; j++) {
            Coord *point = &piece.points[j];
            point->front_index = pieces.fronts[point->front_index].points[0].front_index;
        }
    }

    for (size_t i = 0; i < pieces.numFronts; i++) free(pieces.fronts[i].points);
    free(pieces.fronts);

    return Pg;
}
