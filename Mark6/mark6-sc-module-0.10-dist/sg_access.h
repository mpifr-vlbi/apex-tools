/*
 * $Id: sg_access.h 2209 2014-06-19 20:39:59Z gbc $
 *
 * Code to understand and access sg files efficiently.
 */

#ifndef sg_access_h
#define sg_access_h

#include <stdint.h>
#include "vdif.h"

/*
 * A structure to hold mmap-d info while the file is open
 */
typedef struct sg_mmap_info {
    off_t       size;           /* size of the file in bytes */
    void        *start;         /* starting offset, non-zero if in use */
    void        *eomem;         /* end of valid memory, non-zero if in use */
    int         mmfd;           /* file descriptor >=0 if in use */
} SGMMInfo; 

/*
 * A structure to hold what we can easily find out about an SG fragment
 */
typedef struct sg_info {
    /* general support */
    SGMMInfo    smi;            /* memory mapping information */
    double      eval_time;      /* diagnostic on file access time */
    int         verbose;        /* diagnostic on internal processing */
    char        *name;          /* a malloc'd copy of the file name */

    /* vdif general */
    uint32_t    total_pkts;     /* total number of packets */
    uint32_t    pkt_size;       /* VDIF packet size */
    uint32_t    pkt_offset;     /* offset into packet */
    uint32_t    read_size;      /* total packet + overhead */
    uint32_t    ref_epoch;      /* reference epoch */
    uint32_t    first_secs;     /* seconds of epoch of first packet */
    uint32_t    first_frame;    /* frame counter of first packet */
    uint32_t    final_secs;     /* seconds of epoch of final packet */
    uint32_t    final_frame;    /* frame counter of final packet */
    uint32_t    frame_cnt_max;  /* maximum frame counter value seen */
    VDIFsigu    vdif_signature; /* header signature */

    /* sg version 2 */
    int32_t     sg_version;
    uint32_t    sg_fht_size;    /* size of file header tag */
    uint32_t    sg_wbht_size;   /* write block header tag size */
    uint32_t    sg_wr_block;    /* standard write block size */
    uint32_t    sg_wr_pkts;     /* pkts in standard write block */
    uint32_t    sg_wr_blks_bs;  /* number of write blocks before sb */
    uint32_t    sg_wr_blks_as;  /* number of write blocks after sb */
    uint32_t    sg_wr_pkts_bs;  /* pkts in normal wbs before sb */
    uint32_t    sg_wr_pkts_as;  /* pkts in normal wbs after sb */
    uint32_t    sg_sh_block;    /* starting write block size */
    off_t       sg_sh_blk_off;  /* offset in the file */
    uint32_t    sg_sh_pkts;     /* pkts in starting write block */
    uint32_t    sg_se_block;    /* ending write block size */
    off_t       sg_se_blk_off;  /* offset in the file */
    uint32_t    sg_se_pkts;     /* pkts in ending write block */
    uint32_t    sg_total_blks;  /* total number of blocks */
} SGInfo;

/* Magic number of SG */
#define SG_VERSION_MAGIC    0xfeed6666
#define SG_MAX_VDIF_BYTES   32000
#define SG_MIN_VDIF_BYTES   64

/* positive values >2 for sg_version are scatter-gather versions */
#define SG_VERSION_OK_2             2
/* unstructured files of VDIF packets describable with the above */
#define SG_VERSION_FLAT             1
/* none of the above */
#define SG_VERSION_NOT              0
/* negative values are for errors encountered during SG analysis */
#define SG_VERSION_NOT_ENOUGH_DATA -1
#define SG_VERSION_SIG_FIRST_FAIL  -2
#define SG_VERSION_SIG_FINAL_FAIL  -3
#define SG_VERSION_SIG_SEBLK_FAIL  -4
#define SG_VERSION_SIG_SEBLK_BAIL1 -5
#define SG_VERSION_SIG_SEBLK_BAIL2 -6
#define SG_VERSION_SIG_SEBLK_BAIL3 -7
#define SG_VERSION_SIG_SHBLK_FAIL1 -8
#define SG_VERSION_SIG_SHBLK_FAIL2 -9
#define SG_VERSION_SIG_SHBLK_BAIL  -10
/* returns a short string explaining the above */
extern char *sg_error_str(int err);

/*
 * Methods to open an SG file for use.
 *   sg_access()    opens the file, understands it, and closes it
 *   sg_open()      opens the file, and if sgi is non-null, and calls
 *   sg_info()      to understand the file opened in smi
 *   sg_close()     closes a file opened with sg_open()
 *   sg_reopen()    assumes sgi is valid and opens sgi->name
 *
 * The SGMMInfo pointer returned is to sgi->smi or a NULL pointer.
 */
extern void sg_access(const char *file, SGInfo *sgi);
extern void sg_info(const char *file, SGInfo *sgi);
extern SGMMInfo *sg_open(const char *file, SGInfo *sgi);
extern SGMMInfo *sg_reopen(SGInfo *sgi);
extern void sg_close(SGMMInfo *smi);

/*
 * Random access methods to the packets in the file.
 * Each returns a pointer to the requested packet and if end is not NULL,
 * sets it to the end of the block.  Likewise if nl is not NULL, it will
 * contain the number of packets left in the block,
 * In the event of exceptional conditions, NULL32P is returned.
 */
#define NULL32P ((uint32_t *)0)
extern uint32_t *sg_pkt_by_num(SGInfo *sgi, off_t np, int *nl, uint32_t **end);
extern uint32_t *sg_pkt_by_blk(SGInfo *sgi, off_t nb, int *nl, uint32_t **end);
extern uint32_t *sg_pkt_by_off(SGInfo *sgi, off_t of, int *nl, uint32_t **end);

/*
 * a variant of sg_pkt_by_blk that also returns (packet) byte counts
 * before and after the block, but not including the block
 */
extern uint32_t *sg_pkt_blkby(SGInfo *sgi,
    off_t nb, int *nl, off_t *pktbytesbefore, off_t *pktbytesafter);

/*
 * A diagnostic method to check the signatures on packets found by the above.
 * It returns the number of packets failing the signature check (0 through nl,
 * inclusive), or the end-of-block check.
 */
extern int sg_pkt_check(SGInfo *sgi, uint32_t *pkt, int nl, uint32_t *end);
extern int sg_pkt_times(SGInfo *sgi, uint32_t *pkt, int nl, uint32_t *end);

/*
 * A diagnostic method to describe the SGInfo contents: stdout or buffer
 * If label is not NULL, it is inserted at the beginning of each line.
 */
extern void sg_report(SGInfo *sgi, char *label);
extern char *sg_repstr(SGInfo *sgi, char *label);

/* Computes the vdif header signature from a pointer to 1st header word */
extern uint64_t sg_get_vsig(uint32_t *vhp, void *orig, int verb);

/* Returns vextime for seconds in a ref epoch */
extern char *sg_vextime(int re, int secs);

#endif /* sg_access_h */

/*
 * eof
 */
