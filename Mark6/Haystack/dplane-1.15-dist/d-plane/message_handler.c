// handles all inbound and outbound message traffic
//
// initial version                      rjc 2012.7.20

#include "dplane.h"
#define MAXMESS 5000                // must be >= longest message (bytes)

extern struct global_tag glob;

void message_handler ()
    {
    static int first_time = TRUE,
               sockfd;

    U8 buff[MAXMESS];
    char sport[11];

    int i,
        numbytes,
        rc;

    char *command_strings[10] =
        {
        "ABANDON_FILES",
        "ABORT",
        "CAPTURE",
        "MODULE_INFO",
        "NEW_STREAMS",
        "OUTFILES",
        "REQUEST_MODULE_INFO",
        "REQUEST_STATUS",
        "STATUS",
        "TERMINATE"
        };

    char *command_state_strings[3] =
        {
        "IDLE",
        "POSTED",
        "ACTIVE"
        };

    struct addrinfo hints, 
                    *servinfo, 
                    *p;

    struct sockaddr_storage their_addr;
	socklen_t addr_len;

    struct header_tag *phdr;


                                    // things that are done but once
    if (first_time)
        {
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

                                    // get socket information for incoming port
        sprintf (sport, "%d", TO_DPLANE);
        if ((rc = getaddrinfo (NULL, sport, &hints, &servinfo)) != 0)
            {
            cprintf (0, "getaddrinfo: %s\n", gai_strerror (rc));
            cprintf (0, "fatal error, quitting\n");
            exit (1);
            }


                                    // get datagram socket file descriptor
                                    // loop through all the results trying
                                    // to get a file descriptor
        for (p = servinfo; p != NULL; p = p->ai_next) 
            {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1)
                {
                perror("dplane: socket call");
                continue;
                }
                                    // found a descriptor, bind to the socket
            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
                {
                close(sockfd);
                perror ("dplane: bind call");
                continue;
                }  
                                    // put socket into non-blocking mode
            if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) 
                {
                close(sockfd);
                perror ("dplane: fcntl call");
                continue;
                }  

            break;
            }

        if (p == NULL) 
            {
            cprintf (0, "dplane: failed to bind socket\n");
            return;
            }
                                    // free up the linked list
        freeaddrinfo (servinfo);
        first_time = FALSE;
        }                           // end of first time code

    if ((numbytes = recvfrom (sockfd, buff, MAXMESS-1 , 0, (struct sockaddr *) &their_addr, 
                              &addr_len)) == -1) 
        {
                                    // report all errors other than non-reception of data
        if (errno != EWOULDBLOCK)
            perror("recvfrom");
        }
    else                            // successful message read -- process it
        {
        phdr = (struct header_tag *) buff;
        if (glob.verbosity >= 3)    // print whole hex message at high verbosity levels
            {
            printf ("%s message received at %lf length %d code %d seq %d\n", 
                     command_strings[phdr->code], glob.tnow, numbytes, phdr->code, phdr->seqno);
            for (i=8; i<numbytes; i++)
                {
                printf ("%02x", buff[i]);
                if (i % 4 == 3)
                    printf (" ");
                if (i % 32 == 7)
                    printf ("\n");
                }
            printf ("\n");
            }
                                    // ensure command is idle before overwriting it
                                    // terminate allowed again, so forced flush possible
        if (glob.command[phdr->code].state != IDLE && phdr->code != TERMINATE)
            {
            cprintf (0, "new %s command ignored, as this command is still in %s state\n", 
                     command_strings[phdr->code], 
                     command_state_strings[glob.command[phdr->code].state]);
            }
        else if (phdr->code < 0 || phdr->code >= NUM_COMMANDS)
            {
            cprintf (0, "illegal command code %d, message ignored\n", phdr->code);
            }
        else
            {
            switch (phdr->code)
                {                   // copy appropriate message into global area
                case ABANDON_FILES:
                    memcpy (&glob.abandon_files, buff, sizeof (glob.abandon_files));
                    break;
                case ABORT:
                    memcpy (&glob.abort, buff, sizeof (glob.abort));
                    break;
                case CAPTURE:
                    memcpy (&glob.capture, buff, sizeof (glob.capture));
                    break;
                case NEW_STREAMS:
                    memcpy (&glob.new_streams, buff, sizeof (glob.new_streams));
                    break;
                case OUTFILES:
                    memcpy (&glob.outfiles, buff, sizeof (glob.outfiles));
                    break;
                case REQUEST_MODULE_INFO:
                    memcpy (&glob.request_module_info, buff, sizeof (glob.request_module_info));
                    break;
                case REQUEST_STATUS:
                    memcpy (&glob.request_status, buff, sizeof (glob.request_status));
                    break;
                case TERMINATE:
                    memcpy (&glob.terminate, buff, sizeof (glob.terminate));
                    break;
                default:
                    break;
                }
            glob.command[phdr->code].tlast = glob.tnow;
            glob.command[phdr->code].state = POSTED;
            }
        }
    }
