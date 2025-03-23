#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main()
{
    double semitone_ratio = pow(2, 1/12.0), c5 = 220.0 * pow(semitone_ratio, 3), c0 = c5 * pow(0.5, 5);
    char input[8];
    int    midinote;


    printf("enter frequency (XXX.XX): ");
    if(fgets(input, sizeof(input), stdin) == NULL) {
        printf("error reading the input\n");
        return 1;
    }

    double frequency = atof(input);

    double fracmidi = log(frequency / c0) / log(semitone_ratio);
    midinote = (int) (fracmidi + 0.5);
    
    // Calculate actual MIDI note frequency
    double midi_freq = c0 * pow(semitone_ratio, midinote);
    
    // Find adjacent MIDI note frequencies
    double midi_freq_lower = c0 * pow(semitone_ratio, midinote - 1);
    
    // Determine pitch bend percentage
    int deviation = (int) round(((frequency - midi_freq) / (midi_freq - midi_freq_lower) * 100.0));
    
    // Print result
    printf("The nearest MIDI note to frequency %.2f Hz is %d (%.2f Hz)\n", frequency, midinote, midi_freq);
    printf("Pitchbend deviation: %i%%\n", deviation);

    return 0;
}
