// perform any work that is on a periodic schedule
//
// initial code                            rjc 2012.7.20

#include "dplane.h"

extern struct global_tag glob;

void periodic_services ()
    {
    int i;
    char buff[80];
    static int first_time = TRUE;
     
                                    // first time through - do once
    if (first_time)
        {
        if (gethostname (buff, sizeof(buff)) < 0)
            {
            printf ("error retrieving hostname for statuses\n");
            strcpy (glob.status.hostname, "noname");
            }
        else
            {                       // got name, strip off domain, if it's there
            for (i=0; i<sizeof(buff); i++)
                if (buff[i] == '.')
                    buff[i] = 0;
            strncpy (glob.status.hostname, buff, sizeof (glob.status.hostname));
            cprintf (3, "hostname %s\n", glob.status.hostname);
            }
                                    // boiler plate for status msg
        glob.status.head.code = STATUS;
                                    // set up a default status interval
        glob.command[STATUS].state = ACTIVE;
        glob.command[STATUS].tlast = 0;
        glob.request_status.interval = DEFAULT_STATUS_INTERVAL;
        first_time = FALSE;
        }
                                    // send status msg if requested and due
    if (glob.command[STATUS].state == ACTIVE)
        {
        if (glob.request_status.interval == 0
         || glob.tnow > glob.command[STATUS].tlast + glob.request_status.interval)
                                    // status desired, send a message
            {
                                    // recently read system clock value
            glob.status.tsys = glob.tnow;
                                    // transfer buffer usage & time into status command
            for (i=0; i<glob.nstreams; i++)
                {                 
                glob.status.stream[i].t_vdif_recent = glob.stream[i].t_vdif;
                                    // calculate buffer usage in rounded MB
                glob.status.stream[i].mb_ring = (int) ((glob.stream[i].lead + 500000)
                                                        / 1000000);
                glob.status.stream[i].npack_good = glob.stream[i].npack_good;
                glob.status.stream[i].npack_wrong_psn = glob.stream[i].npack_wrong_psn;
                glob.status.stream[i].npack_wrong_len = glob.stream[i].npack_wrong_len;
                glob.status.stream[i].nbytes = glob.stream[i].nbytes;
                }
            glob.status.mb_cache = (int) ((glob.backlog_total + 500000)
                                                        / 1000000);
            glob.status.nstreams = glob.nstreams;
                                    // broadcast this udp message
            sendmess (&glob.status, sizeof (glob.status), glob.bcast_addr, FROM_DPLANE, TRUE);
            glob.command[STATUS].tlast = glob.tnow;
                                    // clear error flags in the gsw
            glob.status.gsw &= ~GSW_ALL_ERRORS;

            }
                                    // 0 interval implies do only once
        if (glob.request_status.interval == 0)
            glob.command[STATUS].state = IDLE;
        }
    }
