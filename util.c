#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util.h"

void loadFronts(Problem *O, const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    size_t numLines = 0;
    char line[BUFSIZ*2];
    while (fgets(line, BUFSIZ*2, file)) {
        if (strspn(line, " \t\r\n") == strlen(line)) continue;
        numLines++;
    }

    rewind(file);

    O->fronts = malloc(numLines * sizeof(ParetoFront));
    if (!O->fronts) {
        perror("malloc");
        fclose(file);
        return;
    }

    size_t c = 0;
    rewind(file);
    while (fgets(line, BUFSIZ*2, file)) {
        if (strspn(line, " \t\r\n") == strlen(line)) continue;
        O->fronts[c].size = parseLine(line, &O->fronts[c].points);
        O->fronts[c].capacity = O->fronts[c].size;
        for (size_t j = 0; j < O->fronts[c].size; j++) {
            O->fronts[c].points[j].front_index = c;
        }

        c++;
    }

    O->numFronts = c;

    fclose(file);
}

size_t parseLine(const char* line, Coord **coords)
{
    size_t count = 0;
    const char* p = line;

    while ((p = strchr(p, '(')) != NULL) {
        count++;
        p++;
    }

    *coords = malloc(count * sizeof(struct Coord));

    p = line;
    size_t parsed = 0;
    while (parsed < count) {
        double x, y;
        int matched = 0;

        // allow some tolerance in file reading in case we change the format later/use David's data
        if (sscanf(p, " ( %lf , %lf )", &x, &y) == 2) {
            matched = 1;
        } else if (sscanf(p, "(%lf , %lf)", &x, &y) == 2) {
            matched = 1;
        } else if (sscanf(p, "%lf,%lf,", &x, &y) == 2) {
            matched = 1;
        } else if (sscanf(p, "(%lf,%lf)", &x, &y) == 2) {
            matched = 1;
        } else if (sscanf(p, ",(%lf, %lf)", &x, &y) == 2) {
            matched = 1;
        } else if (sscanf(p, "(%lf, %lf)", &x, &y) == 2) {
            matched = 1;
        } else if (sscanf(p, "(%lf, %lf),", &x, &y) == 2) {
            matched = 1;
        } else {
            LOG(("Failed to parse coordinate at: %s\n", p));
        }

        if (matched) {
            (*coords)[parsed].x = x;
            (*coords)[parsed].y = y;
            (*coords)[parsed].type = 0;
            (*coords)[parsed].front_index = (size_t)-1;
            parsed++;

            const char* end = strchr(p, ')');
            if (!end) break;
            p = end + 1;
        } else {
            const char* next = strchr(p + 1, '(');
            if (!next) break;
            p = next;
        }
    }

    //LOG(("Parsed %zu coordinates: %s\n", parsed, line));

    return parsed;
}

void dumpProblem(const Problem* O, const char* filepath) {
    FILE* file = fopen(filepath, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return;
    }

    for (size_t i = 0; i < O->numFronts; i++) {
        for (size_t j = 0; j < O->fronts[i].size; j++) {
            fprintf(file, "(%lf, %lf)", O->fronts[i].points[j].x, O->fronts[i].points[j].y);
            if (j < O->fronts[i].size - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);

    LOG(("Fronts dumped to %s\n", filepath));

    return;
}

void dumpFront(const ParetoFront* f, const char* filepath) {
    FILE* file = fopen(filepath, "w");
    if (!f) {
        perror("Failed to open file for writing");
        return;
    }

    for (size_t j = 0; j < f->size; j++) {
        fprintf(file, "(%lf, %lf)", f->points[j].x, f->points[j].y);
        if (j < f->size - 1) {
            fprintf(file, ",");
        }
    }
    fprintf(file, "\n");

    fclose(file);

    LOG(("Front dumped to %s\n", filepath));

    return;
}

void appendCoord(ParetoFront *f, Coord pt) {
    if (f->size == f->capacity) {
        size_t new_capacity = (f->capacity == 0) ? CAPACITY_INCREMENT : (f->capacity * 2);
        Coord *tmp = realloc(f->points, new_capacity * sizeof(Coord));
        if (!tmp) {
            perror("realloc");
            exit(1);
        }
        f->points = tmp;
        f->capacity = new_capacity;
    }
    f->points[f->size++] = pt;

    return;
}

void appendFront(Problem *source, size_t *capacity, ParetoFront front) {
    if (source->numFronts == *capacity) {
        *capacity = (*capacity == 0) ? CAPACITY_INCREMENT : (*capacity * 2);
        ParetoFront *tmp = realloc(source->fronts, (*capacity) * sizeof(ParetoFront));
        if (!tmp) {
            perror("realloc");
            exit(1);
        }
        source->fronts = tmp;
    }
    source->fronts[source->numFronts++] = front;

    return;
}

_Bool segmentsIntersect(Coord a1, Coord a2, Coord b1, Coord b2, Coord* intersection) {
    long double denom = (a2.x - a1.x) * (b2.y - b1.y) - (a2.y - a1.y) * (b2.x - b1.x);
    if (denom == 0) return 0;

    long double ua = ((b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x)) / denom;
    long double ub = ((a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x)) / denom;

    if (ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1) {
        intersection->x = a1.x + ua * (a2.x - a1.x);
        intersection->y = a1.y + ua * (a2.y - a1.y);
        intersection->type = 1; // mark as intersection
        intersection->front_index = (size_t)-1;
        return 1;
    }

    return 0;
}

inline _Bool coincident(Coord a, Coord b) {
    return (a.x - b.x) == 0 && (a.y - b.y) == 0;
}

int xlt(const void *a, const void *b)
{
    const Coord *ca = a;
    const Coord *cb = b;

    if (ca->x < cb->x) return -1;
    if (ca->x > cb->x) return 1;
    return 0;
}

Coord pointAt(Segment segment, double x) {
    long double t = ((long double)x - segment.left.x) /
                    ((long double)segment.right.x - segment.left.x);
    return (Coord){
        .x = x,
        .y = segment.left.y + t * ((long double)segment.right.y - segment.left.y),
        .front_index = segment.id
    };
}

int compareSegments(const void *a, const void *b) {
    const Segment *s = a;
    const Segment *t = b;
    int xOrder = xlt(&s->left, &t->left);
    if (xOrder) return xOrder;

    _Bool sPoint = s->left.x == s->right.x;
    _Bool tPoint = t->left.x == t->right.x;
    if (sPoint != tPoint) return sPoint ? -1 : 1;

    long double sMid = 0.5L * s->left.y + 0.5L * s->right.y;
    long double tMid = 0.5L * t->left.y + 0.5L * t->right.y;
    if (sMid != tMid) return sMid < tMid ? -1 : 1;
    return (s->id > t->id) - (s->id < t->id);
}

void appendResult(Solution *result, size_t *capacity, Segment segment) {
    if (result->numFronts) {
        ParetoFront *last = &result->fronts[result->numFronts - 1];
        Coord end = last->points[last->size - 1];
        if (coincident(end, segment.left)) {
            if (last->size == 1 || end.front_index == segment.id) {
                if (!coincident(end, segment.right)) appendCoord(last, segment.right);
                return;
            }
        }
    }

    ParetoFront piece = {0};
    appendCoord(&piece, segment.left);
    if (!coincident(segment.left, segment.right)) appendCoord(&piece, segment.right);
    appendFront(result, capacity, piece);
}

size_t firstEventAfter(ParetoFront events, double x) {
    size_t low = 0;
    size_t high = events.size;
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (events.points[mid].x <= x) low = mid + 1;
        else high = mid;
    }
    return low;
}

void appendSegment(Segments *segments, Segment segment) {
    if (segments->size == segments->capacity) {
        if (segments->capacity > SIZE_MAX / sizeof(Segment) / 2) {
            fprintf(stderr, "Too many refined segments\n");
            exit(EXIT_FAILURE);
        }
        size_t capacity = segments->capacity ? 2 * segments->capacity : 8;
        Segment *items = realloc(segments->items, capacity * sizeof(*items));
        if (!items) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        segments->items = items;
        segments->capacity = capacity;
    }
    segments->items[segments->size++] = segment;
}

void intersectFronts(ParetoFront a, ParetoFront b, ParetoFront *events) {
    size_t i = 0;
    size_t j = 0;
    while (i + 1 < a.size && j + 1 < b.size) {
        Coord ar = a.points[i + 1];
        Coord br = b.points[j + 1];
        Coord intersection;
        if (a.points[i].x < br.x && b.points[j].x < ar.x &&
            segmentsIntersect(a.points[i], ar, b.points[j], br, &intersection)) {
            appendCoord(events, intersection);
        }
        if (ar.x <= br.x) i++;
        if (br.x <= ar.x) j++;
    }
}

void *allocateArray(size_t count, size_t size) {
    void *array = calloc(count, size);
    if (!array) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    return array;
}

static double interpolateY(Coord p1, Coord p2, double x) {
    return p1.y + (p2.y - p1.y) *
           (x - p1.x) / (p2.x - p1.x);
}

static int sign(double x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

int countIntersections(const ParetoFront *A, const ParetoFront *B) {
    if (A->size < 2 || B->size < 2)
        return 0;

    size_t a = 0;
    size_t b = 0;

    int intersections = 0;

    double left = fmax(A->points[0].x, B->points[0].x);
    double right = fmin(A->points[A->size - 1].x,
                        B->points[B->size - 1].x);

    if (left > right)
        return 0;

    while (a + 1 < A->size - 1 &&
           A->points[a + 1].x <= left)
        a++;

    while (b + 1 < B->size - 1 &&
           B->points[b + 1].x <= left)
        b++;

    int previousSign = 0;
    int onZeroRun = 0;

    while (a + 1 < A->size && b + 1 < B->size) {

        Coord a1 = A->points[a];
        Coord a2 = A->points[a + 1];

        Coord b1 = B->points[b];
        Coord b2 = B->points[b + 1];

        double xEnd = fmin(a2.x, b2.x);
        if (xEnd > right)
            xEnd = right;

        if (xEnd < left)
            break;

        double dLeft =
            interpolateY(a1, a2, left) -
            interpolateY(b1, b2, left);

        double dRight =
            interpolateY(a1, a2, xEnd) -
            interpolateY(b1, b2, xEnd);

        int sLeft = sign(dLeft);
        int sRight = sign(dRight);

        if (sLeft != 0 && sRight != 0 &&
            sLeft != sRight) {
            intersections++;
        }

        if (sLeft == 0 && !onZeroRun) {
            intersections++;
            onZeroRun = 1;
        }

        if (sRight != 0)
            onZeroRun = 0;

        previousSign = sRight;

        if (xEnd >= right)
            break;

        if (a2.x <= xEnd)
            a++;

        if (b2.x <= xEnd)
            b++;

        left = xEnd;
    }

    if (previousSign == 0 && !onZeroRun)
        intersections++;

    return intersections;
}

void intersectionProfile(Problem *O) {
    size_t n = O->numFronts;

    size_t m = O->fronts[0].size - 1;
    size_t numBins = 2 * m - 1;

    size_t bins[numBins + 1];

    for (size_t k = 0; k < numBins; k++)
        bins[k] = 0;

    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {

            int intersections =
                countIntersections(&O->fronts[i],
                                   &O->fronts[j]);

            if (intersections >= 0 &&
                (size_t)intersections <= numBins) {

                bins[intersections]++;

            } else {
                LOG(("WARNING: %d intersections between fronts %zu and %zu\n",
                     intersections, i, j));
            }
        }
    }

    LOG(("Intersection profile:\n"));

    LOG(("["));

    for (size_t k = 0; k < numBins; k++) {
        if (k == numBins-1) LOG(("%zu", bins[k]));
        else LOG(("%zu, ", bins[k]));

    }

    LOG(("]\n"));
}