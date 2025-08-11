#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv)
{
    const float sampleRate = 44100;
    float levelBegin, levelEnd, releaseTime, samplePoints;
    float coeff, currentLevel;


    if (argc < 5) {
        fprintf(stderr, "usage: expenvelgen levelBegin levelEnd releaseTime samplePoints");
        return 1;
    }

    levelBegin = atof(argv[1]);
    levelEnd = atof(argv[2]);
    releaseTime = atof(argv[3]);
    samplePoints = atof(argv[4]);

    currentLevel = levelBegin;
    coeff = (log(levelEnd) - log(levelBegin)) /
            (releaseTime * sampleRate);

    float step = releaseTime / samplePoints;
    float thisstep = 0.0;

    for (int i = 0; i < samplePoints; i++) {
        currentLevel += coeff * currentLevel;
        fprintf(stdout, "%.4lf\t%.8lf\n", thisstep, currentLevel);
        thisstep += step;
    }
}