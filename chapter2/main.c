#include <stdio.h>
#include <stdlib.h>
#include "portsf.h"

#define BLOCK_SIZE 1024

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_PANPOS, ARG_NARGS};

int main(int argc, char* argv[]) {
    PSF_PROPS inprops, outprops;
    long framesread, totalread;

    /* init resource vars to default states */
    int ifd = -1, ofd = 1;
    int error = 0;
    psf_format outformat = PSF_FMT_UNKNOWN;
    PSF_CHPEAK* peaks = NULL;
    float* inframe = NULL;
    float* outframe = NULL;
    float panpos;
    PANPOS thispos;

    printf("SFPAN: change panning of soundfile\n");

    if (argc < ARG_NARGS) {
        printf("insufficient number of arguments\n"
            "usage:\nsfpan infile outfile panpos\n"
            "\twhere ampfac > 0");
        return 1;
    }

    panpos = atof(argv[ARG_PANPOS]);
    if (panpos > 1.0 || panpos < -1.0) {
        printf("Error: panpos must be between -1 and 1.\n");
        return 1;
    }

    thispos = simplepan(panpos);

    if (psf_init()) {
        printf("unable to start portsf\n");
        return 1;
    }

    ifd = psf_sndOpen(argv[ARG_INFILE], &inprops, 0);
    if (ifd < 0) {
        printf("Error: unable to open infile %s\n",
        argv[ARG_INFILE]);
        return 1;
    }

    if (inprops.chans != 1) {
        printf("Error: infile must be mono.\n");
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
    outprops = inprops;
    outprops.format = outformat;
    outprops.chans = 2;

    ofd = psf_sndCreate(argv[2], &outprops, 0,0, PSF_CREATE_RDWR);
    if (ofd < 0) {
        printf("Error: unable to create outfile %s\n",
            argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    /* allocate  space for input frames */
    inframe = (float*) malloc(inprops.chans * BLOCK_SIZE * sizeof(float));
    if (inframe==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    /* output in stereo will be twice as big */
    outframe = (float *) malloc(inprops.chans * BLOCK_SIZE * sizeof(float));
    if (outframe==NULL) {
        puts("No memory!\n");
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

    /* single frame loop to do copy, report any errors */
    framesread = psf_sndReadFloatFrames(ifd, inframe, BLOCK_SIZE);
    totalread = 0;
    while (framesread > 0) {
        totalread++;
        if (totalread % (BLOCK_SIZE) == 0) {
            printf("\rCopied %ld frames", totalread);
            fflush(stdout);
        }

        if (psf_sndWriteFloatFrames(ofd, outframe, framesread) != framesread) {
            printf("Error writing to outfile!\n");
            error++;
            break;
        }
        /* <-------------- Do any processing here! --------------> */
        framesread = psf_sndReadFloatFrames(ifd, inframe, BLOCK_SIZE);
    }
    if (framesread < 0) {
        printf("Error reading infile. Outfile is incomplete.\n");
        error++;
    }
    else
        printf("Done. %d sample frames copied to %s\n",
            totalread * BLOCK_SIZE, argv[ARG_OUTFILE]);

    /* report PEAK values to user */
    if (psf_sndReadPeaks(ofd, peaks, NULL) > 0) {
        long i;
        double peaktime;
        printf("PEAK information:\n");
        for (int i = 0; i < outprops.chans; i++) {
            peaktime = (double) peaks[i].pos / outprops.srate;
            printf("CH %d:t%.4f at %.4f secs\n",
                i + 1, peaks[i].val, peaktime);
        }
    }
    /* do all cleanup */
    exit:
    if (ifd >= 0)
        psf_sndClose(ifd);
    if (ofd >= 0)
        psf_sndClose(ofd);
    if (inframe)
        free(inframe);
    if(outframe)
        free(outframe);
    if (peaks)
        free(peaks);
    psf_finish();
    return error;
}

