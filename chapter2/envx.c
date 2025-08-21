#include <stdio.h>
#include <stdlib.h>
#include "portsf.h"

#define DEFAULT_WINDOW_MSECS (15)

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_NARGS};

int main(int argc, char* argv[]) {
    int error = 0;
    double windur = DEFAULT_WINDOW_MSECS;

    printf("ENVX: extract amplitude envelope"
        "from mono soundfile\n");

    if (argc < ARG_NARGS) {
        printf("insufficient arguments\n"
            "usage: envx [-wN] infile outfile.brk\n"
            "             -wN: set extraction window size to N msecs\n"
            "                  (default: 15)\n");
        return 1;
    }

    if (argc > 1) {
        char flag;
        while (argv[1][0] == '-') {
            flag = argv[1][1];
            switch (flag) {
                case('\0'):
                    printf("Error: missing flag name\n");
                    return 1;
                case('w'):
                    windur = atof(&argv[1][2]);
                    if (windur <= 0.0) {
                        printf("bad value for Window Duration."
                            "Must be positive.\n");
                        return 1;
                    }
                    break;
                default:
                    break;
            }
            argc--;
            argv++;
        }
    }
    if (psf_init()) {
        printf("unable to start portsf\n");
        return 1;
    }

    FILE* outfile = fopen(argv[ARG_OUTFILE], "w");
    if (outfile == NULL) {
        printf("envx: unable to create breakpoint file %s\n",
            argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    PSF_PROPS inprops;
    int ifd = -1;
    /* TODO: verify infile format for this application */
    ifd = psf_sndOpen(argv[ARG_INFILE], &inprops, 0);
    if (ifd < 0) {
        printf("Error: unable to open infile %s\n",
        argv[ARG_INFILE]);
        return 1;
    }

    if (inprops.chans > 1) {
        printf("Soundfile contains %d channels, but should be mono.\n", inprops.chans);
        error++;
        goto exit;
    }

    windur /= 1000.0; // convert to seconds
    unsigned long winsize = (unsigned long)(windur * inprops.srate);

    /* allocate  space for one sample frame */
    float* inframe = (float*) malloc(winsize * sizeof(float));
    if (inframe==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    long framesread, totalread;
    /* single frame loop to do copy, report any errors */

    double brktime = 0.0; // holds the time for the current breakpoint time
    int npoints = 0;

    while ((framesread = psf_sndReadFloatFrames(ifd, inframe, winsize)) > 0) {
        double amp = maxsamp(inframe, framesread);

        /* TODO store brktime and amp as a breakpoint */
        if (fprintf(outfile, "%f\t%f\n", brktime, amp) < 2) {
            printf("Failed to write to breakpoint file %s\n", argv[ARG_OUTFILE]);
            error++;
            break;
        }

        npoints++;
        brktime += windur;
    }
    if (framesread < 0) {
        printf("Error reading infile. Outfile is incomplete.\n");
        error++;
    }
    else
        printf("Done. %d errors\n", error);
    printf("%d sample frames copied to %s\n",
            npoints, argv[ARG_OUTFILE]);

    /* do all cleanup */
    exit:
    if (ifd >= 0)
        psf_sndClose(ifd);
    if (inframe)
        free(inframe);
    if (outfile)
        if (fclose(outfile))
            printf("envx: failed to close output file %s \n", argv[ARG_OUTFILE]);
    psf_finish();
    return error;
}
