// sequencer moves the various commands along through their state machines
//
// initial version (patterned on domino)                    rjc 2012.7.20

#include "dplane.h"

extern struct global_tag glob;

void sequencer ()
    {
    int i,
        j,
        kstr,
        n,
        nc,
        any_armable,
        any_open,
        all_threads_terminated,
        active_stream,
        open_stream,
        num_wblocks,
        rc,
        imod,
        idisk,
        fm,
        bitmask;

    long int npkt;

    char *name;

    struct file_header_tag file_header;

    struct passwd *ppw;
    struct group *pgr;

    extern pthread_t fthred[NFILES];
                                    // prototypes
    void *writer (void *);


    for (nc=0; nc<NUM_COMMANDS; nc++)
        if (glob.command[nc].state != IDLE)
            switch (nc)
                {
                case ABANDON_FILES: // abandon use of one or more files
                    for (i=0; i<NFILES; i++)
                        if (glob.abandon_files.file_mask & (1<<i))
                            {
                                    // kill the writer thread
                            pthread_cancel (fthred[i]);
                                    // clear associated statuses
                            glob.status.file_timeout &= ((1<<i) ^ 0xffffffff);
                            glob.sfile[i].wr_pending = FALSE;
                            glob.sfile[i].file_open = FALSE;
                            cprintf (1, "abandoned file #%d: %s\n", i, glob.sfile[i].filename);
                                    // start replacement writer thread
                            pthread_create (&fthred[i], NULL, writer,  (void *) (long) i);
                            }
                    glob.command[ABANDON_FILES].state = IDLE;
                    break;

                case ABORT:         // abort recording
                    for (i=0; i<glob.nstreams; i++)
                        glob.stream[i].abort_request = TRUE;
                                    // force closure of file, too
                    glob.capture.close_on_drain = TRUE;
                    glob.command[ABORT].state = IDLE;
                    break;

                case CAPTURE:
                    if (glob.command[CAPTURE].state == POSTED)
                        {           // process new request for capture
                        any_armable = FALSE;
                        for (i=0; i<glob.nstreams; i++)
                            {
                            if (glob.stream[i].rb_state != CONFIGURED)
                                    {
                                    cprintf (1, "stream %d not configured, skipping capture\n", i);
                                    continue;
                                    }
                                    // convert start and end into stream format (if necessary)
                                    // set start and end - FIXME for other time options
                            glob.stream[i].tstart_vdif = glob.capture.epoch_time;
                            glob.stream[i].tstop_vdif = glob.capture.epoch_time 
                                                      + glob.capture.duration;
                            cprintf (2, "stream %d capture start and stop times %d : %d\n", 
                                     i, glob.stream[i].tstart_vdif, glob.stream[i].tstop_vdif); 
                            any_armable = TRUE;
                            }
                                    // ensure that there is at least 1 active stream
                        if (!any_armable)
                            {
                            cprintf (0, "attempted capture with no streams armable\n");
                            glob.status.gsw |= GSW_NO_STREAMS_ERR;
                            glob.command[CAPTURE].state = IDLE;
                            break;
                            }
                                    // ensure that there is at least one open file
                        any_open = FALSE;
                        for (i=0; i<glob.nfils; i++)
                            any_open |= glob.sfile[i].file_open;
                        if (!any_open)
                            {
                            cprintf (0, "attempted capture with no open files\n");
                            glob.status.gsw |= GSW_NO_FILES_ERR;
                            glob.command[CAPTURE].state = IDLE;
                            break;
                            }
                                    // reset previous lead to 0, arm overrun detect
                        glob.previous_lead = 0;
                        glob.overrun_detect = TRUE;
                                    // arm the streams for capture at the appropriate time
                        for (i=0; i<glob.nstreams; i++)
                            if (glob.stream[i].rb_state == CONFIGURED)
                                glob.stream[i].rb_state = ARMED;

                        glob.command[CAPTURE].state = ACTIVE;
                        }
                    else if (glob.command[CAPTURE].state == ACTIVE)
                        {
                        for (i=0; i<glob.nstreams; i++)
                            {
                            if (glob.stream[i].rb_state == QUIESCENT)
                                {
                                    // wait until buffers drain
                                    // close files
                                }
                            }
                        }
                    break;

                case MODULE_INFO:   // should this be handled by cplane??
                    printf ("module_info message NYI\n");
                    glob.command[MODULE_INFO].state = IDLE;
                    break;

                case NEW_STREAMS:
                    if (glob.command[NEW_STREAMS].state == POSTED)
                        {
                                    // find out if any streams are active or open
                        active_stream = FALSE;
                        open_stream   = FALSE;
                        for (i=0; i<MAXSTREAM; i++)
                            {
                            active_stream |= glob.stream[i].rb_state > CONFIGURED;
                            open_stream   |= glob.stream[i].rb_state != QUIESCENT;
                            }
                                    // can't define new stream is any still active
                        if (active_stream)
                            {
                            cprintf (1, "previous stream still active\n");
                            glob.status.gsw |= GSW_ACTIVE_STREAMS_ERR;
                            glob.command[NEW_STREAMS].state = IDLE;
                            break;
                            }
                                    // if any streams are open, they need to be closed
                        if (open_stream)
                            {
                            glob.close_streams = TRUE;
                            break;  // stay in POSTED state until streams close
                            }
                                    // they're all closed, don't keep trying
                        glob.close_streams = FALSE;
                                    // only add new streams if there really are some
                        glob.nstreams = glob.new_streams.num_streams;
                        if (glob.nstreams > 0)
                            {
                                    // mk5b needs special treatment, as each vdif record
                                    // that is newly created has 2 mk5 packets + vdif header
                            if (strncmp (glob.new_streams.stream[0].format, "mk5b", 4) == 0)
                                glob.outpkt_size = 2 * glob.new_streams.stream[0].payload_size + 16;
                            else
                                glob.outpkt_size = glob.new_streams.stream[0].payload_size;

                                    // set write block size to be a multiple of outpkt_size
                            npkt = (WBLOCK_SIZE - sizeof (struct wb_header_tag)) / glob.outpkt_size;
                            glob.wb_payload_size = npkt * glob.outpkt_size;
                                    // subdivide buffer for new streams
                            glob.rb_size = glob.rb_total_size / glob.nstreams 
                                     - sizeof (struct wb_header_tag);
                                   // set ring buffer size to be a multiple of write block
                                   // size 
                            num_wblocks = glob.rb_size / glob.wb_payload_size;
                            glob.rb_size = (long int) num_wblocks * glob.wb_payload_size;
                            cprintf (2, "stream rb_size %ld num_wblocks %d wb_payload_size %d "
                                         "pkts/blk %ld\n", 
                                     glob.rb_size, num_wblocks, glob.wb_payload_size, npkt);

                            for (i=0; i<glob.nstreams; i++)
                                {
                                    // for now, assume all streams same format as first
                                if (strncmp (glob.new_streams.stream[0].format, "vdif", 4) == 0)
                                    glob.stream[i].format = VDIF;
                                else if (strncmp (glob.new_streams.stream[0].format, "mk5b", 4) == 0)
                                    glob.stream[i].format = MK5B;
                                else
                                    glob.stream[i].format = UNKNOWN_FORMAT;

                                glob.stream[i].paysize = glob.new_streams.stream[0].payload_size;
                                glob.stream[i].packsize = glob.stream[i].paysize 
                                                     + glob.new_streams.stream[0].payload_offset;
                                    // initialize ring buffer pointers; offset start by
                                    // sizeof (struct wb_header_tag) bytes 
                                    // for space for prepended block header (which may be
                                    // suppressed; if not, internal headers overwrite 
                                    // previous block)
                                glob.stream[i].rb_origin = glob.porb + i * glob.rb_size 
                                                         + sizeof (struct wb_header_tag);
                                cprintf (2, "new stream %d buffer origin %p\n", 
                                         i, glob.stream[i].rb_origin);
                                glob.stream[i].wpind = 0;
                                glob.stream[i].rpind = 0;
                                    // copy new device names into global area
                                strncpy (glob.stream[i].dev, glob.new_streams.stream[i].id, 5);
                                    // enable spooler to configure pfring for subsequent capture
                                glob.stream[i].ready_to_close = FALSE;
                                glob.stream[i].abort_request = FALSE;
                                glob.stream[i].rb_state = CONFIGURING;
                                }
                            }
                        }
                    glob.command[NEW_STREAMS].state = IDLE;
                    break;

                case OUTFILES:      // assign file names for subsequent data writes
                    if (glob.command[OUTFILES].state == POSTED)
                        {
                                    // ensure that previous files have all been closed
                        for (i=0; i<glob.nfils; i++)
                            if (glob.sfile[i].file_open > 0)
                                {
                                printf ("Error: previous files are still open; "
                                        "try again later\n");
                                perror (NULL);
                                glob.status.gsw |= GSW_FACTIVE_ERR;
                                glob.command[OUTFILES].state = IDLE;
                                break;
                                }
                                    // double-break out of OUTFILES case
                        if (i<glob.nfils)
                            break;

                                    // setup file header block
                        file_header.sync_word = SYNC_WORD;
                        file_header.version = FILE_VERSION;
                        file_header.block_size = glob.wb_payload_size 
                                               + sizeof (struct wb_header_tag);
                                    
                                    // for now, assume all streams same format as first
                        file_header.packet_format = glob.stream[0].format;
                        file_header.packet_size = glob.outpkt_size;

                                    // extract information out of outfiles message
                        n = 0;
                        glob.nfils = 0;

                        if (glob.outfiles.mode == RAID)
                            {
                            strcpy (glob.sfile[0].filename, "/mnt/raid/");
                            strncat (glob.sfile[0].filename, glob.outfiles.fname, MAX_NAME);
                            n = 1;
                            }
                        else        // sg mode, need to generate multiple file names
                            {
                            fm = glob.outfiles.fmask[0] | glob.outfiles.fmask[1]
                               | glob.outfiles.fmask[2] | glob.outfiles.fmask[3];
                                    // loop over modules and disks
                            bitmask = 1;
                            for (imod=1; imod<5; imod++)
                                for (idisk=0; idisk<8; idisk++)
                                    {
                                    // is this disk active for at least one of the streams?
                                    if (fm & bitmask)
                                        {
                                    // form appropriate name and put in global area
                                        sprintf (glob.sfile[n].filename, "/mnt/disks/%d/%d/", imod, idisk);
                                        strncat (glob.sfile[n].filename, glob.outfiles.fname, MAX_NAME);
                                    // form file subpools for each stream
                                        for (kstr=0; kstr<4; kstr++)
                                            if (glob.outfiles.fmask[kstr] & bitmask)
                                                glob.stream[kstr].fpool[n] = TRUE;
                                        n++;
                                        }
                                    // shift to examine next disk's bit
                                    bitmask <<= 1;
                                    }
                            }
                                    // post the number of files found to be active
                        glob.nfils = n;
                            
                                    // open each file
                        for (i=0; i<glob.nfils; i++)
                            {
                            name = glob.sfile[i].filename; 
                                    // open file to be ready for writes
                            glob.sfile[i].phyle = fopen (name, "w");

                                    // if open error report & abort outfiles command
                            if (glob.sfile[i].phyle == NULL)
                                {
                                cprintf (1, "Error opening file %s\n", name);
                                perror (NULL);
                                glob.status.gsw |= GSW_FOPEN_ERR;
                                glob.status.gsw_fileno = i;
                                    // don't leave open files dangling
                                for (j=0; j<i; j++)
                                    {
                                    if (fclose (glob.sfile[j].phyle))
                                        cprintf (1, "Problem closing file %s, too!\n",
                                                  glob.sfile[i].filename);
                                    glob.sfile[j].file_open = FALSE;
                                    }
                                glob.command[OUTFILES].state = IDLE;
                                break;
                                }
                            else    // opened OK
                                {
                                glob.sfile[i].file_open = TRUE;
                                cprintf (2, "%s opened\n", name);
                                glob.sfile[i].nbytes = 0;
                                    // disable stream-level buffering
                                setbuf (glob.sfile[i].phyle, NULL);
                                }
                                    // show that some files are now open
                            glob.status.gsw |= GSW_FILES_OPEN;

                                    // change owner and group to match oper account
                            ppw = getpwnam ("oper");
                            if (ppw == NULL)
                                {
                                cprintf (1, "problem getting password file entry for oper\n");
                                }
                            else
                                {
                                pgr = getgrnam ("mark6");
                                if (pgr == NULL)
                                    {
                                    cprintf (1, "problem getting group file entry for mark6\n");
                                    }
                                else
                                    {

                                    rc = chown (name, ppw->pw_uid, pgr->gr_gid);
                                    if (rc)
                                        {
                                        cprintf (1, "problem using chown to change owner & group\n");
                                        perror ("details");
                                        }
                                    }
                                }

                                    // write file header block if sg mode
                            if (glob.outfiles.mode == SG)
                                {
                                rc = fwrite (&file_header, sizeof (file_header), 1,
                                             glob.sfile[i].phyle);
                                    // check for write error, and handle if necessary
                                if (rc < 0)
                                    {
                                    cprintf (1, "header write error rc %d: ", rc);
                                    perror (NULL);
                                    glob.status.gsw |= GSW_WRITE_ERROR;
                                    glob.command[OUTFILES].state = IDLE;
                                    break;
                                    }
                                glob.sfile[i].nbytes += sizeof (file_header);
                                }
                            }
                        }
                    glob.command[OUTFILES].state = IDLE;
                    break;

                case REQUEST_STATUS:
                    glob.status.head.code = STATUS;
                    glob.command[REQUEST_STATUS].state = IDLE;
                    glob.command[STATUS].state = ACTIVE;
                    break;

                case TERMINATE:     // manage shutdown of program
                    if (glob.command[TERMINATE].state == POSTED)
                        {
                                    // ask threads to terminate and
                                    // wait for buffers to drain
                        for (i=0; i<MAXSTREAM; i++)
                            glob.stream[i].terminate_spooler = TRUE;
                        glob.terminate_phylum = TRUE;
                        glob.command[TERMINATE].state = ACTIVE;
                        cprintf (2, "dplane terminating\n");
                        }
                    else if (glob.command[TERMINATE].state == ACTIVE)
                        {
                                    // don't quit until all spooler and phylum 
                                    // threads have terminated
                        all_threads_terminated = TRUE;
                        for (i=0; i<MAXSTREAM; i++)
                            if (glob.stream[i].terminated_spooler == FALSE)
                                all_threads_terminated = FALSE;
                        if (glob.terminated_phylum  == FALSE)
                            all_threads_terminated = FALSE;
                                    // can quit now iff all threads are done
                        if (all_threads_terminated 
                         || glob.terminate.force_terminate == TRUE)
                            {
                            glob.command[TERMINATE].state = IDLE;
                            cprintf (2, "********** dplane normal exit\n");
                            exit (0);
                            }
                        }
                }
    }
