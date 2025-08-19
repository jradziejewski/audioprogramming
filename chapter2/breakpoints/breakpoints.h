#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <stdio.h>
#include <stdlib.h>

typedef struct breakpoint {
    double time;
    double value;
} BREAKPOINT;


BREAKPOINT get_maxpoint(const BREAKPOINT* points, unsigned long npoints);
BREAKPOINT* get_breakpoints(FILE* fp, unsigned long* psize);
int inrange(const BREAKPOINT* points, double minval, double maxval, unsigned long npoints);
double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time);

#endif //BREAKPOINTS_H
