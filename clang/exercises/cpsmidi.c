#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv[])
{
    double c5, c0, semitoneratio;
    double frequency;
    int midinote;

    semitoneratio = pow(2,1.0/12);
    c5 = 220.0 * pow(semitoneratio, 3);
    c0 = c5 * pow(0.5, 5);

    if (argc != 2) {
        printf("cpsmidi : converts MIDI note to frequency.\n");
        printf("usage: cpsmidi MIDInote\n" /* ,argv[0]*/);
        printf(" range: 0 <= MIDInote <= 127 \n");
        return 1;
    }

    midinote = atoi(argv[1]);

    if (midinote < 0) {
        printf("Bad MIDI note value: %s\n", argv[1]);
        return 1;
    }

    if (midinote > 127) {
        printf("Beyond MIDI range: %s\n", argv[1]);
        return 1;
    }

    frequency = c0 * pow(semitoneratio, midinote);
    printf("frequency of MIDI note %d = %f\n", midinote, frequency);
    return 0;
}

