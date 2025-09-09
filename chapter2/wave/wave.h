#ifndef WAVE_H
#define WAVE_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI (3.141592653589732)
#endif
#define TWOPI (2.0 * M_PI)

enum {WAVE_SINE, WAVE_TRIANGLE, WAVE_SQUARE, WAVE_SAWUP, WAVE_SAWDOWN, WAVE_NTYPES};

typedef struct t_oscil {
    double twopiovrsr; /* to hold a constant value 2PI/sr */
    double curfreq;
    double curphase;
    double incr;
} OSCIL;

typedef double (*tickfunc)(OSCIL* osc, double);

void oscil_init(OSCIL* osc, unsigned long srate);
OSCIL* oscil(void);
OSCIL* new_oscil(unsigned long srate);
double sinetick(OSCIL* p_osc, double freq);
double sqtick(OSCIL* p_osc, double freq);
double sawdtick(OSCIL* p_osc, double freq);
double sawutick(OSCIL* p_osc, double freq);
double tritick(OSCIL* p_osc, double freq);

#endif //WAVE_H
