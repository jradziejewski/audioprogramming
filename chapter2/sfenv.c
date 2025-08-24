#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "portsf.h"
#include "breakpoints.h"

#define BLOCK_SIZE 1024

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_BRKFILE, ARG_NARGS};

int main(int argc, char* argv[]) {
    PSF_PROPS inprops, outprops;
    long framesread, totalread;

    /* init resource vars to default states */
    int ifd = -1, ofd = 1;
    int error = 0;
    psf_format outformat = PSF_FMT_UNKNOWN;
    PSF_CHPEAK* peaks = NULL;
    float* inframe = NULL;
    float panpos;
    PANPOS thispos;
    FILE* brkfile = NULL;
    unsigned long npoints;
    BREAKPOINT* points = NULL;

    printf("SFENV: apply breakpoint envelope to source soundfile\n");

    if (argc < ARG_NARGS) {
        printf("insufficient number of arguments\n"
            "usage:\nsfenv infile outfile envelope.brk\n"
            "\tenvelope.brk is breakpoint file");
        return 1;
    }

    brkfile = fopen(argv[ARG_BRKFILE], "r");
    if (brkfile == NULL) {
        printf("Error: Unable to open\n"
            "breakpoint file %s\n", argv[ARG_BRKFILE]);
        error++;
        goto exit;
    }

    points = get_breakpoints(brkfile, &npoints);
    if (points==NULL) {
        printf("No breakpoints read.\n");
        error++;
        goto exit;
    }

    if (npoints < 2) {
        printf("Error: at least two breakpoints required\n");
        free(points);
        fclose(brkfile);
        return 1;
    }

    if (points[0].time != 0.0) {
        printf("Error in breakpoint data: "
            "first time must be 0.0\n");
        error++;
        goto exit;
    }

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

    /* allocate space for PEAK info */
    peaks = (PSF_CHPEAK*) malloc(outprops.chans * sizeof(PSF_CHPEAK));
    if (peaks==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    printf("copying...\n");

    double sampletime = 0.0;
    double timeincr = 1.0 / inprops.srate;
    double stereopos;
    PANPOS pos;

    /* single frame loop to do copy, report any errors */
    framesread = psf_sndReadFloatFrames(ifd, inframe, BLOCK_SIZE);

    /* init time position counter for reading envelope */
    double incr = 1.0 / inprops.srate;
    /* setup counters to track through the breakpoint data */
    double curpos = 0.0;
    unsigned long ileft = 0, iright = 1;
    BREAKPOINT leftpoint = points[ileft];
    BREAKPOINT rightpoint = points[iright];
    double width = rightpoint.time - leftpoint.time;
    double height = rightpoint.value - leftpoint.value;
    bool more_points = true;


    totalread = 0;
    while (framesread > 0) {
        totalread++;
        double frac, thisamp;
        if (totalread % (BLOCK_SIZE) == 0) {
            printf("\rCopied %ld frames", totalread);
            fflush(stdout);
        }

        for (unsigned long i = 0; i < framesread; i++) {
            if (more_points) {
                if (width == 0.0)
                    thisamp = rightpoint.value;
            } else {
                /* not vertical: get interp value from this span */
                frac = (curpos - leftpoint.time) / width;
                thisamp = leftpoint.value + (height * frac);
            }
            /* move up ready for next sample */
            curpos += incr;
            if (curpos > rightpoint.time) {
                ileft++; iright++;
                if (iright < npoints) {
                    leftpoint = points[ileft];
                    rightpoint = points[iright];
                    width = rightpoint.time - leftpoint.time;
                    height = rightpoint.value - leftpoint.value;
                } else
                    more_points = false;
            }
            inframe[i] = (float)(inframe[i] * thisamp);
        }

        if (psf_sndWriteFloatFrames(ofd, inframe, framesread) != framesread) {
            printf("Error writing to outfile!\n");
            error++;
            break;
        }
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
    if (brkfile)
        fclose(brkfile);
    if (points)
        free(points);
    if (ifd >= 0)
        psf_sndClose(ifd);
    if (ofd >= 0)
        psf_sndClose(ofd);
    if (inframe)
        free(inframe);
    if (peaks)
        free(peaks);
    psf_finish();
    return error;
}