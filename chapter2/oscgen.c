#include <stdio.h>
#include <stdlib.h>

#include "breakpoints.h"
#include "portsf.h"
#include "wave.h"

#define BLOCK_SIZE 1024

enum {
    ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE, ARG_NCHANS, ARG_AMP,
    ARG_FREQ, ARG_WAVETYPE, ARG_NOSCS, ARG_NARGS
};


int main(int argc, char* argv[]) {
    PSF_PROPS outprops;
    unsigned long nbufs, outframes, remainder, i;
    unsigned long nframes = BLOCK_SIZE;

    BRKSTREAM *ampstream = NULL, *freqstream = NULL;
    FILE *fpamp= NULL, *fpfreq = NULL;
    unsigned long brkampSize = 0;
    double minval, maxval;
    double amp, freq;

    /* init resource vars to default states */
    int ofd = 1;
    int error = 0;
    psf_format outformat = PSF_FMT_UNKNOWN;
    PSF_CHPEAK* peaks = NULL;

    printf("OSCGEN: sinusoidal oscillator bank\n");

    if (argc < ARG_NARGS) {
        printf("insufficient number of arguments\n"
            "usage:\noscgen outfile dur srate nchans amp freq wavetype noscs\n"
            "where wavetype=:\n"
            "1     = square\n"
            "2     = triangle\n"
            "3     = saw up\n"
            "4     = saw down\n"
            "dur   = duration of outfile (seconds)\n"
            "srate = required sample rate of outfile\n"
            "amp   = amplitude value or breakpoint file (0 < amp <= 1.0)\n"
            "freq  = frequency (freq > 0)\n");
        return 1;
    }


    int wavetype = atoi(argv[ARG_WAVETYPE]);
    if (wavetype < WAVE_SINE || wavetype >= WAVE_NTYPES) {
        printf("Error: bad value for wave type\n");
        error++;
        goto exit;
    }

    tickfunc tick;

    double dur = atof(argv[ARG_DUR]);
    unsigned long srate = atoi(argv[ARG_SRATE]);
    unsigned long noscs = atoi(argv[ARG_NOSCS]);


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

    fpfreq = fopen(argv[ARG_FREQ], "r");

    if (fpfreq == NULL) {
        freq = atof(argv[ARG_FREQ]);
        if (freq <= 0) {
            printf("Error: frequency must be positive\n");
            error++;
            goto exit;
        }
        printf("%d\n", freq);
    } else {
        freqstream = bps_newstream(fpfreq, outprops.srate, &brkampSize);

        if (bps_getminmax(freqstream, &minval, &maxval) == -1) {
            printf("Error reading range of breakpoint file %s\n", argv[ARG_FREQ]);
            error++;
            goto exit;
        }
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

    /* create amp and freq arrays */
    double *oscamps = (double*) malloc(noscs * sizeof(double));
    if (oscamps==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }
    double *oscfreqs = (double*) malloc(noscs * sizeof(double));
    if (oscfreqs==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }
    /* create array of pointers to OSCILs */
    OSCIL **oscs = (OSCIL**) malloc(noscs * sizeof(OSCIL*));
    if (oscs==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    double ampfac = 1.0;
    double freqfac = 1.0;
    double ampadjust = 0.0;

    for (i = 0; i < noscs; i++) {
        oscs[i] = new_oscil(outprops.srate);
        if (oscs[i] == NULL) {
            puts("No memory!\n");
            error++;
            goto exit;
        }
    }

    switch (wavetype) {
        case WAVE_TRIANGLE:
            for(i=0;i< noscs;i++) {
                oscamps[i] = ampfac;
                oscfreqs[i] = freqfac;
                freqfac += 2.0;
                ampadjust += ampfac;
                ampfac = 1.0 /(freqfac*freqfac);
            }
            break;
        case WAVE_SQUARE:
            for (i = 0; i < noscs; i++) {
                ampfac = 1.0 / freqfac;
                oscamps[i] = ampfac;
                oscfreqs[i] = freqfac;
                freqfac += 2.0;
                ampadjust += ampfac;
            }
            break;
        case WAVE_SAWUP:
        case WAVE_SAWDOWN:
            for (i = 0; i < noscs; i++) {
                ampfac = 1.0 / freqfac;
                oscamps[i] = ampfac;
                oscfreqs[i] = freqfac;
                freqfac += 1.0;
                ampadjust += ampfac;
            }
            if (wavetype == WAVE_SAWUP) {
                ampadjust = -ampadjust; /* inverts waveform */
            }
            break;
        default:
            break;
    }

    for (i = 0; i < noscs; i++) {
        oscamps[i] /= ampadjust;
    }

    double val;

    for (i = 0; i < nbufs; i++) {
        if (i == nbufs-1) nframes = remainder;
        for (unsigned long j = 0; j < nframes; j++) {
            long k;
            if (freqstream)
                freq = bps_tick(freqstream);
            if (ampstream)
                amp = bps_tick(ampstream);
            val = 0.0;
            for (k = 0; k < noscs; k++) {
                val += oscamps[k] * sinetick(oscs[k], freq * oscfreqs[k]);
            }
            outframe[j] = (float)(val * amp);
        }
        if (psf_sndWriteFloatFrames(ofd, outframe, nframes) != nframes) {
            printf("Error writing to outfile\n");
            error++;
            break;
        }
    }

    /* do all cleanup */
    exit:
    if (oscamps)
        free(oscamps);
    if (oscfreqs)
        free(oscfreqs);
    for (i = 0; i < noscs; i++) {
        if (oscs[i])
            free(oscs[i]);
    }
    if (fpamp)
        if (fclose(fpamp))
            printf("Error closing file\n");
    if (ofd >= 0)
        psf_sndClose(ofd);
    psf_finish();
    return error;
}
