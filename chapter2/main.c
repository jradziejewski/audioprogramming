#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "portsf.h"

#define NFRAMES (1024)

enum {ARG_PROGNAME, ARG_INFILE, ARG_OUTFILE, ARG_DBVAL, ARG_NARGS};

int main(int argc, char* argv[]) {
    PSF_PROPS props;
    long framesread, totalread;

    /* init resource vars to default states */
    int ifd = -1, ofd = 1;
    int error = 0;
    psf_format outformat = PSF_FMT_UNKNOWN;
    PSF_CHPEAK* peaks = NULL;
    float* frame = NULL;
    double dbval, inpeak = 0.0;
    float ampfac, scalefac;

    printf("SFNORM: normalize level of soundfile\n");

    if (argc < ARG_NARGS) {
        printf("insufficient number of arguments\n"
            "usage:\nsfnorm infile outfile dbVal\n"
            "\twhere dbVal <= 0");
        return 1;
    }

    dbval = atof(argv[ARG_DBVAL]);
    if (dbval > 0.0) {
        printf("Error: dbVal cannot be positive.\n");
        return 1;
    }
    ampfac = (float) pow(10.0, dbval/20);

    if (psf_init()) {
        printf("unable to start portsf\n");
        return 1;
    }

    ifd = psf_sndOpen(argv[ARG_INFILE], &props, 0);
    if (ifd < 0) {
        printf("Error: unable to open infile %s\n",
        argv[ARG_INFILE]);
        return 1;
    }

    props.samptype = PSF_SAMP_IEEE_FLOAT;
    /* check outfile extension is one we know about */
    outformat = psf_getFormatExt(argv[ARG_OUTFILE]);
    if (outformat == PSF_FMT_UNKNOWN) {
        printf("Outfile name %s has unknown format.\n"
            "Use any of .wav, .aiff, .aif, .afc, .aifc\n",
            argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }
    props.format = outformat;

    ofd = psf_sndCreate(argv[2], &props, 0,0, PSF_CREATE_RDWR);
    if (ofd < 0) {
        printf("Error: unable to create outfile %s\n",
            argv[ARG_OUTFILE]);
        error++;
        goto exit;
    }

    /* allocate  space for one sample frame */
    frame = (float*) malloc(props.chans * NFRAMES * sizeof(float));
    if (frame==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    /* allocate space for PEAK info */
    peaks = (PSF_CHPEAK*) malloc(props.chans * sizeof(PSF_CHPEAK));
    if (peaks==NULL) {
        puts("No memory!\n");
        error++;
        goto exit;
    }

    printf("copying...\n");

    /* single frame loop to do copy, report any errors */
    framesread = psf_sndReadFloatFrames(ifd, frame, NFRAMES);

    if (psf_sndReadPeaks(ifd, peaks, NULL) > 0) {
        long i;
        for (i = 0; i < props.chans; i++) {
            if (peaks[i].val > inpeak)
                inpeak = peaks[i].val;
        }
    }
    else {
        while (framesread > 0) {
            double thispeak;
            thispeak = maxsamp(frame, NFRAMES * props.chans);
            if (thispeak > inpeak)
                inpeak = thispeak;
            framesread = psf_sndReadFloatFrames(ifd, frame, NFRAMES);
        }
    }

    if ((psf_sndSeek(ifd, 0, PSF_SEEK_SET)) < 0) {
        printf("Error: unable to rewind infile.\n");
        error++;
        goto exit;
    }

    framesread = psf_sndReadFloatFrames(ifd, frame, NFRAMES);

    totalread = 0;
    while (framesread > 0) {
        totalread += framesread;

        if (totalread % (NFRAMES) == 0) {
            printf("\rCopied %ld frames", totalread);
            fflush(stdout);
        }


        scalefac = (float)(ampfac/inpeak);

        for (int i = 0; i < framesread * props.chans; i++) {
            frame[i] *= scalefac;
        }

        if (psf_sndWriteFloatFrames(ofd, frame, framesread) != framesread) {
            printf("Error writing to outfile!\n");
            error++;
            break;
        }
        /* <-------------- Do any processing here! --------------> */

        framesread = psf_sndReadFloatFrames(ifd, frame, NFRAMES);
    }

    if (framesread < 0) {
        printf("Error reading infile. Outfile is incomplete.\n");
        error++;
    }
    else
        printf("Done. %d sample frames copied to %s\n",
            totalread * NFRAMES, argv[ARG_OUTFILE]);

    /* report PEAK values to user */
    if (psf_sndReadPeaks(ofd, peaks, NULL) > 0) {
        long i;
        double peaktime;
        printf("PEAK information:\n");
        for (int i = 0; i < props.chans; i++) {
            peaktime = (double) peaks[i].pos / props.srate;
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
    if (frame)
        free(frame);
    if (peaks)
        free(peaks);
    psf_finish();
    return error;
}