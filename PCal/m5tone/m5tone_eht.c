/**
 * Extract the amplitude, phase, and coherence of the test tone
 * injected at sky frequency at APEX during 230 GHz VLBI.
 *
 * Accounts for an LO offset (APEX: 15.022 Hz) during phase extraction.
 * All parameters are hard coded. Change by editing the #defines below.
 *
 * Based on m5tone.py
 * (C) 2021 Jan Wagner
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mark5access.h>
#include <complex.h>
#include <math.h>
#include <fftw3.h>
#include <malloc.h>


/** Baseband */
#ifdef STATION_APEX
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    426     /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     -15.022 /* Offset of 1st LO to compensate for */
    #define HAVE_LO_OFFSET   1       /* 1: LO offset present and should be removed, 0: disable removal of LO offset */
    #define USE_FAST_LO_OFFSET 1     /* 1: use approximation, de-rotate tone in bin after DFT, faster, 0: no approximation, de-rotate the entire sample stream */
#endif
#ifdef STATION_APEX_2018_VDI // BAND 3 tone, 1st LO was a VDI synth that has an LO offset
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    529.0   /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     -15.022 /* Offset of 1st LO to compensate for */
    #define HAVE_LO_OFFSET   1       /* 1: LO offset present and should be removed, 0: disable removal of LO offset */
    #define USE_FAST_LO_OFFSET 1     /* 1: use approximation, de-rotate tone in bin after DFT, faster, 0: no approximation, de-rotate the entire sample stream */
#endif
#ifdef STATION_APEX_2023_VDI // BAND 4 tone, 1st LO was a VDI synth that has an LO offset
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    423.0   /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     -15.022 /* Offset of 1st LO to compensate for */
    #define HAVE_LO_OFFSET   1       /* 1: LO offset present and should be removed, 0: disable removal of LO offset */
    #define USE_FAST_LO_OFFSET 1     /* 1: use approximation, de-rotate tone in bin after DFT, faster, 0: no approximation, de-rotate the entire sample stream */
#endif
#ifdef STATION_APEX_2024_VDI_260G // BAND 4 tone, 1st LO was a VDI synth that has an LO offset
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    423.0   /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     -56.86  /* Offset of 1st LO to compensate for */
    #define HAVE_LO_OFFSET   1       /* 1: LO offset present and should be removed, 0: disable removal of LO offset */
    #define USE_FAST_LO_OFFSET 1     /* 1: use approximation, de-rotate tone in bin after DFT, faster, 0: no approximation, de-rotate the entire sample stream */
#endif
#ifdef STATION_APEX_2018_RS // BAND 3 tone, RohdeSchwarz(?) 1st LO synth borrowed from ALMA, without LO offset
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    529.0   /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     0       /* Offset of 1st LO to compensate for */
    #define HAVE_LO_OFFSET   0       /* 1: LO offset present and should be removed, 0: disable removal of LO offset */
    #define USE_FAST_LO_OFFSET 0     /* 1: use approximation, de-rotate tone in bin after DFT, faster, 0: no approximation, de-rotate the entire sample stream */
#endif
#ifdef STATION_KITTPEAK
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    500     /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     0       /* Offset of 1st LO to compensate for */
    #define HAVE_LO_OFFSET   0       /* 1: LO offset present and should be removed, 0: disable removal of LO offset */
    #define USE_FAST_LO_OFFSET 0     /* 1: use approximation, de-rotate tone in bin after DFT, faster, 0: no approximation, de-rotate the entire sample stream */
#endif
#ifndef LO_OFFSET_HZ
    #error "Please define STATION_APEX or STATION_KITTPEAK for compilation (e.g., gcc -Wall -DSTATION_APEX m5tone_eht2021.c -o m5tone_eht2021_Ax ...)"
#endif

/** Fourier transform and averaging */
#define DFT_LENGTH 409600     /* 204800 ch over 4096 Ms/s = 20 kHz/channel */
#define DFT_AVG  4000         /* 400 msec == EHT 2018 visibility data AP of 0.4s */
//#define DFT_AVG    1000     /* 1000 DFTs : 1000 blocks x 409600-point DFT / 4096Msps = 100 millisec */
//#define DFT_AVG  100        /* 10 msec */
//#define DFT_AVG  10         /* 1 msec */

/** Reference sinusoid based method */
#define REFERENCE_SIG_LENGTH 4096
#define SUB_AVG 100           /* choose so that REFERENCE_SIG_LENGTH*SUB_AVG == DFT_LENGTH */

/** Derived settings */
#define VDIF_CHAN_FS_HZ ( (double)(2e6*VDIF_CHAN_BW_MHZ) )
#define TONE_BIN        ( (size_t)(((double)TONE_FREQ_MHZ*DFT_LENGTH)/(2.0*VDIF_CHAN_BW_MHZ)) )


double processRecordingByDFT(const char *infilename, int chan, int fftlen, int navg, FILE* out);
double processRecordingByCrosscorr(const char *infilename, int chan, int nsampavg, FILE* out);


void usage()
{
    printf("Usage: m5tone <fname of vdif file>\n");
}


double processRecordingByDFT(const char *infilename, int chan, int fftlen, int navg, FILE* out)
{
    struct mark5_stream *ms;
    struct mark5_format_generic *fmt;

    float **data;
    fftwf_complex *dft_in, *dft_out;
    fftwf_plan pf;
    complex pcal;
    double ampsum, amp, phase, coherence;

    int l, i, j, n;
    int endoffile = 0;

    int mjd, sec;
    double ns, APmidMJD, scanStartMJD;
    const double AP_msec = ((1e3*fftlen)*(double)navg) / VDIF_CHAN_FS_HZ;
    char resultline[1024];

    size_t samplecount = 0;
    size_t APcount = 0;

    // Open VDIF file
    mark5_library_init();
    fmt = new_mark5_format_vdif(8192, 1, 2, 1, 8192, 32, 0);
    ms = new_mark5_stream(new_mark5_stream_file(infilename,0), fmt);
    if (!ms) {
        fprintf(stderr, "Problem creating mark5_stream\n");
        return 0;
    }
    mark5_stream_get_frame_time(ms, &mjd, &sec, &ns);
    scanStartMJD = mjd + (sec + ns/1e9) / 86400.0;

    // Summary of settings
    printf("Starting MJD: %.8f\n", scanStartMJD);
    printf("Hardcoded settings: %d-point FFT, tone at %.1f MHz, LO offset %.3f Hz that %s to be removed\n",
        DFT_LENGTH, (float)TONE_FREQ_MHZ, (float)LO_OFFSET_HZ,
        (HAVE_LO_OFFSET==1) ? "is" : "is NOT"
    );
    printf("Tone at %.3f kHz in the %.3f MHz wide band lands in %.3f kHz-wide bin %d.\n",
        (float)(1e3*TONE_FREQ_MHZ), (float)VDIF_CHAN_BW_MHZ, ((float)VDIF_CHAN_BW_MHZ*2e3)/DFT_LENGTH, (int)TONE_BIN
    );
    printf("Integrating for %.2f milliseconds with %d spectra per integration.\n", AP_msec, navg);

    // Allocate decoder output arrays
    data = (float **)malloc(ms->nchan * sizeof(float **));
    for (j = 0; j < ms->nchan; j++) {
        data[j] = (float *)malloc(fftlen*sizeof(float));
    }
    if (fftlen % ms->samplegranularity != 0) {
        fprintf(stderr, "WARNING -- decoding a nonstandard number of samples.  Expect bogus results\n");
    }

    // Prepare fftw for transform with fftlen points
    dft_in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftlen);
    dft_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftlen);
    pf = fftwf_plan_dft_1d(fftlen, dft_in, dft_out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Process entire file
    while (!endoffile) {

        // Prepare for next averaging period
        mark5_stream_get_frame_time(ms, &mjd, &sec, &ns);
        APmidMJD = mjd + (sec + 0.5e-3*AP_msec + ns/1e9) / 86400.0;
        pcal = 0.0 + 0.0 * I;
        ampsum = 0;

        // Averaging
        for (l = 0; l < navg; l++) {

            n = mark5_stream_decode(ms, fftlen, data);
            if (n < 0) { // (n != fftlen) {
                endoffile = 1;
                printf("Short read, %d bytes. EOF.\n", n);
                break;
            }

            #if HAVE_LO_OFFSET && !USE_FAST_LO_OFFSET
            for (i = 0; i < fftlen; i++) {
                // De-rotate the whole time domain sample stream by the LO offset
                const float lo_offset_phase = (2*M_PI) * ((double)samplecount) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
                const complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
                dft_in[i] = data[VDIF_CHAN_IDX][i] * lo_offset_phasor;
                samplecount++;
            }
            #endif

            #if !HAVE_LO_OFFSET || USE_FAST_LO_OFFSET
            for (i = 0; i < fftlen; i++) {
                // No time domain de-rotation, do it after DFT
                dft_in[i] = data[VDIF_CHAN_IDX][i];
            }
            samplecount += fftlen;
            #endif

            fftwf_execute(pf);

            #if HAVE_LO_OFFSET && USE_FAST_LO_OFFSET
            {
                // Post-DFT de-rotation by LO offset of the tone bin only
                // TODO: is the equivalent sample number correct?
                const float lo_offset_phase = (2*M_PI) * ((double)samplecount - (fftlen - TONE_BIN)) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
                const complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
                dft_out[TONE_BIN] = dft_out[TONE_BIN] * lo_offset_phasor;
            }
            #endif

            ampsum += cabs(dft_out[TONE_BIN]);
            pcal += dft_out[TONE_BIN];

            printf("%4d / %4d\r", l, navg);
        }

        pcal /= (fftlen * navg);
        ampsum /= (fftlen * navg);

        amp = cabs(pcal);
        phase = carg(pcal) * 180.0/M_PI;
        coherence = cabs(pcal) / ampsum;

        memset(resultline, '\0', sizeof(resultline));
        snprintf(resultline, sizeof(resultline)-1,
            "%.9lf mjd : %10.6lf sec : %11.6e /_ %+8.3f deg  %.3f\n",
            APmidMJD, 1e-3*(APcount*AP_msec + AP_msec/2),
            amp, phase, coherence
        );
        printf(resultline);

        memset(resultline, '\0', sizeof(resultline));
        snprintf(resultline, sizeof(resultline)-1,
            "%.9lf %10.6lf %11.6e %+8.3f %.3f\n",
            APmidMJD, 1e-3*(APcount*AP_msec + AP_msec/2),
            amp, phase, coherence
        );
        fprintf(out, resultline);

        APcount++;
    }

    // Tidy up
    for(j = 0; j < ms->nchan; j++) {
        free(data[j]);
    }
    free(data);

    delete_mark5_stream(ms);

    fftwf_destroy_plan(pf);
    fftwf_free(dft_in);
    fftwf_free(dft_out);

    return 0;
}

double processRecordingByCrosscorr(const char *infilename, int chan, int nsampavg, FILE* out)
{
    struct mark5_stream *ms;
    struct mark5_format_generic *fmt;

    float **data;
    float complex *reference;
    float complex pcal;
    double ampsum, amp, phase, coherence;

    int l, i, j, k, n;
    int endoffile = 0;

    int mjd, sec;
    double ns, APmidMJD, scanStartMJD;
    const double AP_msec = (1e3*nsampavg) / VDIF_CHAN_FS_HZ;
    char resultline[1024];

    size_t samplecount = 0;
    size_t APcount = 0;

    const int chunksize = REFERENCE_SIG_LENGTH;
    const int navg = nsampavg / (chunksize * SUB_AVG);

    // Open VDIF file
    mark5_library_init();
    fmt = new_mark5_format_vdif(8192, 1, 2, 1, 8192, 32, 0);
    ms = new_mark5_stream(new_mark5_stream_file(infilename,0), fmt);
    if (!ms) {
        fprintf(stderr, "Problem creating mark5_stream\n");
        return 0;
    }
    mark5_stream_get_frame_time(ms, &mjd, &sec, &ns);
    scanStartMJD = mjd + (sec + ns/1e9) / 86400.0;

    // Summary of settings
    printf("Starting MJD: %.8f\n", scanStartMJD);
    printf("Hardcoded settings: %d-point rotator, tone at %.1f MHz, LO offset %.3f Hz that %s to be removed\n",
        chunksize, (float)TONE_FREQ_MHZ, (float)LO_OFFSET_HZ,
        (HAVE_LO_OFFSET==1) ? "is" : "is NOT"
    );
    printf("Tone at %.3f kHz in the %.3f MHz wide band lands in %.3f kHz-wide bin %d.\n",
        (float)(1e3*TONE_FREQ_MHZ), (float)VDIF_CHAN_BW_MHZ, ((float)VDIF_CHAN_BW_MHZ*2e3)/chunksize, (int)TONE_BIN
    );
    printf("Integrating for %.2f milliseconds with %d spectra per integration.\n", AP_msec, navg);

    // Allocate decoder output arrays
    data = (float **)malloc(ms->nchan * sizeof(float **));
    for (j = 0; j < ms->nchan; j++) {
        data[j] = (float *)malloc(chunksize * sizeof(float));
    }
    if (chunksize % ms->samplegranularity != 0) {
        fprintf(stderr, "WARNING -- decoding a nonstandard number of samples.  Expect bogus results\n");
    }

    // Prepare reference signal
    reference = memalign(4096, sizeof(float complex) * chunksize);
    for (i = 0; i < chunksize; i++) {
        const float phase = 2.0 * M_PI * ((double)i) * (TONE_FREQ_MHZ*1e6) / VDIF_CHAN_FS_HZ;
        reference[i] = cosf(phase) + _Complex_I*sinf(phase);
    }

    // Process entire file
    while (!endoffile) {

        // Prepare for next averaging period
        mark5_stream_get_frame_time(ms, &mjd, &sec, &ns);
        APmidMJD = mjd + (sec + 0.5e-3*AP_msec + ns/1e9) / 86400.0;
        pcal = 0.0 + 0.0 * I;
        ampsum = 0;

        // Averaging
        for (l = 0; l < navg && !endoffile; l++) {

            float complex accu = 0.0f + _Complex_I*0.0f;

            for (k = 0; k < SUB_AVG && !endoffile; k++) {

                n = mark5_stream_decode(ms, chunksize, data);
                if (n < 1) { // (n != fftlen) {
                    endoffile = 1;
                    printf("Short read, %d bytes. EOF.\n", n);
                    break;
                }

            #if !HAVE_LO_OFFSET
                for (i = 0; i < chunksize; i++) {
                    accu += reference[i] * data[VDIF_CHAN_IDX][i];
                }
            #else
            // -- # elif !USE_FAST_LO_OFFSET
                for (i = 0; i < chunksize; i++) {
                    // De-rotate the whole time domain sample stream by the LO offset
                    const float lo_offset_phase = (-2*M_PI) * ((double)samplecount) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
                    const float complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
                    accu += (data[VDIF_CHAN_IDX][i] * reference[i]) * lo_offset_phasor;
                    samplecount++;
                }
            //#else
            //    for (i = 0; i < chunksize; i++) {
            //        // Coarse derotate first
            //        accu += reference[i] * data[VDIF_CHAN_IDX][i];
            //     }
            //    samplecount += chunksize;
            //    if (1) { // TODO: the below still decoheres for some reason!
            //        // Adjust the phase of accu
            //        const float lo_offset_phase = 2.0 * M_PI * ((double)samplecount - chunksize) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
            //        const float complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
            //        accu *= lo_offset_phasor;
            //    }
            #endif

            } // for(subavg)

            ampsum += cabs(accu);
            pcal += accu;

            printf("%4d / %4d\r", l, navg);
        }

        pcal /= (chunksize * navg);
        ampsum /= (chunksize * navg);

        amp = cabs(pcal);
        phase = carg(pcal) * 180.0/M_PI;
        coherence = cabs(pcal) / ampsum;

        memset(resultline, '\0', sizeof(resultline));
        snprintf(resultline, sizeof(resultline)-1,
            "%.9lf mjd : %10.6lf sec : %11.6e /_ %+8.3f deg  %.3f\n",
            APmidMJD, 1e-3*(APcount*AP_msec + AP_msec/2),
            amp, phase, coherence
        );
        printf(resultline);

        memset(resultline, '\0', sizeof(resultline));
        snprintf(resultline, sizeof(resultline)-1,
            "%.9lf %10.6lf %11.6e %+8.3f %.3f\n",
            APmidMJD, 1e-3*(APcount*AP_msec + AP_msec/2),
            amp, phase, coherence
        );
        fprintf(out, resultline);

        APcount++;
    }


    // Tidy up
    for(j = 0; j < ms->nchan; j++) {
        free(data[j]);
    }
    free(data);
    free(reference);

    delete_mark5_stream(ms);

    return 0;
}


int main(int argc, char **argv)
{
    char *infile = argv[1];
    char *filename;
    char *resultsfile;
    FILE *fout;

    if (argc != 2) {
        usage();
        exit(1);
    }

    // Open unbuffered output file writer stream
    filename = strrchr(infile, '/');
    if (filename == NULL) {
        filename = infile;
    }
    if (filename[0] == '/') {
        filename++;
    }
    resultsfile = malloc(strlen(filename) + strlen(".m5t") + 1);
    sprintf(resultsfile, "%s.m5t", filename);
    printf("resultsfile = %s\n", resultsfile);
    fout = fopen(resultsfile, "w");
    if (fout == NULL) {
        printf("Error");
    }
    setvbuf(fout, (char *)NULL, _IONBF, 0);

    // Extract phase of tone in input file
    processRecordingByDFT(argv[1], VDIF_CHAN_IDX, DFT_LENGTH, DFT_AVG, fout);
//    processRecordingByCrosscorr(argv[1], VDIF_CHAN_IDX, DFT_LENGTH*DFT_AVG, fout);

    fclose(fout);
    free(resultsfile);

    return 0;
}
