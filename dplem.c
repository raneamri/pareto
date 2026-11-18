#include <stdio.h>

#include "main.h"
#include "util.h"

static ParetoFront createIntervals(const Problem *problem) {
    ParetoFront X = {0};
    for (size_t i = 0; i < problem->numFronts; i++) {
        ParetoFront front = problem->fronts[i];
        for (size_t j = 0; j < front.size; j++)
            appendCoord(&X, front.points[j]);
        for (size_t j = i + 1; j < problem->numFronts; j++)
            intersectFronts(front, problem->fronts[j], &X);
    }

    if (X.size > 1) qsort(X.points, X.size, sizeof(Coord), xlt);
    size_t unique = 0;
    for (size_t i = 0; i < X.size; i++) {
        if (unique == 0 || X.points[i].x != X.points[unique - 1].x)
            X.points[unique++] = X.points[i];
    }
    X.size = unique;
    return X;
}

static Segments refineFronts(const Problem *problem, ParetoFront events) {
    Segments S = {0};
    for (size_t i = 0; i < problem->numFronts; i++) {
        ParetoFront front = problem->fronts[i];
        if (front.size == 0) continue;

        Coord left = front.points[0];
        left.front_index = i;
        if (front.size == 1)
            appendSegment(&S, (Segment){left, left, i});

        size_t event = firstEventAfter(events, left.x);
        for (size_t j = 1; j < front.size; j++) {
            Coord right = front.points[j];
            right.front_index = i;
            Segment original = {left, right, i};
            while (event < events.size && events.points[event].x < right.x) {
                Coord split = pointAt(original, events.points[event++].x);
                appendSegment(&S, (Segment){left, split, i});
                left = split;
            }
            appendSegment(&S, (Segment){left, right, i});
            left = right;
            if (event < events.size && events.points[event].x == right.x) event++;
        }
    }
    return S;
}

Solution dplem(const Problem *problem) {
    ParetoFront X = createIntervals(problem);
    Segments S = refineFronts(problem, X);
    free(X.points);

    Solution Pg = {0};
    if (S.size == 0) return Pg;
    qsort(S.items, S.size, sizeof(Segment), compareSegments);

    size_t capacity = 0;
    double ymin = S.items[0].left.y;
    _Bool started = 0;

    for (size_t i = 0; i < S.size; ) {
        size_t end = i + 1;
        double x = S.items[i].left.x;
        while (end < S.size && S.items[end].left.x == x) end++;

        Segment first = S.items[i];
        _Bool hasPoint = first.right.x == x;
        size_t interval = i;

        while (interval < end && S.items[interval].right.x == x) interval++;

        if (interval < end && S.items[interval].left.y <= first.left.y) {
            first = S.items[interval];
            hasPoint = 0;
        }
        if (!started || (hasPoint && first.left.y < ymin)) {
            ymin = first.left.y;
            appendResult(&Pg, &capacity, (Segment){first.left, first.left, first.id});
            started = 1;
        }

        if (interval < end) {
            Segment segment = S.items[interval];
            if (segment.right.y < ymin) {
                if (segment.left.y > ymin) {
                    long double t = ((long double)segment.left.y - ymin) /
                                    ((long double)segment.left.y - segment.right.y);
                    segment.left.x += t * ((long double)segment.right.x - segment.left.x);
                    segment.left.y = ymin;
                }
                appendResult(&Pg, &capacity, segment);
                ymin = segment.right.y;
            }
        }
        i = end;
    }

    free(S.items);

    return Pg;
}
