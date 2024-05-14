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

#include <assert.h>
#include <complex.h>
#include <malloc.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <mark5access.h>
#include <fftw3.h>

#include "m5tone_hardcoded_confs.h"

void usage()
{
    printf("Usage: m5tone <vdif file>\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Fourier transform and averaging */
#define DFT_LENGTH 409600     /* 204800 ch over 4096 Ms/s = 20 kHz/channel */
#define DFT_AVG  4000         /* 400 msec == EHT 2018 visibility data AP of 0.4s */
//#define DFT_AVG    1000     /* 1000 DFTs : 1000 blocks x 409600-point DFT / 4096Msps = 100 millisec */
//#define DFT_AVG  100        /* 10 msec */
//#define DFT_AVG  10         /* 1 msec */

#define NUM_WORKER_THREADS 6

/** Derived settings */
#define VDIF_CHAN_FS_HZ ( (double)(2e6*VDIF_CHAN_BW_MHZ) )
#define TONE_BIN        ( (size_t)(((double)TONE_FREQ_MHZ*DFT_LENGTH)/(2.0*VDIF_CHAN_BW_MHZ)) )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct workerthread_tt
{
    // Config
    size_t fftlen;
    size_t navg;

    // Data in
    double APmidMJD;
    double samplenumber;
    float *channeldata;
    int channeldata_valid;

    // Results out
    double amp, phase, coherence;

    // Internals
    pthread_t threadid;
    fftwf_complex *dft_in, *dft_out;
    fftwf_plan pf;

} workerthread_t;

static void worker_init(workerthread_t *w, size_t fftlen, size_t navg);
static void worker_deinit(workerthread_t *w);
static void worker_process(workerthread_t *w);
static void *worker_process_invoke(void *threadargs);

double processRecordingByDFT(const char *infilename, int chan, int fftlen, int navg, FILE* out, int nthreads);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void worker_init(workerthread_t *w, size_t fftlen, size_t navg)
{
    w->fftlen = fftlen;
    w->navg = navg;
    w->channeldata_valid = 0;
    w->samplenumber = 0;

    w->channeldata = (float *)malloc(sizeof(float) * fftlen * (navg+1));
    w->dft_in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftlen);
    w->dft_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fftlen);
    w->pf = fftwf_plan_dft_1d(fftlen, w->dft_in, w->dft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

void worker_deinit(workerthread_t *w)
{
    free(w->channeldata);
    fftwf_destroy_plan(w->pf);
    fftwf_free(w->dft_in);
    fftwf_free(w->dft_out);
}

void worker_process(workerthread_t *w)
{
    int l, i;

    complex pcal = 0.0 + 0.0 * I;
    double ampsum = 0;

    float *samples = w->channeldata;

    if (!w->channeldata_valid) {
        return;
    }

    for (l = 0; l < w->navg; l++) {

        #if HAVE_LO_OFFSET && !USE_FAST_LO_OFFSET
        for (i = 0; i < w->fftlen; i++) {
            // De-rotate the whole time domain sample stream by the LO offset
            const float lo_offset_phase = (2*M_PI) * ((double)w->samplenumber) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
            const complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
            w->dft_in[i] = samples[i] * lo_offset_phasor;
            w->samplenumber++;
        }
        #endif

        #if !HAVE_LO_OFFSET || USE_FAST_LO_OFFSET
        for (i = 0; i < w->fftlen; i++) {
            // No time domain de-rotation, do it after DFT
            w->dft_in[i] = samples[i];
        }
        w->samplenumber += w->fftlen;
        #endif

        fftwf_execute(w->pf);

        #if HAVE_LO_OFFSET && USE_FAST_LO_OFFSET
        {
            // Post-DFT de-rotation by LO offset of the tone bin only
            // TODO: is the equivalent sample number correct?
            const float lo_offset_phase = (2*M_PI) * ((double)w->samplenumber - (w->fftlen - TONE_BIN)) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
            const complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
            w->dft_out[TONE_BIN] = w->dft_out[TONE_BIN] * lo_offset_phasor;
        }
        #endif

        samples += w->fftlen;
        ampsum += cabs(w->dft_out[TONE_BIN]);
        pcal += w->dft_out[TONE_BIN];

        //printf("%5d / %5zu\r", l, w->navg);
    }

    pcal /= (w->fftlen * w->navg);
    ampsum /= (w->fftlen * w->navg);

    w->amp = cabs(pcal);
    w->phase = carg(pcal) * 180.0/M_PI;
    w->coherence = cabs(pcal) / ampsum;
}

static void *worker_process_invoke(void *threadargs)
{
    worker_process((workerthread_t*)threadargs);
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double processRecordingByDFT(const char *infilename, int chan, int fftlen, int navg, FILE* out, int nthreads)
{
    struct mark5_stream *ms;
    struct mark5_format_generic *fmt;
    workerthread_t* workerthreads;

    int i, n, rc;
    int worker, block;
    float **multichanneldata;
    int endoffile = 0;

    int mjd, sec;
    double ns, scanStartMJD;
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
    multichanneldata = (float **)malloc(ms->nchan * sizeof(float **));
    for (i = 0; i < ms->nchan; i++) {
        multichanneldata[i] = (float *)malloc(fftlen*sizeof(float));
    }
    if (fftlen % ms->samplegranularity != 0) {
        fprintf(stderr, "WARNING -- decoding a nonstandard number of samples.  Expect bogus results\n");
    }

    // Init workers
    assert(nthreads >= 1);
    workerthreads = calloc(nthreads, sizeof(workerthread_t));
    for (worker=0; worker<nthreads; worker++) {
        worker_init(&workerthreads[worker], fftlen, navg);
    }

    // Process entire file
    while (!endoffile) {

        // Load data for next full averaging period(s)
        // + Dispatch worker threads
        for (worker=0; worker<nthreads; worker++) {
            float *dst;

            mark5_stream_get_frame_time(ms, &mjd, &sec, &ns);
            workerthreads[worker].APmidMJD = mjd + (sec + 0.5e-3*AP_msec + ns/1e9) / 86400.0;
            dst = workerthreads[worker].channeldata;

            for (block=0; block<navg; block++) {
                n = mark5_stream_decode(ms, fftlen, multichanneldata);
                if (n < 0) { // (n != fftlen) {
                    endoffile = 1;
                    printf("Short read, %d bytes. EOF.\n", n);
                    break;
                }
                memcpy(dst, multichanneldata[VDIF_CHAN_IDX], sizeof(float) * fftlen);
                dst += fftlen;
	        }
            workerthreads[worker].channeldata_valid = !endoffile;
            workerthreads[worker].samplenumber = samplecount;
            samplecount += fftlen * navg;

            // worker_process(&workerthreads[worker]);
            rc = pthread_create(&workerthreads[worker].threadid, NULL, &worker_process_invoke, &workerthreads[worker]);
            if (rc) {
                printf("Error launching thread nr %d/%d : %s\n", worker+1, nthreads, strerror(rc));
            }
            printf("Launched worker %d/%d\n", worker+1, nthreads);
        }

// TODO: producer-consumer with thread pool over MPI

        // Get the results
        for (worker=0; worker<nthreads; worker++) {
            const workerthread_t *w = &workerthreads[worker];

            pthread_join(w->threadid, NULL);
            printf("Joined worker %d/%d\n", worker+1, nthreads);

            if (!w->channeldata_valid) {
                continue;
            }

            memset(resultline, '\0', sizeof(resultline));
            snprintf(resultline, sizeof(resultline)-1,
                "%.9lf mjd : %10.6lf sec : %11.6e /_ %+8.3f deg  %.3f\n",
                w->APmidMJD, 1e-3*(APcount*AP_msec + AP_msec/2),
                w->amp, w->phase, w->coherence
            );
            printf(resultline);

            memset(resultline, '\0', sizeof(resultline));
            snprintf(resultline, sizeof(resultline)-1,
                "%.9lf %10.6lf %11.6e %+8.3f %.3f\n",
                w->APmidMJD, 1e-3*(APcount*AP_msec + AP_msec/2),
                w->amp, w->phase, w->coherence
            );
            fprintf(out, resultline);

            APcount++;
        }

    }

    // Tidy up
    for(i = 0; i < ms->nchan; i++) {
        free(multichanneldata[i]);
    }
    free(multichanneldata);
    delete_mark5_stream(ms);
    for (worker=0; worker<nthreads; worker++) {
        worker_deinit(&workerthreads[worker]);
    }
    free(workerthreads);

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
    processRecordingByDFT(argv[1], VDIF_CHAN_IDX, DFT_LENGTH, DFT_AVG, fout, NUM_WORKER_THREADS);

    fclose(fout);
    free(resultsfile);

    return 0;
}
