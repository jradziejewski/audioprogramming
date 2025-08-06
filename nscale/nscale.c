#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/*
The goal of this program is to display the frequencies of equal-tepered intervals
for N notes per octave. If N is 12, we will get the notes of the standard chromatic scale.
If N is 24, we will get a quarter-tone scale.
Reasonable limit for N: 24

Starting on MIDI note provided by user

Usage:
nscale notes midinote
 */

int main(int argc, char* argv[]) {
    int notes, midinote;
    double frequency, ratio;
    double c0, c5;
    double intervals[24];

    /* Argument checks */

    if (argc != 3) {
        printf("usage: nscale notes midinote\n");
        return 1;
    }

    notes = atoi(argv[1]);
    if (notes < 1) {
        printf("Error: notes must be positive");
        return 1;
    }

    if (notes > 24) {
        printf("Error: maximum value for notes is 24");
    }


    midinote = atoi(argv[2]);

    if (midinote < 0) {
        printf("Error: cannot have negative MIDI notes");
    }

    if (midinote > 127) {
        printf("Error: maximum MIDI note is 127\n");
    }

    /* program here */

    ratio = pow(2.0,1.0 / notes);

    /* Find middle C - three semitones above low A = 220 */
    c5 = 220.0 * pow(ratio, 3);
    /* MIDI note 0 is C, 5 octaves below middle C */
    c0 = c5 * pow(0.5, 5);
    frequency = c0 * pow(ratio, midinote);

    // for (int i = 0; i < notes; i++) {
    //     intervals[i] = frequency;
    //     frequency *= ratio;
    // }
    int i = 0;
    while (i < notes) {
       intervals[i] = frequency;
        frequency *= ratio;
        i++;
    }

    for (int i = 0; i < notes; i++) {
        printf("%f\n", intervals[i]);
    }

    printf("Octave: %f", frequency);

    return 0;
}