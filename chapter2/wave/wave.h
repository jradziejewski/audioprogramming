#ifndef WAVE_H
#define WAVE_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.141592653589732)
#endif
#define TWOPI (2.0 * M_PI)

typedef struct t_oscil {
    double twopiovrsr; /* to hold a constant value 2PI/sr */
    double curfreq;
    double curphase;
    double incr;
} OSCIL;


void oscil_init(OSCIL* osc, unsigned long srate);
OSCIL* oscil(void);
double sinetick(OSCIL* p_osc, double freq);

#endif //WAVE_H
