#include <stdio.h>
#include <string.h>

#include "main.h"
#include "util.h"

static Solution mergePair(Solution a, Solution b) {
    size_t count = a.numFronts + b.numFronts;
    if (count == 0) return (Solution){0};

    Problem pair = {allocateArray(count, sizeof(*pair.fronts)), count};
    if (a.numFronts)
        memcpy(pair.fronts, a.fronts, a.numFronts * sizeof(*pair.fronts));
    if (b.numFronts)
        memcpy(pair.fronts + a.numFronts, b.fronts, b.numFronts * sizeof(*pair.fronts));

    Solution merged = dplem(&pair);
    for (size_t i = 0; i < merged.numFronts; i++) {
        ParetoFront piece = merged.fronts[i];
        for (size_t j = 0; j < piece.size; j++) {
            Coord *point = &piece.points[j];
            point->front_index = pair.fronts[point->front_index].points[0].front_index;
        }
    }
    free(pair.fronts);

    return merged;
}

static void releaseNode(MergeNode node) {
    if (!node.owned) return;
    for (size_t i = 0; i < node.pieces.numFronts; i++)
        free(node.pieces.fronts[i].points);
    free(node.pieces.fronts);
}

Solution bdplem(const Problem *problem, int threads) {
    size_t m = problem->numFronts;
    if (m < 2) return dplem(problem);
    if (threads < 1) threads = 1;

    MergeNode *current = allocateArray(m, sizeof(*current));
    MergeNode *next = allocateArray(m / 2 + m % 2, sizeof(*next));
    for (size_t i = 0; i < m; i++)
        current[i].pieces = (Solution){&problem->fronts[i], 1};

    while (m > 1) {
        size_t pairs = m / 2;
        int workers = pairs < (size_t)threads ? (int)pairs : threads;
        #pragma omp parallel for num_threads(workers) if(workers > 1) schedule(dynamic)
        for (size_t j = 0; j < pairs; j++) {
            MergeNode a = current[2 * j];
            MergeNode b = current[2 * j + 1];
            next[j] = (MergeNode){mergePair(a.pieces, b.pieces), 1};
            releaseNode(a);
            releaseNode(b);
        }
        if (m % 2) next[pairs] = current[m - 1];
        m = pairs + m % 2;

        MergeNode *previous = current;
        current = next;
        next = previous;
    }

    Solution Pg = current[0].pieces;
    free(current);
    free(next);

    return Pg;
}
