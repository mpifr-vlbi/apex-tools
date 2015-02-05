// dplane - main program
//
// created                                 rjc 2012.4.30
// modified to stay resident               rjc 2012.7.20

#include "dplane.h"

struct global_tag glob;
unsigned short int lookup[65536];

int main (int argc, 
          char **argv)
    {
    struct timeval tv;
    struct timezone tz;
    __useconds_t wait_us;

    double tdelta;

                                    // function prototypes
    void initialize (void);
    void message_handler (void);
    void periodic_services (void);
    void sequencer (void);

                                    // trap invalid run sequences
    if (argc != 2 || strcmp (argv[1],"-h") == 0 || strcmp (argv[1],"--help") == 0
                  || strlen (argv[1]) != 1 || *(argv[1]) < '0' ||  *(argv[1]) > '3')
        {
        printf ("Usage:\n");
        printf ("dplane <verbosity>      (0|1|2|3 for mute|terse|gabby|prolix)\n");
        exit (0);
        }
    glob.verbosity = atoi (argv[1]);

    cprintf (2, "********** dplane v%4.2f active\n", VERSION);

                                    // initialize global area, start threads, etc.
    initialize ();

                                    // loop forever...or until dplane terminates
    while (TRUE)
        {
        gettimeofday (&tv, &tz);
        glob.tnow = (double ) tv.tv_sec + 1e-6 * tv.tv_usec;
                                        // process commands by stepping through appropriate
                                        // states for all active commands
        sequencer ();
                                        // do periodically scheduled services, if any
        periodic_services ();
                                        // read all pending messages, and process them
        message_handler ();
                                        // pause each time through major program loop
        gettimeofday (&tv, &tz);
                                        // calculate time for this pass in us
        tdelta = 1e6 * (tv.tv_sec + 1e-6 * tv.tv_usec - glob.tnow);

        if (tdelta <= 0.0)              // took no time or possible midnight rollover
            wait_us = LOOP_PERIOD;      // use nominal sleep period
                                      
        else                            // sleep for unused portion of LOOP_PERIOD
            wait_us = (int) (LOOP_PERIOD - tdelta);
                                        // don't sleep at all if we've overstayed our welcome
        if (tdelta < LOOP_PERIOD)
            usleep (wait_us);
        }
    exit (0);
    }


