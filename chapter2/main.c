#include <stdio.h>
#include <stdlib.h>
#include "portsf.h"
#include "wave.h"

#define BLOCK_SIZE 1024

enum {ARG_PROGNAME, ARG_OUTFILE, ARG_DUR, ARG_SRATE, ARG_AMP, ARG_FREQ, ARG_NARGS};

int main(int argc, char* argv[]) {
    PSF_PROPS outprops;
    unsigned long nbufs, outframes, remainder;

    /* init resource vars to default states */
    int ifd = -1, ofd = 1;
    int error = 0;
    psf_format outformat = PSF_FMT_UNKNOWN;
    PSF_CHPEAK* peaks = NULL;
    printf("SIGGEN: produces a sine wave (as a mono soundfile)\n");

    if (argc < ARG_NARGS) {
        printf("insufficient number of arguments\n"
            "usage:\nsiggen outfile dur srate amp freq\n");
        return 1;
    }

    double srate = atof(argv[ARG_SRATE]);

    outprops.srate = srate;
    outprops.chans = 1;
    outprops.samptype = PSF_SAMP_16;
    outprops.chformat = STDWAVE;

    double dur = atof(argv[ARG_DUR]);
    outframes = (unsigned long) (dur * outprops.srate + 0.5);
    nbufs = outframes / BLOCK_SIZE;
    remainder = outframes - nbufs * BLOCK_SIZE;
    if (remainder > 0)
        nbufs++;

    if (psf_init()) {
        printf("unable to start portsf\n");
        return 1;
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

    ofd = psf_sndCreate(argv[2], &outprops, 0,0, PSF_CREATE_RDWR);
    if (ofd < 0) {
        printf("Error: unable to create outfile %s\n",
            argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    /* allocate space for PEAK info */
    peaks = (PSF_CHPEAK*) malloc(outprops.chans * sizeof(PSF_CHPEAK));
    if (peaks==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    printf("copying...\n");

    /* do all cleanup */
    exit:
    if (ifd >= 0)
        psf_sndClose(ifd);
    if (ofd >= 0)
        psf_sndClose(ofd);
    if (peaks)
        free(peaks);
    psf_finish();
    return error;
}
