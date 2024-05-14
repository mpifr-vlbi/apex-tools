#ifndef M5TONE_HARDCODED_CONFS_H
#define M5TONE_HARDCODED_CONFS_H

/** Baseband */
#ifdef STATION_APEX 
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    426     /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     -15.022 /* Offset of 1st LO to compensate for */
    #define HAVE_LO_OFFSET   1       /* 1: LO offset present and should be removed, 0: disable removal of LO offset */
    #define USE_FAST_LO_OFFSET 1     /* 1: use approximation, de-rotate tone in bin after DFT, faster, 0: no approximation, de-rotate the entire sample stream */
#endif
#ifdef STATION_APEX_2022_VDI // BAND 1 tone, 1st LO was a VDI synth that has an LO offset
    #define VDIF_CHAN_IDX    0       /* VDIF channel that has the tone; 0-based indexing */
    #define VDIF_CHAN_BW_MHZ 2048    /* Baseband signal bandwidth in MHz */
    #define TONE_FREQ_MHZ    798.0   /* Baseband tone freq in MHz */
    #define LO_OFFSET_HZ     +15.022 /* Offset of 1st LO to compensate for */
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

#endif // M5TONE_HARDCODED_CONFS_H

