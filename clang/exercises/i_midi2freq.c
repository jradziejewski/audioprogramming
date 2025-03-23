#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main()
{
    double c5, c0, semitone_ratio, frequency;
    int midinote;
    char message[4];

    semitone_ratio = pow(2, 1.0/12);
    c5 = 220.0 * pow(semitone_ratio, 3);
    c0 = c5 * pow(0.5, 5);

    printf("enter midi note (0 - 127): ");
    if(fgets(message, sizeof(message), stdin) == NULL) {
        printf("error reading the input\n");
        return 1;
    }

    if(message[0] == '\0') {
        printf("Have a nice day!\n");
        return 1;
    }

    midinote = atoi(message);
    if(midinote < 0 || midinote > 127) {
        printf("%s - bad MIDI number\n", message);
        return 1;
    }

    frequency = c0 * pow(semitone_ratio, midinote);
    printf("freq of MIDI note %d = %f\n", midinote, frequency);

    return 0;
}
