// Online C compiler to run C program online
#include <stdio.h>
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

void oscil_init(OSCIL* osc, unsigned long srate) {
    osc->twopiovrsr = TWOPI / (double) srate;
    osc->curfreq = 0.0;
    osc->curphase = 0.0;
    osc->incr = 0.0;
}

OSCIL* oscil(void) {
    OSCIL* osc = (OSCIL*) malloc(sizeof(OSCIL));
    if (osc == NULL)
        return NULL;
    return osc;
}

int main() {
    double startphase = 0.0;
    double srate = 44100.0;
    double freq = 440.0;
    double curphase = 0.0;
    int outsamps = 50;
    double output[50];
    double incr = freq * TWOPI / srate;

    for(int i = 0; i < outsamps; i++) {
        output[i] = (float) sin(curphase);
        curphase += incr;
        if(curphase >= TWOPI)
            curphase -= TWOPI;

        printf("%f\n", output[i]);
    }

    return 0;
}