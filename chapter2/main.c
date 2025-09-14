#include <stdio.h>
#include <stdlib.h>

#include "breakpoints.h"
#include "portsf.h"
#include "wave.h"

#define BLOCK_SIZE 1024

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_TYPE, ARG_DUR, ARG_SRATE, ARG_AMP, ARG_FREQ, ARG_NARGS};

int main(int argc, char* argv[]) {
    PSF_PROPS outprops;
    unsigned long nbufs, outframes, remainder, i;
    unsigned long nframes = BLOCK_SIZE;

    OSCIL **oscs = NULL;
    double *oscamps = NULL, *oscfreqs = NULL; /* for oscbank amp and freq data */
    unsigned long noscs;

    BRKSTREAM* ampstream = NULL;
    FILE *fpamp = NULL;
    unsigned long brkampSize = 0;
    double minval, maxval;
    double amp;

    /* init resource vars to default states */
    int ofd = 1;
    int error = 0;
    psf_format outformat = PSF_FMT_UNKNOWN;
    PSF_CHPEAK* peaks = NULL;
    printf("SIGGEN: produces a sine wave (as a mono soundfile)\n");

    if (argc < ARG_NARGS) {
        printf("insufficient number of arguments\n"
            "usage:\noscgen outfile dur srate nchans amp freq wavetype noscs\n"
            "where wavetype=:\n"
            "0     = square\n"
            "1     = triangle\n"
            "2     = saw up\n"
            "3     = saw down\n"
            "dur   = duration of outfile (seconds)\n"
            "srate = required sample rate of outfile\n"
            "amp   = amplitude value or breakpoint file (0 < amp <= 1.0)\n"
            "freq  = frequency (freq > 0)\n");
        return 1;
    }


    int wavetype = atoi(argv[ARG_TYPE]);
    if (wavetype < WAVE_SINE || wavetype >= WAVE_NTYPES) {
        printf("Error: bad value for wave type\n");
        error++;
        goto exit;
    }

    tickfunc tick;
    switch (wavetype) {
        case WAVE_SINE:
            tick = sinetick;
            break;
        case WAVE_TRIANGLE:
            tick = tritick;
            break;
        case WAVE_SQUARE:
            tick = sqtick;
            break;
        case WAVE_SAWUP:
            tick = sawutick;
            break;
        case WAVE_SAWDOWN:
            tick = sawdtick;
        default:
            break;
    }

    double dur = atof(argv[ARG_DUR]);
    unsigned long srate = atof(argv[ARG_SRATE]);
    double freq = atof(argv[ARG_FREQ]);
    if (freq < 0) {
        printf("Error: frequency must be positive\n");
        error++;
        goto exit;
    }


    OSCIL* p_osc = new_oscil(srate);

    outprops.srate = srate;
    outprops.chans = 1;
    outprops.samptype = PSF_SAMP_16;
    outprops.chformat = STDWAVE;

    outframes = (unsigned long) (dur * outprops.srate + 0.5);
    nbufs = outframes / nframes;
    remainder = outframes - nbufs * nframes;
    if (remainder > 0)
        nbufs++;

    if (psf_init()) {
        printf("unable to start portsf\n");
        return 1;
    }

    fpamp = fopen(argv[ARG_AMP], "r");
    if (fpamp == NULL) {
        amp = atof(argv[ARG_AMP]);
        if (amp <= 0.0 || amp > 1.0) {
            printf("Error: amplitude value out of range:"
                   "0.0 < amp <= 1.0\n");
            error++;
            goto exit;
        }
    } else {
        ampstream = bps_newstream(fpamp, outprops.srate, &brkampSize);

        if (bps_getminmax(ampstream, &minval, &maxval)) {
            printf("Error reading range of breakpoint file %s\n", argv[ARG_AMP]);
            error++;
            goto exit;
        }
    }



    /* output in stereo will be twice as big */
    float* outframe = malloc(BLOCK_SIZE * outprops.chans * sizeof(float));
    if (outframe==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    /* check outfile extension is one we know about */
    outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
    if (outformat == PSF_FMT_UNKNOWN) {
        printf("Outfile name %s has unknown format.\n"
            "Use any of .wav, .aiff, .aif, .afc, .aifc\n",
            argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    outprops.format = outformat;

    ofd = psf_sndCreate(argv[ARG_OUTFILE], &outprops, 0,0, PSF_CREATE_RDWR);
    if (ofd < 0) {
        printf("Error: unable to create outfile %s\n",
            argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    for (i = 0; i < nbufs; i++) {
        if (i == nbufs-1) nframes = remainder;
        for (unsigned long j = 0; j < nframes; j++) {
            if (ampstream)
                amp = bps_tick(ampstream);
            if (amp == 1.0) amp = 0.999f;
            // TODO add breakpoint support for frequency
            outframe[j] = (float) (amp * tick(p_osc, freq));
        }
        if (psf_sndWriteFloatFrames(ofd, outframe, nframes) != nframes) {
            printf("Error writing to outfile\n");
            error++;
            break;
        }
    }

    /* do all cleanup */
    exit:
    if (fpamp)
        if (fclose(fpamp))
            printf("Error closing file\n");
    if (ofd >= 0)
        psf_sndClose(ofd);
    psf_finish();
    return error;
}
