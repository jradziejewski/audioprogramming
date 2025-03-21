#include <stdio.h>
#include <math.h>

int main()
{
    double semitone_ratio = pow(2, 1/12.0), c5 = 220.0 * pow(semitone_ratio, 3), c0 = c5 * pow(0.5, 5);
    double frequency;
    int    midinote;

    /* calculate required numbers */
    semitone_ratio = pow(2, 1/12.0); /* approx. 1.0594631 */
    /* find Middle C, three semitones above low A = 220 */
    c5 = 220.0 * pow(semitone_ratio, 3);
    /* MIDI note 0 is C, 5 octaves below middle C */
    c0 = c5 * pow(0.5, 5);

    /* calculate a frequency for a given MIDI note number */
    midinote = 60;
    frequency = c0 * pow(semitone_ratio, midinote);
    printf("MIDI note %d has frequency %f\n", midinote, frequency);

    return 0;
}
