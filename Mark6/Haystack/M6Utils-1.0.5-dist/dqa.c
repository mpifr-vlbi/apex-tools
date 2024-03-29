// dqa - data quality analyzer for captured vdif packets
//
// initial version                  2012.6.16  rjc
// support version 2 block format   2013.4.24  rjc
// add de-threading feature         2013.9.27  rjc

#include "../d-plane/dplane.h"

#define TWO31_1 2147483647
#define BMAX 100000
#define MAXT 8

int main (int argc, 
           char **argv)
    {
    int i,
        j,
        n,
        rc,
        nfr,
        thr,
        sec,
        check[MAXT]  = {0, 0, 0, 0, 0, 0, 0, 0}, // flag to perform continuity tests
        noos[MAXT]   = {0, 0, 0, 0, 0, 0, 0, 0}, // # packets out of sequence per thread
        np[MAXT]     = {0, 0, 0, 0, 0, 0, 0, 0}, // # packets read per thread
        nsjump[MAXT] = {0, 0, 0, 0, 0, 0, 0, 0}, // # second jumps per thread
        nfill[MAXT]  = {0, 0, 0, 0, 0, 0, 0, 0}, // # fill-frame packets
        first_sec,
        first_frame,
        last_pkt[MAXT],             // last packet # by thread
        last_sec[MAXT],             // last packet's time field by thread
        total_pkts,
        payload_size,               // length of vdif packets
        block_size,                 // size of contiguous blocks in scatter mode
        npkt_block,
        version,
        scattered,
        n_within_block = 0,
        max_frame_num = 0,
        hdr_size,
        nptot = 0,
        dethread = FALSE,
        opened[MAXT] = {FALSE, FALSE, FALSE, FALSE},
        thread[MAXT],               // actual thread by index (up to nthreads)
        nthreads = 0,
        tid;

    unsigned int *pint,
                 *pubuff;

    long int total_bytes = 0;

    char buffer[BMAX],
         infile[128],
         outfile[MAXT][128];

    FILE *fin,
         *fout[MAXT];

    vdif_header *vdh;

    vdh = (vdif_header *) buffer;
    pubuff = (unsigned int *) buffer;

    if (argc != 2 && argc != 3)
        {
        printf ("Usage: dqa <file_name> to get quality summary\n");
        printf (" ~or~  dqa <file_name> <#packets> for detailed report\n");
        printf (" ~or~  dqa -d <file_name> to also de-thread the file\n");
        exit (1);
        }
                                    // limit #packets as per request
    if (argc == 3)
        {
        if (strcmp ("-d", argv[1])) // not dethread mode - copy input file name
            {
            strncpy (infile, argv[1], 128);
            total_pkts = atoi (argv[2]);
            }
        else                        // dethread mode; create file names for later use
            {
            dethread = TRUE;
            total_pkts = TWO31_1;   // 2^31 - 1  (i.e. read all)
            strncpy (infile, argv[2], 128);
            printf ("dethreading to files:\n");
            for (i=0; i<MAXT; i++)
                {
                strcpy (outfile[i], infile);
                j = strlen (outfile[i]);
                while (outfile[i][j] != '.' && j > 0)
                    j--;
                if (j == 0)         // if no .suffix, then just append to original
                    j = strlen (outfile[i]);
                                    // append thread number 0..3
                                    // e.g. - change name.vdif to name_2.vdif
                sprintf (outfile[i]+j, "_%d", i);
                strcpy (outfile[i]+j+2, infile+j);
                printf ("%s\n", outfile[i]);
                }
            }
        }
    else
        {
        total_pkts = TWO31_1;       // 2^31 - 1  (i.e. read all)
        strncpy (infile, argv[1], 128);
        }

    printf ("opening %s\n", infile);
    fin = fopen (infile, "r");

    if (fin == NULL)
        {
        perror ("dqa");
        printf ("can't open file %s\n", infile);
        exit (1);
        }
                                    // find out format, record size, etc.
    rc = fread (buffer, 56, 1, fin);
    if (rc != 1)
        {
        printf ("Problem reading %s\n", infile);
        perror (NULL);
        exit (1);
        }
    rewind (fin);                   // reposition to file start

                                    // is it a scatter file?
    if (*(unsigned int *) buffer == SYNC_WORD)
        {
        if (dethread)
            {
            printf ("can't dethread a scatter file, quitting.\n");
            exit (1);
            }
        scattered = TRUE;
        version = *((unsigned int *) (buffer + 4));
        switch (version)
            {
            case 1:
                pint = (unsigned int *) (buffer + 32);
                hdr_size = 4;
                break;

            case 2:
                pint = (unsigned int *) (buffer + 36);
                hdr_size = 8;
                break;

            default:
                printf ("Undefined version %d, quitting.\n", version);
                exit (1);
            }

        payload_size = 8 * (*pint & 0x00ffffff);

        block_size = *((unsigned int *) (buffer + 8));
        npkt_block = block_size / payload_size;
                                    // sanity test for integral payloads per block
        if (npkt_block * payload_size != block_size - hdr_size)
            {
            printf ("sg mode - fatal block size error! payload %d block %d npkts %d\n",
                    payload_size, block_size, npkt_block);
            exit (1);
            }
        printf ("scatter mode file version %d block_size %d pkts/block %d\n",
                    version, block_size, npkt_block);

                                    // skip over header block
        rc = fread (buffer, 20, 1, fin);
        total_bytes = 20;
                                    // force read of first block's header
        n_within_block = npkt_block;          
        }
    else                            // no, just plain vdif format
        {
        scattered = FALSE;
        pint = (unsigned int *) (buffer + 8);
        payload_size = 8 * (*pint & 0x00ffffff);
        npkt_block = TWO31_1;       // no blocking - don't suppress checks
        printf ("vdif file\n");
        }
    printf ("packet payload size %d\n", payload_size);
                                    // ensure that read isn't too large
    if (payload_size > BMAX)
        {
        printf ("ERROR - payload too large for buffer, likely file problem\n");
        exit (1);
        }
    

                                    // loop over all packets in file
    for (n=0; n<total_pkts; n++)
        {
                                    // iff scattered format, skip over block #
        if (scattered && n_within_block == npkt_block)          
            {
            rc = fread (buffer, hdr_size, 1, fin);
            // printf("n %d rc %d\n",n,rc);
            if (rc == 1)
                {
                if (argc == 3)
                    printf ("read block %d packet %d at %lx size %x\n", 
                            pubuff[0], n, total_bytes, pubuff[1]);
                total_bytes += hdr_size;
                npkt_block = (pubuff[1] - 8) / payload_size;
                n_within_block = 0;
                }
            }
                                    // read another packet
        rc = fread (buffer, payload_size, 1, fin);

        if (rc != 1)                // error or eof detected
            {
            if (feof (fin))
                break;              // quit loop on eof
            else
                {
                perror (NULL);
                printf ("quitting due to error reading file %s. %d packets read\n",
                        infile, n);
                exit (1);
                }
            }

                                    // convenience variables
        nfr = vdh->frame;
        sec = vdh->seconds;
        tid = vdh->threadid;
                                    // find tid in threads table, or add it
        for (i=0; i<nthreads; i++)
            if (tid == thread[i])
                break;
                          
        if (i<nthreads)             // in table, use indirect link
            thr = i;
        else if (nthreads == MAXT)     // not in table, max# of threads, complain
            {
            if (argc == 3)          // print only for detailed report
                {
                printf ("packet %d thread ID %d max threads exceeded at %lx\n",
                        nptot, tid, total_bytes);
                printf ("%08x %08x %08x %08x %08x\n",
                        pubuff[0], pubuff[1], pubuff[2], pubuff[3], pubuff[4]);
                }
            thr = nthreads - 1;     // throw into last bin
            }
        else                        // not in table, add this thread to it
            {
            thread[nthreads] = tid;
            thr = nthreads++;
            }


                                    // detect, tally, and skip over fill-frame packets
                                    // note that f-f can happen in 1st or 2nd half
        if (vdh->invalid || pubuff[8] == MK5B_FILL || pubuff[1800] == MK5B_FILL)
            {
            nfill[thr]++;
            last_pkt[thr]++;
            n_within_block++;
            nptot++;
            total_bytes += payload_size;
            check[thr] = FALSE;     // suppress checking after missing packets
            continue;
            }

                                    // store away the starting point
        if (n == 0)
            {
            first_sec = sec;
            first_frame = nfr;
            }

                                    // suppress checking on block rollover
        if (n_within_block == 0)
            for (i=0; i<MAXT; i++)
                check[i] = FALSE;

                                    // check VDIF packet header for increment or rollover
        if (check[thr])             // don't check very first, or on block boundary
            {
            if (nfr != last_pkt[thr]+1 
            && (nfr != 0 || sec != last_sec[thr]+1))
                {
                if (argc == 3)
                    {
                    printf ("out of seq pkt %6d in thr %d fr# expect %x got %x at %lx\n", 
                            n, thr, last_pkt[thr]+1, nfr, total_bytes);
                    printf ("%08x %08x %08x %08x %08x\n",
                            pubuff[0], pubuff[1], pubuff[2], pubuff[3], pubuff[4]);
                    }
                noos[thr]++;
                }
            if (sec != last_sec[thr] && sec != last_sec[thr]+1)
                {
                if (argc == 3)
                    printf ("second jump from %d to %d for thread %d\n",
                            last_sec[thr], sec, thr);
                nsjump[thr]++;
                }
            }
        else                        // re-enable checking for this thread
            check[thr] = TRUE;
                                    // store packet# and time
        last_pkt[thr] = nfr;
        last_sec[thr] = sec;
        if (nfr > max_frame_num)
            max_frame_num = nfr;
                                    // check body of VDIF packet (NYI)
                                    // do dethreading, if indicated
        if (dethread)
            {
            if (!opened[thr])
                {
                fout[thr] = fopen (outfile[thr], "w");
                if (fout[thr] == NULL)
                    {
                    perror ("dqa");
                    printf ("error opening %s, quitting.\n", outfile[thr]);
                    exit (1);
                    }
                opened[thr] = TRUE;
                printf ("opened %s for writing\n", outfile[thr]);
                }
            rc = fwrite (buffer, payload_size, 1, fout[thr]);
            if (rc != 1)
                {
                perror ("dqa");
                printf ("error writing to %s, quitting.\n", outfile[thr]);
                exit (1);
                }
            }
        n_within_block++;
        np[thr]++;
        nptot++;
        total_bytes += payload_size;
        }

    printf ("read %ld bytes\n", total_bytes);
    printf ("time span %d:%d --> %d:%d\n", first_sec, first_frame, sec, nfr);

    printf ("thread                         ");
    for (i=0; i<nthreads; i++)
        printf ("%10d", thread[i]);
    printf ("\n");

    printf ("number of packets (by thread): ");
    for (i=0; i<nthreads; i++)
        printf ("%10d", np[i]);
    printf ("\n");

    printf ("number of packets out of order:");
    for (i=0; i<nthreads; i++)
        printf ("%10d", noos[i]);
    printf ("\n");

    printf ("number of second jumps:        ");
    for (i=0; i<nthreads; i++)
        printf ("%10d", nsjump[i]);
    printf ("\n");

    printf ("number of invalid/fill-frame:  ");
    for (i=0; i<nthreads; i++)
        printf ("%10d", nfill[i]);
    printf ("\n");

                                    // close files and skedaddle
    fclose (fin);
    for (i=0; i<MAXT; i++)
        if (opened[i])
            fclose (fout[i]);
    exit (0);
    }
