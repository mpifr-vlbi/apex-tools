// phylum thread - writes from circular buffer to file
//
// gratuitous obscure name from "Book 'em, Danno!" (Hawaii 50, circa 1965)
// created  -                                   rjc  2012.5.1 
// modified for continuous operation            rjc  2012.8.13
// use single phylum copy for multiple streams  rjc  2012.11.28

#include "dplane.h"

extern struct global_tag glob;

void phylum (void *id)
    {
    int kstr = 0,
        i,
        wr_size,
        icore,
        rc,
        nblock = 0,                 // block serial number
        wb_head_size,
        all_files_closeable,
        wrote_block;

    long int remainder,
             total_lead,
             overrun_trigger;

    struct wb_header_tag wb_header;

                                    // prototypes
    int scat_write (u_char *, int, int);

                                    // lock thread into a (hopefully) unique core
    icore = 1;
    if (bind2core (icore) == 0)
        {
        cprintf (2, "Started phylum on core %d\n", icore);
        }
    else
        {
        cprintf (1, "Problem binding phylum to core %d\n", icore);
        }
                                    // will trigger if change more than - 1/8 buffer
    overrun_trigger = - glob.rb_total_size / 8;
                                    // exit loop on terminate signal
                                    // *and* we're caught up (i.e. all buffers drained)
    while (glob.terminate_phylum == FALSE 
           || glob.stream[0].lead != 0 || glob.stream[1].lead != 0 
           || glob.stream[2].lead != 0 || glob.stream[3].lead != 0)
        {
        wrote_block = FALSE;
        for (kstr=0; kstr<glob.nstreams; kstr++)
          {
          total_lead = 0;           // will hold total buffer lead
          if (glob.stream[kstr].rb_state > CONFIGURED)
            {                       // stream in active state; check for work to do
                                    // write to disk if there is more
            glob.stream[kstr].lead = glob.stream[kstr].wpind - glob.stream[kstr].rpind;
            if (glob.stream[kstr].lead < 0 )          // correct for wp wrapped rel. to rp
                glob.stream[kstr].lead += glob.rb_size;
                                    // detect transition from draining to configured state
            else if (glob.stream[kstr].lead == 0   
                  && glob.stream[kstr].rb_state == DRAINING)
                {
                                    // place this stream back into configured state
                glob.stream[kstr].rb_state = CONFIGURED;
                glob.stream[kstr].ready_to_close = TRUE;
                }
            total_lead += glob.stream[kstr].lead;
                                    // write iff there is at least 1 write block
                                    // or we're no longer spooling and the lead is non-zero
            if (glob.stream[kstr].lead > glob.wb_payload_size 
             || glob.stream[kstr].rb_state == DRAINING)
                {
                                    // calc remainder of bytes to end of buffer
                remainder = glob.rb_size - glob.stream[kstr].rpind;
                                    // write min (WBLOCK, remainder, lead) bytes
                wr_size = (remainder > glob.wb_payload_size) ? glob.wb_payload_size : remainder;
                if (glob.stream[kstr].lead < wr_size)
                    wr_size = glob.stream[kstr].lead;
                                    // prepend the block serial number
                if (glob.outfiles.mode == RAID)
                    wb_head_size = 0;
                else
                    {
                    wb_head_size = sizeof (wb_header);
                    wb_header.blocknum = nblock;
                    wb_header.wb_size = wr_size + wb_head_size;
                    memcpy (glob.stream[kstr].rb_origin + glob.stream[kstr].rpind 
                        - wb_head_size, &wb_header, wb_head_size);
                    }
                                    // write out the block unless suppressed
                rc = scat_write (glob.stream[kstr].rb_origin 
                                 + glob.stream[kstr].rpind - wb_head_size, 
                                 wr_size + wb_head_size, kstr);

                if (rc >= 0)
                    {               // successfully wrote 1 block
                    glob.stream[kstr].rpind += wr_size;
                    nblock++;
                                    // zero out read pointer at wrap-around
                    if (glob.stream[kstr].rpind >= glob.rb_size)
                        glob.stream[kstr].rpind = 0;
                    wrote_block = TRUE;
                    }
                }
            }
          }                         // bottom of stream loop

                                    // check for overrun, iff overrun detection is active
        if (glob.overrun_detect)
            {
            if (total_lead - glob.previous_lead < overrun_trigger)
                glob.status.gsw |= GSW_BUFFER_OVERRUN_ERR;
            glob.previous_lead = total_lead;
            }
                                    // close output files, iff desired and buffers drained
        if (glob.capture.close_on_drain)
            {
            all_files_closeable = glob.nstreams > 0 ? TRUE : FALSE;
                                    // check that each active stream buffer has drained
            for (i=0; i<glob.nstreams; i++)
                all_files_closeable &= glob.stream[i].ready_to_close;
                                    // and that no file has an active write pending
            for (i=0; i<glob.nfils; i++)
                all_files_closeable &= (glob.sfile[i].wr_pending == FALSE);
                                    // close all output files if done writing
            if (all_files_closeable)
                {
                for (i=0; i<glob.nfils; i++)
                    {
                    if (glob.sfile[i].file_open == FALSE)
                        {
                        cprintf (2, "ignoring close request for file %s, which is not open\n",
                                glob.sfile[i].filename);
                        continue;
                        }
                    rc = fclose (glob.sfile[i].phyle);
                    if (rc)
                        { 
                        cprintf (1, "file %s fclose error rc %d: ", glob.sfile[i].filename, rc);
                        perror (NULL);
                        }
                    else
                        cprintf (2, "%s closed - wrote %7.1f GB\n", 
                                 glob.sfile[i].filename, glob.sfile[i].nbytes / 1e9);

                    glob.sfile[i].file_open = FALSE;
                    }
                                    // clear ready to close flags
                for (i=0; i<glob.nstreams; i++)
                    glob.stream[i].ready_to_close = FALSE;
                                    // show that all files are now closed
                glob.status.gsw &= ~GSW_FILES_OPEN;
                nblock = 0;         // get ready to start writing later at block 0
                                    // disable the overrun detection logic
                glob.overrun_detect = FALSE;
                }
            }
                                    // sleep iff no block was written this cycle
        if (!wrote_block)
            usleep (WSLEEP_US);
        }
    glob.terminated_phylum = TRUE;
    }

