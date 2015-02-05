// performs any initialization necessary at startup
//
// initial version                       rjc 2012.7.20

#include "dplane.h"

extern struct global_tag glob;
extern unsigned short int lookup[65536];

pthread_t sthred[MAXSTREAM];
pthread_t pthred;
pthread_t fthred[NFILES];

void initialize ()
    {
    int i,
        j,
        rc,
        page_size,
        num_pages,
        od, ev,                     // will contain odd and even bits of a word
        found;

    u_char *pmem;

    struct ifaddrs *ifap,
                   *pif;

    struct sockaddr_in *sa;
    char *addr;

                                    // prototypes
    void *spooler (void *);
    void *phylum (void *);
    void *writer (void *);


                                    // initialize global memory
    glob.terminate_phylum   = FALSE;
    glob.terminated_phylum  = FALSE;
    glob.close_streams      = FALSE;

    for (i=0; i<MAXSTREAM; i++)
        {
        glob.stream[i].terminate_spooler  = FALSE;
        glob.stream[i].terminated_spooler = FALSE;
        glob.stream[i].lead = 0;
        glob.stream[i].dev[0] = 0;  // null out input devices
        glob.stream[i].rb_state = QUIESCENT;
        glob.stream[i].npack_good = 0;
        glob.stream[i].npack_wrong_psn = 0;
        glob.stream[i].npack_wrong_len = 0;
        glob.stream[i].nbytes = 0;
        for (j=0; j<NFILES; j++)
            glob.stream[i].fpool[j] = FALSE;
        }

    for (i=0; i<NFILES; i++)
        {
        glob.sfile[i].file_open = FALSE;
        glob.sfile[i].filename[0] = '\0';
        glob.sfile[i].nbytes = 0;
        }
    
    glob.capture.close_on_drain = FALSE;

    glob.numproc = sysconf (_SC_NPROCESSORS_CONF);
                                    // request and configure large ring buffer
    page_size = sysconf (_SC_PAGE_SIZE);
    num_pages = sysconf (_SC_PHYS_PAGES);
    glob.rb_total_size = (long int) page_size * num_pages - RESERVED_SPACE;
                                    // restrict useable buffer to 2GB..200GB
    if (glob.rb_total_size < 2000000000L || glob.rb_total_size > 200000000000L)
        {
        printf ("Error getting RAM size; page_size %d num_pages %d rb_total_size %ld\n",
                page_size,  num_pages, glob.rb_total_size);
        exit (-1);
        }

    pmem = (u_char *) malloc (glob.rb_total_size);
    if (pmem == NULL)
        {
        printf ("Error allocating large ring buffer memory of %ld bytes\n",
                glob.rb_total_size);
        exit (-1);
        }
    else
        cprintf (2, "ring buffer origin %p size %ld\n", pmem, glob.rb_total_size);
                                    // now lock rb into physical memory
    rc = mlock (pmem, glob.rb_total_size);
    if (rc < 0)
        {
        printf ("WARNING!!! The ring buffer couldn't be locked into RAM\n"
                "Perhaps there is a permissions problem\n");
        perror ("dplane");
        }
    printf ("ring buffer total size %ld bytes\n", glob.rb_total_size);
                                    // globalize the pointer to the ring buffer
    glob.porb = pmem;

                                    // generate the mk5 lookup table, which maps
                                    // 00:01:10:11 to 00:10:01:11, for all 2 bit pairs
    for (i=0; i<65536; i++)
        {
        od = i & 0xaaaa;            // odd bits
        ev = i & 0x5555;            // even bits
                                    // and now the magic part...
        lookup[i] = i ^ ((od^(ev<<1)) | (ev^(od>>1)));
        }

                                    // determine and store the local broadcast lan mask
    rc = getifaddrs (&ifap);
    if (rc)
        {
        cprintf (1, "error trying to determine local lan mask\n");
        perror ("getifaddrs call");
        }
                                    // search through linked-list for eth0
                                    // harmless default
    strcpy (glob.bcast_addr, "255.255.255.255");
    found = FALSE;
    for (pif = ifap; pif != NULL; pif = pif->ifa_next)
        {
        if (strncmp (pif->ifa_name, "eth0", 4) == 0)
            {
            if (pif->ifa_addr->sa_family == AF_INET)
                {
                                    // find ip address
                sa = (struct sockaddr_in *) pif->ifa_addr;
                addr = inet_ntoa (sa->sin_addr);
                strncpy (glob.eth0_ip, addr, 16);
                cprintf (3, "eth0's ip address: %s\n", glob.eth0_ip);
                strncpy (glob.bcast_addr, addr, 16);
                                    // now access broadcast address
                sa = (struct sockaddr_in *) pif->ifa_ifu.ifu_broadaddr;
                addr = inet_ntoa (sa->sin_addr);
                strncpy (glob.bcast_addr, addr, 16);
                cprintf (3, "broadcast addr: %s\n", glob.bcast_addr);
                }
            }
        if (found)                  // stop looking?
            break;
        }

                                    // fire off stream reading and writing threads
    for (i=0; i<MAXSTREAM; i++)     // multiple spooling threads
        pthread_create (&sthred[i], NULL, spooler, (void *) (long) i);
                                    // single thread for master writer phylum
    pthread_create (&pthred, NULL, phylum,  (void *) (long) i);
                                    // individual threads for each potential file
    for (i=0; i<NFILES; i++)
        pthread_create (&fthred[i], NULL, writer,  (void *) (long) i);
    }
