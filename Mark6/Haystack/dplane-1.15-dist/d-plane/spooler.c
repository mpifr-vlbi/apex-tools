// spooler thread - just spool into big circular buffers
//
// created  - rjc 2012.4.30
// handle missing halves of mk5 packets cleanly  rjc 2013.3.20
// make packet reads mostly non-blocking         rjc 2013.11.25
// insert fill pattern for all missing packets   rjc 2014.1.9

#include "dplane.h"

extern struct global_tag glob;
extern unsigned short int lookup[65536];

void *spooler (void *id)
    {
    int kstr = (int) (long) id;     // stream for this thread (0..3)

    u_int32_t version;

    int rc,
        icore,
        i,
        jdref,
        *punk,
        iyear,
        num_dwords,
        jjj,
        sssss,
        sep,
        nmt,                        // # of consecutive empty packet reads
        first_armed = FALSE,
        m5_1st_half,                // 0|1 iff psn is odd|even
        datasize;

    unsigned int *vdata,
                 sequence_error,
                 spooling_mask,
                 fill_frame[2508],
                 last_psn,
                 *ppsn;             // ptr to the packet serial number

    unsigned short int *pfro,
                       *pto;

    long int psn_gap;

    char buf[32];

    u_char buffer[BUFLEN],
           mac_address[6],
           *pbuff,
           **ppbuff,
           *phdr;
    
    u_int8_t wait_for_packet = NON_BLOCK;
    // u_int8_t wait_for_packet = BLOCK;

    struct pfring_pkthdr hdr;

    struct vdif_header *pvdh,       // pointer to a vdif header in the buffer
                       vdh_fill;    // vdif header for fill frames
    struct mk5b_header *pm5h;       // pointer to a mk5b header in the buffer

    pfring *pd;

    struct timespec req,
                    rem;
    struct tm t;
    time_t tty;
                                    // table below is valid for year 2000.0 to 2032.0
                                    // and contains mjd on Jan 1 and Jul 1 for each year
    static int mjds[64] = 
        {
        544, 726, 910,  91, 275, 456, 640, 821,  // 2000-2003
          5, 187, 371, 552, 736, 917, 101, 282,  // 2004-2007
        466, 648, 832,  13, 197, 378, 562, 743,  // 2008-2011
        927, 109, 293, 474, 658, 839,  23, 204,  // 2012-2015
        388, 570, 754, 935, 119, 300, 484, 665,  // 2016-2019
        849,  31, 215, 396, 580, 761, 945, 126,  // 2020-2023
        310, 492, 676, 857,  41, 222, 406, 587,  // 2024-2027
        771, 953, 137, 318, 502, 683, 867,  48   // 2028-2031
        };

    cprintf (3, "spooler(%d) initiated\n", kstr);

    pd = NULL;

    pbuff = buffer;
    ppbuff = &pbuff;
    cprintf (3, "spooler[%d] pbuff %p ppbuff %p\n", kstr, pbuff, ppbuff);
    phdr = (u_char *) &hdr;
    memset (&hdr, 0, sizeof (hdr));
                                    // preload flags for this stream's status
    sequence_error = GSW_STR0_SEQUENCE_ERR << kstr;
    spooling_mask  = GSW_STR0_SPOOLING     << kstr;

                                    // construct skeleton vdif fill header
    memset ((void *) &vdh_fill, 0, sizeof (struct vdif_header));
    vdh_fill.invalid  = 1;
    vdh_fill.eversion = MK5B_SYNC >> 24;
    vdh_fill.extra1   = 0x00ffffff & MK5B_SYNC;
    vdh_fill.threadid = kstr;

                                    // create a packet fill-pattern samples
    for (i=0; i<2508; i++)
        fill_frame[i] = MK5B_FILL;
                                    // lock thread into a (hopefully) unique core
                                    // cpu's 1/2/5/6
    icore = (kstr + 2) % glob.numproc;

    if (bind2core (icore) == 0)
        {
        cprintf (2, "Started spooler thread %d on core %d\n", kstr, icore);
        }
    else
        {
        cprintf (1, "Problem binding spooler thread %d to core %d\n", kstr, icore);
        }
                                    // initialize delay structures
    req.tv_sec = 0;
    req.tv_nsec = 1000 * PACKET_SLEEP;


                                    // infinite loop until quittin' time
    while (glob.stream[kstr].terminate_spooler == FALSE)
        {
        switch (glob.stream[kstr].rb_state)
          {
                                    // are we configuring a new stream?
          case CONFIGURING:
                                    // first check to see if old stream needs closing
                                    //
                                    // open pfring on devices
            if (pd == NULL)         // skip over open if already open
                {
                pd = pfring_open (glob.stream[kstr].dev, glob.stream[kstr].packsize,
                                  PF_RING_REENTRANT | PF_RING_PROMISC);

                if (pd == NULL) 
                    {
                    cprintf (1, "pfring_open error for %s (pf_ring not loaded"
                             " or not in privileged mode?)\n", glob.stream[kstr].dev);
                    glob.status.gsw |= GSW_PFRING_OPEN_ERR;
                    return (NULL);
                    } 
                else 
                    {
                    pfring_set_application_name (pd, "dplane");
                    pfring_version (pd, &version);

                    cprintf (3, "Using PF_RING v.%d.%d.%d\n",
                                (version & 0xFFFF0000) >> 16, 
                                (version & 0x0000FF00) >> 8, 
                                 version & 0x000000FF);
                    }

                if (pfring_get_bound_device_address (pd, mac_address) != 0)
                    {
                    cprintf(1, "pfring_get_bound_device_address() failed\n");
                    }
                else
                    {
                    cprintf (2, "input device %s [%s] packet length %d\n", 
                             glob.stream[kstr].dev, etheraddr_string (mac_address, buf),
                             glob.stream[kstr].packsize);
                    }
                }
            else
                {
                cprintf (3, "stream %d already open, skipping pfring_open\n", kstr);
                }

            switch (glob.stream[kstr].format)
                {
                case VDIF:          // point to vdif header (past other hdrs)
                    pvdh = (struct vdif_header *) 
                       (pbuff + glob.new_streams.stream[kstr].payload_offset);
                    vdh_fill.framelength = glob.new_streams.stream[kstr].payload_size/8 + 4;
                    num_dwords = 1; // ensure that stats are updated
                    break;

                case MK5B:          // point to vdif header (past other hdrs)
                    pm5h = (struct mk5b_header *) 
                       (pbuff + glob.new_streams.stream[kstr].payload_offset);
                    vdh_fill.framelength = glob.new_streams.stream[kstr].payload_size/4 + 2;
                                    // calculate seconds since vdif epoch
                    tty = (time_t) glob.tnow;
                    gmtime_r ( &tty, &t);
                    sep = t.tm_yday * 86400 + (t.tm_hour * 60 + t.tm_min) * 60 + t.tm_sec;
                    iyear = (t.tm_year - 100) * 2;
                                    // adjust to July 1 epoch if 2nd half of year
                    if (t.tm_mon > 5)
                        {
                        iyear++;    // indicate next epoch
                        if (t.tm_year % 4)
                            sep -= 15638400;
                        else        // leap year
                            sep -= 15724800;
                        }
                    jdref = mjds[iyear];
                    break;

                case UNKNOWN_FORMAT:
                default:
                    punk = (void *) 
                       (pbuff + glob.new_streams.stream[kstr].payload_offset);
                    break;
                }
                                    // psn_offset of -1 means no psn checking..pointer null
            ppsn = glob.new_streams.stream[kstr].psn_offset == -1 ? 0 :
                (unsigned int *) (pbuff +  glob.new_streams.stream[kstr].psn_offset);
                                    // enable the ring (needed for non-blocking calls)
            rc = pfring_enable_ring (pd);
            nmt = 0;                // go to more aggressive packet reading
                                    // update rb state
            glob.stream[kstr].rb_state = CONFIGURED;
            break;



          case CONFIGURED:
            glob.stream[kstr].nbytes = 0;
            glob.stream[kstr].npack_good = 0;
            glob.stream[kstr].npack_wrong_psn = 0;
            glob.stream[kstr].npack_wrong_len = 0;
            first_armed = TRUE;
          case ARMED:
          case SPOOLING:
          case DRAINING:
                                    // rb state is currently spooling or armed (by capture cmd)
                                    // try to read packet with (blocking) call to pfring
            rc = pfring_recv (pd, ppbuff, BUFLEN, &hdr, wait_for_packet);
                                    // was the packet empty?
            if (rc == 0)
                {
                nmt++;              // yes, bump empty packet counter
                break;
                }
            nmt = 0;                // reset count on proper packet receipt

                                    // discard packets if neither armed nor spooling
            if (glob.stream[kstr].rb_state == CONFIGURED
             || glob.stream[kstr].rb_state == DRAINING)
                break;
           
          
                                    // process packet ************* start

            if (rc > 0)             // packet received OK?
                {
                                    // reject all packets of incorrect length
                if (hdr.caplen != glob.stream[kstr].packsize)
                    {
                    cprintf (3, "caplen %d != packsize %d\n",
                             hdr.caplen,glob.stream[kstr].packsize);
                    glob.stream[kstr].npack_wrong_len++;
                    break;
                    }
                                    // handling of the time field is format-specific
                if (glob.stream[kstr].format == MK5B)
                    {
                                    // if it's available, the parity of the packet ser#
                                    // is used to decide which half packet
                                    // otherwise, look for m5b sync word to decide
                    m5_1st_half = ppsn ?
                        (*ppsn & 1) == 0 :
                        pm5h->sync_word == MK5B_SYNC;

                                    // first half of m5 packet?
                    if (m5_1st_half)
                        {
                                    // calculate vdif time since epoch
                                    // decode bcd time fields
                        jjj = 100 * ((pm5h->jjj        ) >> 8)
                             + 10 * ((pm5h->jjj & 0x0f0) >> 4)
                             +       (pm5h->jjj & 0x00f);

                        sssss = 10000 * ( pm5h->sssss           >> 16)
                               + 1000 * ((pm5h->sssss & 0xf000) >> 12)
                               +  100 * ((pm5h->sssss & 0x0f00) >>  8)
                               +   10 * ((pm5h->sssss & 0x00f0) >>  4)
                               +        ( pm5h->sssss & 0x000f       );

                        glob.stream[kstr].t_vdif = (jjj - jdref) * 86400 + sssss;
                                    // handle 1000-day wrap case
                        if (glob.stream[kstr].t_vdif < 0)
                            glob.stream[kstr].t_vdif += 86400000;
                        }

                                    // second half of m5 packet
                    else if (glob.stream[kstr].rb_state != SPOOLING)
                        break;      // don't even bother looking at non-headers
                    }
                                    // copy in vdif time as read iff vdif format
                else if (glob.stream[kstr].format == VDIF)
                    glob.stream[kstr].t_vdif = pvdh->seconds;

                                    // check for debug mode with relative timing
                                    // only once, upon first arming and good time receipt
                if (first_armed)
                    { 
                                    // if desired, initiate a do-once relative start time set
                    if (glob.stream[kstr].tstart_vdif < 60)
                        {           // start and stop are relative to current time
                        glob.stream[kstr].tstart_vdif += glob.stream[kstr].t_vdif;
                        glob.stream[kstr].tstop_vdif  += glob.stream[kstr].t_vdif;
                        cprintf (2, "relative mode vdif start..end %d..%d\n", 
                                    glob.stream[kstr].tstart_vdif, 
                                    glob.stream[kstr].tstop_vdif);
                        }
                                    // change boilerplate fill frame threadid for vdif
                    if (glob.stream[kstr].format == VDIF)
                        vdh_fill.threadid = pvdh->threadid;
                    first_armed = FALSE;
                    }

                                    // process packets for times within the start-stop range

                if (glob.stream[kstr].t_vdif >= glob.stream[kstr].tstart_vdif
                 && glob.stream[kstr].t_vdif <  glob.stream[kstr].tstop_vdif)
                    {   
                                    // capture the packet

                                    // into desired data span; rb state transition?
                    if (glob.stream[kstr].rb_state == ARMED)
                        {
                                    // update status bit to show we are spooling
                        glob.status.gsw |= spooling_mask;
                                    // arrange the psn to show no gap (at startup)
                        if (ppsn)
                            last_psn = *ppsn - 1;
                                    // set rb state to spooling
                        glob.stream[kstr].rb_state = SPOOLING;
                        cprintf (2, "stream %d spooling started at %d\n",
                                kstr, glob.stream[kstr].t_vdif);
                        }

                                    // check packet seq#, if not suppressed
                    if (ppsn)
                        {
                        psn_gap = (long int) *ppsn - (long int) last_psn - 1;
                                    // detect rollovers and mark as OK
                        if (*ppsn == 0 && (last_psn == 0x7fffffff || last_psn == 0xffffffff))
                            psn_gap = 0;
                                    // for now, discard any packets with earlier psn's
                                    // someday we'll allow limited "back-filling"
                        if (psn_gap < 0)
                            {
                            cprintf (3, "ppsn %d last_psn %d\n",*ppsn,last_psn);
                            cprintf (3, "negative psn_gap of %ld packets\n", psn_gap);
                            break;
                            }
                                    // limit the max# of fill packets to be inserted
                        if (psn_gap > MAX_PSN_GAP)  // but keep parity the same
                            psn_gap = (psn_gap % 2) ? MAX_PSN_GAP-1 : MAX_PSN_GAP;
                        
                                    // fill in the missing packets, if there are any
                        if (psn_gap > 0)
                            {
                            cprintf (3, "stream %d filled %ld packets to psn %ld\n",
                                     kstr, psn_gap, *ppsn);
                            for (i=0; i<psn_gap; i++)
                                {
                                datasize = glob.stream[kstr].paysize;
                                    // copy in a skeleton vdif header if it's vdif
                                if (glob.stream[kstr].format == VDIF)
                                    {
                                    memcpy (glob.stream[kstr].rb_origin + glob.stream[kstr].wpind,
                                        (void *) &vdh_fill, 32);
                                    glob.stream[kstr].wpind += 32;
                                    // paysize includes vdif header
                                    datasize = glob.stream[kstr].paysize - 32;
                                    }
                                    // or the 1st half of a mk5b frame
                                else if (glob.stream[kstr].format == MK5B
                                               && ((last_psn + i) & 1) == 1)
                                    {
                                    memcpy (glob.stream[kstr].rb_origin + glob.stream[kstr].wpind,
                                        (void *) &vdh_fill, 16);
                                    glob.stream[kstr].wpind += 16;
                                    // paysize includes mk5b header
                                    datasize = glob.stream[kstr].paysize - 16;
                                    }
                                    // copy in fill-pattern samples
                                memcpy (glob.stream[kstr].rb_origin + glob.stream[kstr].wpind,
                                        (void *) fill_frame, datasize);
                                    // update write pointer
                                glob.stream[kstr].wpind += datasize;
                                    // look ahead - if next packet would overflow
                                    // force buffer wrap-around
                                if (glob.stream[kstr].wpind + glob.stream[kstr].paysize 
                                        > glob.rb_size)
                                    glob.stream[kstr].wpind = 0;
                                }
                            glob.stream[kstr].npack_wrong_psn += psn_gap;
                                    // report that an error occurred
                            glob.status.gsw |= sequence_error;
                            }
                        }

                                    // format-dependent packet processing

                                    // vdif format?
                    if (glob.stream[kstr].format == VDIF)
                                    // just copy vdif packet to rb
                        memcpy (glob.stream[kstr].rb_origin + glob.stream[kstr].wpind,
                                pvdh, glob.stream[kstr].paysize);
                    

                                    // mk5b format?
                    else if (glob.stream[kstr].format == MK5B)
                        {           // for mk5b, combine pkt motion and conversion
                        if (m5_1st_half)
                            {       // 1st half packet received
                                
                                    // first insert vdif header words into ring buffer
                            vdata = (unsigned int *) (glob.stream[kstr].rb_origin 
                                                    + glob.stream[kstr].wpind);
                            vdata[0] = glob.stream[kstr].t_vdif;
                            vdata[1] = iyear << 24 | pm5h->frame_num;
                            vdata[2] = CHANS_16 | M5VDIFLENGTH;
                                    // 2 b/samp, threadID, station 'm5'
                            vdata[3] = TWOBITS | kstr << 16 | STATION_ID;
                                    // reposition write pointer past vdif header
                            glob.stream[kstr].wpind += 16;
                                    // copy in unmodified mk5b header
                            memcpy (vdata + 4, pm5h, 16);
                            pto = (short unsigned int *) vdata + 16;
                            pfro = (short unsigned int *) pm5h + 8;
                                    // will copy first 4992 bytes
                            num_dwords = 624; 
                            }

                        else        // 2nd half packet received
                            {       // set up copy for 2nd half of packet
                                pto = (short unsigned int *) (glob.stream[kstr].rb_origin 
                                                            + glob.stream[kstr].wpind);
                                pfro = (short unsigned int *) pm5h;
                                    // will copy last 5008 bytes
                                num_dwords = 626; 
                            }

                                    // do conversion of ~5 KB of mk5b samples 
                                    // to vdif encoding
                        for (i=0; i<num_dwords; i++)
                            {       // coded inline for efficiency, 8 bytes/loop
                            *pto = lookup[*pfro];
                            pto++;
                            pfro++;
                            *pto = lookup[*pfro];
                            pto++;
                            pfro++;
                            *pto = lookup[*pfro];
                            pto++;
                            pfro++;
                            *pto = lookup[*pfro];
                            pto++;
                            pfro++;
                            }
                        }           // bottom of if mk5b

                    else            // copy unknown format packet to rb
                        memcpy (glob.stream[kstr].rb_origin + glob.stream[kstr].wpind,
                                punk, glob.stream[kstr].paysize);


                                    // if any words were copied, update write ptr and stats
                    if (num_dwords > 0)
                        {
                                    // update write pointer
                        glob.stream[kstr].wpind += glob.stream[kstr].paysize;
                        glob.stream[kstr].nbytes += glob.stream[kstr].paysize;
                        glob.stream[kstr].npack_good++;
                        }
                                    // look ahead - if next packet would overflow
                                    // force buffer wrap-around
                    if (glob.stream[kstr].wpind + glob.stream[kstr].paysize > glob.rb_size)
                        glob.stream[kstr].wpind = 0;
                    }               // bottom of loop for times within range

                if (glob.stream[kstr].rb_state >= CONFIGURED && ppsn != 0)
                    last_psn = *ppsn;       // update the last packet ser# read
                }
                                    // process packet ************* end
            break;



          default:                  // quiescent, force sleeps and wait for something to happen
            nmt = MAX_CONSECUTIVE_MT;
            break;
          }

                                    // state-machine bookkeeping

                                    // if t>= end of span or abort set
                                    // shut the capture process down
        if (glob.stream[kstr].rb_state == SPOOLING 
         && glob.stream[kstr].t_vdif >= glob.stream[kstr].tstop_vdif
         || glob.stream[kstr].abort_request)
            {
            cprintf (2, "stream %d spooling ended at %d\n", kstr, glob.stream[kstr].t_vdif);
            cprintf (2, "stream %d: packets good %ld   fill %ld   bytes %ld   wrong len %ld\n", 
                     kstr, glob.stream[kstr].npack_good, glob.stream[kstr].npack_wrong_psn,
                     glob.stream[kstr].nbytes, glob.stream[kstr].npack_wrong_len);
                                    // update status bit to show we are no longer spooling
            glob.status.gsw &= ~spooling_mask;
                                    // set rb state to configured if abort or caught up
            if (glob.stream[kstr].abort_request && glob.abort.drain_buffers == FALSE
             || glob.stream[kstr].lead == 0)
                {
                glob.stream[kstr].rb_state = CONFIGURED;
                glob.stream[kstr].lead = 0;
                glob.stream[kstr].ready_to_close = TRUE;
                }
            else                    // otherwise, give ring buffer time to drain
                glob.stream[kstr].rb_state = DRAINING;
                                    // clear possible causes 
            glob.stream[kstr].abort_request = FALSE;
            glob.command[CAPTURE].state = IDLE;
            }
                                    // close stream if requested and this one's open
        if (glob.close_streams && glob.stream[kstr].rb_state != QUIESCENT)
            {
            pfring_close (pd);
            pd = NULL;
            glob.stream[kstr].rb_state = QUIESCENT;
            }
                                    // break constant execution on string of empty reads
        if (nmt >= MAX_CONSECUTIVE_MT)
            nanosleep (&req, &rem);
        }             
                                    // close pfring
    if (pd != NULL)
        pfring_close (pd);
    glob.stream[kstr].terminated_spooler = TRUE;
    return (NULL);
    }
