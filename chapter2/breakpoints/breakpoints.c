#include "breakpoints.h"

BREAKPOINT maxpoint(const BREAKPOINT* points, long npoints) {
    int i;
    BREAKPOINT point;

    point.time = points[0].time;
    point.value = points[0].value;

    for (i = 0; i < npoints; i++) {
        if (point.value < points[i].value) {
            point.value = points[i].value;
            point.time = points[i].time;
        }
    }

    return point;
}

BREAKPOINT* get_breakpoints(FILE* fp, long* psize) {
    int got;
    long npoints = 0, size = 64;
    double lasttime = 0.0;
    BREAKPOINT* points = NULL;
    char line[80];
    if (fp == NULL)
        return NULL;

    points = (BREAKPOINT*) malloc(sizeof(BREAKPOINT) * size);

    if (points == NULL)
        return NULL;

    while (fgets(line, 80, fp)) {
        got = sscanf(line, "%lf%lf",
            &points[npoints].time, &points[npoints].value);
        if (got < 0)
            continue;
        if (got == 0) {
            printf("Line %ld has non-numeric data\n", npoints+1);
            break;
        }
        if (got==1) {
            printf("Incomplete breakpoint found at point %ld\n",
                npoints+1);
            break;
        }
        if (points[npoints].time < lasttime) {
            printf("data error at point %ld: time not increasing\n",
                npoints+1);
            break;
        }
        lasttime = points[npoints].time;
        if (++npoints == size) {
            BREAKPOINT* tmp;
            size += npoints;
            tmp = (BREAKPOINT*) realloc(points, sizeof(BREAKPOINT) * size);
            if (tmp == NULL) {
                npoints = 0;
                free(points);
                points = NULL;
                break;
            }
            points = tmp;
        }
    }
    if (npoints)
        *psize = npoints;

    return points;
}