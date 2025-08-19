#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <stdio.h>
#include <stdlib.h>

typedef struct breakpoint {
    double time;
    double value;
} BREAKPOINT;


BREAKPOINT maxpoint(const BREAKPOINT* points, long npoints);
BREAKPOINT* get_breakpoints(FILE* fp, long* psize);

#endif //BREAKPOINTS_H
