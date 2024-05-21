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

/*
Test:

rm -f m5tone_mpi_eht ; mpicxx -pthread -Wall -g -O3 -ffast-math -fno-math-errno -m64 -march=native -mtune=native m5tone_mpi_eht.cpp -DSTATION_APEX_2022_VDI -I/cluster/difx/DiFX-281/include \
  -o m5tone_mpi_eht -L/cluster/difx/DiFX-281/lib -lmark5access -lmark6sg -lmark6gather -lcodifio -lfftw3f -lm

*/

#include <assert.h>
#include <complex.h>
#include <malloc.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <mark5access.h>
#include <fftw3.h>

#include <mpi.h>

#include "m5tone_hardcoded_confs.h"

void usage()
{
    printf("Usage: m5tone <vdif file>\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
    #undef DFT_AVG
    //#define DFT_AVG  4000       /* 400 msec == EHT 2018 visibility data AP of 0.4s */
    //#define DFT_AVG  1000       /* 1000 DFTs : 1000 blocks x 409600-point DFT / 4096Msps = 100 millisec */
    #define DFT_AVG  100        /* 10 msec */
    //#define DFT_AVG  10         /* 1 msec */
#endif

//#define RECODE_BEFORE_MPISEND

/** Derived settings */
#define VDIF_CHAN_FS_HZ ( (double)(2e6*VDIF_CHAN_BW_MHZ) )
#define TONE_BIN        ( (size_t)(((double)TONE_FREQ_MHZ*DFT_LENGTH)/(2.0*VDIF_CHAN_BW_MHZ)) )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct computeconfig_tt {

    uint64_t fftlen;
    uint64_t navg;

} computeconfig_t;

typedef struct computeinput_tt {

    int channeldata_valid;
    int sequencenumber;
    uint64_t samplenumber;
    double APmidMJD;
    float *channeldata;

    MPI_Request mpirequest;
    int mpirequestcount;
    int eof;

} computeinput_t;

typedef struct computevars_tt {

    double APmidMJD;
    double amp, phase, coherence;

    fftwf_complex *dft_in, *dft_out;
    fftwf_plan pf;

} computevars_t;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int samplestream_producer(MPI_Comm mpi, const char* vdiffilename);
int observables_computor(MPI_Comm mpi);

void process_sample_data(const computeinput_t& in, const computeconfig_t& cfg, computevars_t *out);
void write_log_entry(FILE *out, double datasec, const computevars_t& data);

void recode(float* data, size_t nsamples);
void decode(unsigned char* input, float* output, size_t nsamples);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int samplestream_producer(MPI_Comm mpi, const char* vdiffilename)
{
    char *logfilename;
    FILE *fout;

    computeinput_t* computeblocks;
    float **multichanneldata;
    int endoffile = 0;

    double AP_len_msec;
    size_t samplecount = 0;
    int sequencenumber = 0;

    // MPI layout
    int rank, mpisize, computesize, rankoffset;
    MPI_Comm_rank(mpi, &rank);
    MPI_Comm_size(mpi, &mpisize);
    rankoffset = 1; // rank 0: vdif sender, rank 1..n vdif recipients
    computesize = mpisize - rankoffset;
    assert(rank == 0);
    assert(computesize >= 1);

    // Share config over MPI
    computeconfig_t cfg;
    cfg.fftlen = DFT_LENGTH;
    cfg.navg = DFT_AVG;
    printf("samplestream_producer() rank=%d - broadcast config for %lu x %lu\n", rank, cfg.fftlen, cfg.navg);
    MPI_Bcast(&cfg.fftlen, 1, MPI_UINT64_T, 0, mpi);
    MPI_Bcast(&cfg.navg, 1, MPI_UINT64_T, 0, mpi);
    AP_len_msec = ((1e3*DFT_LENGTH)*(double)DFT_AVG) / VDIF_CHAN_FS_HZ;

    // Open VDIF file
    mark5_library_init();
    struct mark5_format_generic *fmt = new_mark5_format_vdif(8192, 1, 2, 1, 8192, 32, 0);
    struct mark5_stream *ms = new_mark5_stream(new_mark5_stream_file(vdiffilename,0), fmt);
    if (!ms) {
        fprintf(stderr, "Problem creating mark5_stream\n");
        return 0;
    }

    int mjd, sec;
    double ns;
    mark5_stream_get_frame_time(ms, &mjd, &sec, &ns);
    double scanStartMJD = mjd + (sec + ns/1e9) / 86400.0;

    // Create output file
    const char* pathless_name = strrchr(vdiffilename, '/');
    if (pathless_name == NULL) {
        pathless_name = vdiffilename;
    }
    if (pathless_name[0] == '/') {
        pathless_name++;
	    }

    logfilename = (char*)malloc(strlen(pathless_name) + strlen(".m5t") + 1);
    sprintf(logfilename, "%s.m5t", pathless_name);
    printf("output file = %s\n", logfilename);

    fout = fopen(logfilename, "w");
    if (fout == NULL) {
        printf("Error");
    }
    setvbuf(fout, (char *)NULL, _IONBF, 0);

    // Summary of settings
    printf("Starting MJD: %.9lf\n", scanStartMJD);
    printf("Hardcoded settings: %d-point FFT, tone at %.1f MHz, LO offset %.3f Hz that %s to be removed\n",
        DFT_LENGTH, (float)TONE_FREQ_MHZ, (float)LO_OFFSET_HZ,
        (HAVE_LO_OFFSET==1) ? "is" : "is NOT"
    );
    printf("Tone at %.3f kHz in the %.3f MHz wide band lands in %.3f kHz-wide bin %d.\n",
        (float)(1e3*TONE_FREQ_MHZ), (float)VDIF_CHAN_BW_MHZ, ((float)VDIF_CHAN_BW_MHZ*2e3)/DFT_LENGTH, (int)TONE_BIN
    );
    printf("Integrating for %.2f milliseconds with %zu spectra per integration.\n", AP_len_msec, cfg.navg);

    // Allocate decoder output arrays and data transmit arrays
    multichanneldata = (float **)malloc(ms->nchan * sizeof(float **));
    for (int i = 0; i < ms->nchan; i++) {
        multichanneldata[i] = (float *)malloc(cfg.fftlen*sizeof(float));
    }
    if (cfg.fftlen % ms->samplegranularity != 0) {
        fprintf(stderr, "WARNING -- decoding a nonstandard number of samples.  Expect bogus results\n");
    }
    computeblocks = (computeinput_t*)calloc(computesize, sizeof(computeinput_t));
    for (int i = 0; i < computesize; i++) {
        computeblocks[i].channeldata = (float *)malloc(cfg.fftlen*(cfg.navg+1)*sizeof(float));
        computeblocks[i].mpirequestcount = 0;
        computeblocks[i].eof = 0;
    }

    // Begin sending out data chunks to the compute processes
    int prev_target = 0;
    int pending_results_count = 0;
    while (!endoffile || (endoffile && pending_results_count > 0)) {

        float *dst;
        int recipient_rank = 0;
        const int tag = 0;

        // printf("samplestream_producer() rank=%d - prev target %d, endoffile=%d pending_results_count=%d, looking for next slot\n", rank, prev_target, endoffile, pending_results_count);

        int target = -1;
        for (int i = 0; i < computesize; i++) {
            int ready = 0;
            int candidate = (prev_target + i) % computesize;
            if (computeblocks[candidate].mpirequestcount > 0) {
                MPI_Test(&computeblocks[candidate].mpirequest, &ready, MPI_STATUS_IGNORE);
                if (ready) {
                    target = candidate;
                    break;
                }
            } else if (!endoffile) {
                target = candidate;
                break;
            }
        }

        if (target < 0) {
            prev_target = (prev_target + 1) % computesize;
            // printf("samplestream_producer() rank=%d - no slot found, upping prev_target by +1\n", rank);
            continue;
        }
        assert (target < computesize);

        // MPI World rank of selected node
        recipient_rank = target + rankoffset;

        // Collect results of previous data block from the node
        if (computeblocks[target].mpirequestcount > 0) {
            computevars_t pcalresult;
            MPI_Recv(&pcalresult.APmidMJD, 1, MPI_DOUBLE, recipient_rank, tag, mpi, MPI_STATUS_IGNORE);
            MPI_Recv(&pcalresult.amp, 1, MPI_DOUBLE, recipient_rank, tag, mpi, MPI_STATUS_IGNORE);
            MPI_Recv(&pcalresult.phase, 1, MPI_DOUBLE, recipient_rank, tag, mpi, MPI_STATUS_IGNORE);
            MPI_Recv(&pcalresult.coherence, 1, MPI_DOUBLE, recipient_rank, tag, mpi, MPI_STATUS_IGNORE);
            pending_results_count--;

            if (computeblocks[target].channeldata_valid) {
                double datasec = 1e-3 * AP_len_msec * (computeblocks[target].sequencenumber + 0.5);
                write_log_entry(fout, datasec, pcalresult);
            }

            printf("samplestream_producer() rank=%d - collected results from compute process %d with rank %d\n", rank, target, recipient_rank);
        }

        // At EOF, get only outstanding results, do not attempt to send more data
        if (computeblocks[target].eof) {
            computeblocks[target].mpirequestcount = 0;
            prev_target = target;
            continue;
        }

        // Send new data block to the target node
        mark5_stream_get_frame_time(ms, &mjd, &sec, &ns);
        dst = computeblocks[target].channeldata;
        for (size_t n = 0; n < cfg.navg && !endoffile; n++) {
            int rc = mark5_stream_decode(ms, cfg.fftlen, multichanneldata);
            if (rc < 0) {
                endoffile = 1;
                printf("samplestream_producer() rank=%d - input file EOF (rc=%d)\n", rank, rc);
                break;
            }
            memcpy(dst, multichanneldata[VDIF_CHAN_IDX], sizeof(float) * cfg.fftlen);
            dst += cfg.fftlen;
        }

        computeblocks[target].eof = endoffile;
        computeblocks[target].channeldata_valid = !endoffile;
        computeblocks[target].sequencenumber = sequencenumber;
        computeblocks[target].samplenumber = samplecount;
        computeblocks[target].APmidMJD = mjd + (sec + 0.5e-3*AP_len_msec + ns/1e9) / 86400.0;
        samplecount += cfg.fftlen * cfg.navg;

        recipient_rank = target + rankoffset;
        MPI_Irsend(&computeblocks[target].channeldata_valid, 1, MPI_INT, recipient_rank, tag, mpi, &computeblocks[target].mpirequest);
        MPI_Irsend(&computeblocks[target].sequencenumber, 1, MPI_INT, recipient_rank, tag, mpi, &computeblocks[target].mpirequest);
        MPI_Irsend(&computeblocks[target].samplenumber, 1, MPI_UINT64_T, recipient_rank, tag, mpi, &computeblocks[target].mpirequest);
        MPI_Irsend(&computeblocks[target].APmidMJD, 1, MPI_DOUBLE, recipient_rank, tag, mpi, &computeblocks[target].mpirequest);
#ifndef RECODE_BEFORE_MPISEND
        MPI_Irsend(computeblocks[target].channeldata, cfg.fftlen * cfg.navg, MPI_FLOAT, recipient_rank, tag, mpi, &computeblocks[target].mpirequest);
#else
        recode(computeblocks[target].channeldata, cfg.fftlen * cfg.navg);
        MPI_Irsend(computeblocks[target].channeldata, cfg.fftlen * cfg.navg, MPI_UNSIGNED_CHAR, recipient_rank, tag, mpi, &computeblocks[target].mpirequest);
#endif
        computeblocks[target].mpirequestcount++;
        pending_results_count++;

        printf("samplestream_producer() rank=%d - dispatched data block %d to compute process %d with rank %d\n", rank, sequencenumber, target, recipient_rank);

        prev_target = target;
        sequencenumber++;
    }


    // Tidy up
    fclose(fout);
    free(logfilename);

    printf("samplestream_producer() rank=%d - bye!\n", rank);

    return 0;
}

int observables_computor(MPI_Comm mpi)
{
    computeconfig_t cfg;
    computeinput_t in;
    computevars_t out;
    unsigned char* rawdata;

    const int datasource_rank = 0; // rank 0 is always the sample data producer in this program, ranks >= 1 are compute processes
    const int tag = 0;
    int rank;

    MPI_Comm_rank(mpi, &rank);

    printf("observables_computor()  rank=%d - hi!\n", rank);

    MPI_Bcast(&cfg.fftlen, 1, MPI_UINT64_T, 0, mpi);
    MPI_Bcast(&cfg.navg, 1, MPI_UINT64_T, 0, mpi);

    printf("observables_computor()  rank=%d - received config for %lu x %lu\n", rank, cfg.fftlen, cfg.navg);

    in.channeldata = (float *)malloc(sizeof(float) * cfg.fftlen * (cfg.navg+1));
    out.dft_in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * cfg.fftlen);
    out.dft_out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * cfg.fftlen);
    out.pf = fftwf_plan_dft_1d(cfg.fftlen, out.dft_in, out.dft_out, FFTW_FORWARD, FFTW_ESTIMATE);
    rawdata = (unsigned char*)malloc(cfg.fftlen * (cfg.navg+1));

    printf("observables_computor()  rank=%d - allocated for %lu x %lu\n", rank, cfg.fftlen, cfg.navg);

    do {
        MPI_Recv(&in.channeldata_valid, 1, MPI_INT, datasource_rank, tag, mpi, MPI_STATUS_IGNORE);
        MPI_Recv(&in.sequencenumber, 1, MPI_INT, datasource_rank, tag, mpi, MPI_STATUS_IGNORE);
        MPI_Recv(&in.samplenumber, 1, MPI_UINT64_T, datasource_rank, tag, mpi, MPI_STATUS_IGNORE);
        MPI_Recv(&in.APmidMJD, 1, MPI_DOUBLE, datasource_rank, tag, mpi, MPI_STATUS_IGNORE);
#ifndef RECODE_BEFORE_MPISEND
        MPI_Recv(in.channeldata, cfg.fftlen * cfg.navg, MPI_FLOAT, datasource_rank, tag, mpi, MPI_STATUS_IGNORE);
        // TODO: if network is a bottleneck: convert from float back to 2-bit for MPI_Send, then unpack again to 32-bit float after MPI_Recv, to save 16x of bandwidth
#else
        MPI_Recv(rawdata, cfg.fftlen * cfg.navg, MPI_UNSIGNED_CHAR, datasource_rank, tag, mpi, MPI_STATUS_IGNORE);
        decode(rawdata, in.channeldata, cfg.fftlen * cfg.navg);
#endif

        printf("observables_computor()  rank=%d - got data for seq %d MJD %.9lf for %lu x %lu\n", rank, in.sequencenumber, in.APmidMJD, cfg.fftlen, cfg.navg);

        process_sample_data(in, cfg, &out);

        printf("observables_computor()  rank=%d - finished processing data chunk - seq %d MJD %.9lf amp %11.6e %+8.3lf deg %.3lf\n", rank, in.sequencenumber, out.APmidMJD, out.amp, out.phase, out.coherence);

        MPI_Send(&out.APmidMJD, 1, MPI_DOUBLE, datasource_rank, tag, mpi);
        MPI_Send(&out.amp, 1, MPI_DOUBLE, datasource_rank, tag, mpi);
        MPI_Send(&out.phase, 1, MPI_DOUBLE, datasource_rank, tag, mpi);
        MPI_Send(&out.coherence, 1, MPI_DOUBLE, datasource_rank, tag, mpi);

    } while (in.channeldata_valid);

    free(in.channeldata);
    fftwf_destroy_plan(out.pf);
    fftwf_free(out.dft_in);
    fftwf_free(out.dft_out);

    printf("observables_computor()  rank=%d - bye!\n", rank);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void process_sample_data(const computeinput_t& in, const computeconfig_t& cfg, computevars_t *out)
{
    float _Complex pcal = 0.0f + 0.0I;
    double ampsum = 0;

    float *samples = in.channeldata;
    uint64_t samplenumber = in.samplenumber;

    if (!in.channeldata_valid) {
        out->APmidMJD = 0;
        out->amp = 0;
        out->phase = 0;
        out->coherence = 0;
        return;
    }

#if 0  // speed up MPI debug by omitting compute
    out->APmidMJD = in.APmidMJD;
    out->amp = 1.0;
    out->phase = 23.0;
    out->coherence = 0.45;
    return;
#endif

    for (size_t l = 0; l < cfg.navg; l++) {

        #if HAVE_LO_OFFSET && !USE_FAST_LO_OFFSET
        for (size_t i = 0; i < cfg.fftlen; i++) {
            // De-rotate the whole time domain sample stream by the LO offset
            const float lo_offset_phase = (2*M_PI) * ((double)samplenumber) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
            const float _Complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
            out->dft_in[i] = samples[i] * lo_offset_phasor;
            samplenumber++;
        }
        #endif

        #if !HAVE_LO_OFFSET || USE_FAST_LO_OFFSET
        for (size_t i = 0; i < cfg.fftlen; i++) {
            // No time domain de-rotation, do it after DFT
            out->dft_in[i][0] = samples[i];
            out->dft_in[i][1] = 0.0f;
        }
        samplenumber += cfg.fftlen;
        #endif

        fftwf_execute(out->pf);

        #if HAVE_LO_OFFSET && USE_FAST_LO_OFFSET
        {
            // Post-DFT de-rotation by LO offset of the tone bin only
            // TODO: is the equivalent sample number correct?
            const float lo_offset_phase = (2*M_PI) * ((double)samplenumber - (cfg.fftlen - TONE_BIN)) * LO_OFFSET_HZ / VDIF_CHAN_FS_HZ;
            const float _Complex lo_offset_phasor = cosf(lo_offset_phase) + _Complex_I*sinf(lo_offset_phase);
            const float _Complex xn = { out->dft_out[TONE_BIN][0], out->dft_out[TONE_BIN][1] };
            const float _Complex adjusted = xn * lo_offset_phasor;
            out->dft_out[TONE_BIN][0] = creal(adjusted);
            out->dft_out[TONE_BIN][1] = cimag(adjusted);
        }
        #endif

        samples += cfg.fftlen;
        //ampsum += cabsf(out->dft_out[TONE_BIN]);
        //pcal += out->dft_out[TONE_BIN];
        const float _Complex tonebin = { out->dft_out[TONE_BIN][0], out->dft_out[TONE_BIN][1] };
        ampsum += cabsf(tonebin);
        pcal += tonebin;

        //printf("%5d / %5zu\r", l, cfg.navg);
    }

    pcal /= (cfg.fftlen * cfg.navg);
    ampsum /= (cfg.fftlen * cfg.navg);

    out->APmidMJD = in.APmidMJD;
    out->amp = cabsf(pcal);
    out->phase = cargf(pcal) * 180.0/M_PI;
    out->coherence = cabsf(pcal) / ampsum;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void write_log_entry(FILE *out, double datasec, const computevars_t& data)
{
    char resultline[200];
    memset(resultline, '\0', sizeof(resultline));
    snprintf(resultline, sizeof(resultline)-1,
        "%.9lf %10.6lf %11.6e %+8.3f %.3f\n",
        data.APmidMJD, datasec,
        data.amp, data.phase, data.coherence
    );
    fprintf(out, resultline);
    fflush(out);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void recode(float* data, size_t nsamples)
{
    // Inplace recode from 32-bit 4-value to 8-bit 5-value
    unsigned char* out = (unsigned char*)data;
    uint32_t* in = (uint32_t*)data;
    for (size_t n=0; n<nsamples; n++) {
        uint32_t v32 = in[n];
        unsigned char v8 = 0;
        if (v32 == 0xc0557f63) { v8=1; }
        else if (v32 == 0x40557f63) { v8=2; }
        else if (v32 == 0x3f800000) { v8=3; }
        else if (v32 == 0xbf800000) { v8=4; }
        out[n] = v8;
    }
}

void decode(unsigned char* in, float* out, size_t nsamples)
{
    const uint32_t table[8] = {0, 0xc0557f63, 0x40557f63, 0x3f800000, 0xbf800000, 0, 0, 0};
    uint32_t* out32 = (uint32_t*)out;
    for (size_t n=0; n<nsamples; n++) {
        out32[n] = table[in[n] & 0x07];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    int rank, mpisize;

    // MPI init
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpisize);

    // User arg(s)
    if (argc != 2) {
        usage();
        exit(1);
    }

    // MPI processing
    if (mpisize <= 1) {
        printf("Error: need more than %d nodes in mpirun\n", mpisize);
    } else {
        if (rank == 0) {
            samplestream_producer(MPI_COMM_WORLD, argv[1]);
        } else {
            observables_computor(MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();

    return 0;
}
