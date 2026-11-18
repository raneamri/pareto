#pragma once

#include <stdlib.h>

// quickly toggle off logs
#define VERBOSE 1
#define LOG(x) \
    do { if (VERBOSE) printf x; } while (0)

#define CAPACITY_INCREMENT 8

#define MAX_X INT_MAX
#define MAX_Y INT_MAX

#define size_estimate(n, m) \
    (((sizeof(Coord) * n) + (sizeof(ParetoFront) * m)) * 1e-9)

enum Algorithm {
    DPLEM, BDPLEM, PDPLEM
};

typedef enum Algorithm Algorithm;

struct Coord {
    double x;
    double y;

    int type;
    size_t front_index;
};

typedef struct Coord Coord;

struct ParetoFront {
    struct Coord* points;
    size_t size;

    size_t capacity; // memory management
};

typedef struct ParetoFront ParetoFront;

typedef struct {
    Coord left;
    Coord right;
    size_t id;
} Segment;

typedef struct {
    Segment *items;
    size_t size;
    size_t capacity;
} Segments;

struct Problem {
    ParetoFront* fronts;
    size_t numFronts;
};

typedef struct Problem Problem;
typedef struct Problem Solution; // for readability

typedef struct {
    Solution pieces;
    _Bool owned;
} MergeNode;
