#pragma once

#include "types.h"

void loadFronts(Problem* O, const char* filename);
size_t parseLine(const char* line, Coord **coord);
void dumpProblem(const Problem* O, const char* filepath);
void dumpFront(const ParetoFront* f, const char* filepath);
void *allocateArray(size_t count, size_t size);

void appendCoord(ParetoFront *f, Coord pt);
void appendFront(Problem *source, size_t *capacity, ParetoFront front);
void appendSegment(Segments *segments, Segment segment);
void appendResult(Solution *result, size_t *capacity, Segment segment);

_Bool segmentsIntersect(Coord a1, Coord a2, Coord b1, Coord b2, Coord* intersection);
_Bool coincident(Coord a, Coord b);
Coord pointAt(Segment segment, double x);
int compareSegments(const void *a, const void *b);
size_t firstEventAfter(ParetoFront events, double x);
void intersectFronts(ParetoFront a, ParetoFront b, ParetoFront *events);

int xlt(const void *a, const void *b);