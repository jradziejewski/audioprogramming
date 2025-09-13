#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include <stdio.h>
#include <stdlib.h>

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

typedef struct breakpoint {
    double time;
    double value;
} BREAKPOINT;

typedef struct breakpoint_stream {
    BREAKPOINT* points;
    BREAKPOINT leftpoint, rightpoint;
    unsigned long npoints;
    double curpos;
    double incr;
    double width;
    double height;
    unsigned long ileft, iright;
    int more_points;
} BRKSTREAM;

BREAKPOINT get_maxpoint(const BREAKPOINT* points, unsigned long npoints);
BREAKPOINT* get_breakpoints(FILE* fp, unsigned long* psize);

int inrange(const BREAKPOINT* points, double minval, double maxval, unsigned long npoints);
double val_at_brktime(const BREAKPOINT* points, unsigned long npoints, double time);

BRKSTREAM* bps_newstream(FILE *fp, unsigned long srate, unsigned long *size);
void bps_freepoints(BRKSTREAM* stream);
double bps_tick(BRKSTREAM* stream);
int bps_getminmax(BRKSTREAM* stream, double *outmin, double *outmax);

#endif //BREAKPOINTS_H
