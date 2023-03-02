/**********************************************************
  $Header$
***********************************************************/

/**********************************************************
 *
 * file: cid.c 
 *
 * Open a UDP socket
 * Listen to commands coming in via the socket
 *
 **********************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>

#include "yacc_counters.h"
#include "cid_counters.h"
#include <pthread.h>

#include <iostream>
#include "ace/Log_Msg.h"
#include "ace/SOCK_Dgram.h"
#include "ace/INET_Addr.h"

#define UDP_PORT 17480
 
using namespace std;

ACE_SOCK_Dgram dgram;
ACE_INET_Addr remote;

int tcpPortCounters=1234;
char tcpHostCounters[30];
char deviceName[30];

char *pname;
int lineno;
int my_udp_port=UDP_PORT;
char my_fifo_portfile[50];
   
double cmdListVal[400];
int cmdListLen;


int main(int argc, char *argv[])
{
    
    int quittime;

/* Storage for ID of thread to talk with counters */

    pthread_t apexCountersDataThread;

    quittime = 0;
    if (argc) pname = argv[0];

    sprintf(deviceName,"Counters");


    int i;
    for(i=1;i<argc;i++){
      if (!strcmp(argv[i],"-help")|!strcmp(argv[i],"--help")){
	cout << "--------------------------------------------------------------\n"; 
	cout << "syntax: apexVLBICountersControl -help --help -port xxxx\n";
	cout << "--------------------------------------------------------------\n";
        cout << "-port xxxxxxx ACS Component UDP port number\n";
        cout << "-nameIFV1 xxxxxxx ACS Component UDP device name\n";
	cout << "-portIFV1 xxxxxxx VLBI counters TCP port \n";
	cout << "-hostIFV1 xxxxxxx VLBI counters ip-no or hostname  \n";
	cout << "uses shared library libACE.so* set proper LD_LIBRARY_PATH\n";
	cout << "(-)-help prints out this message\n";
	cout << "--------------------------------------------------------------\n"; 
	_exit(0);
      }

      if (!strcmp(argv[i],"-port") && i < argc-1){ 
	my_udp_port=atol(argv[i+1]);
      }
      if (!strcmp(argv[i],"-nameCounters") && i < argc-1){
         sprintf(deviceName,"%s",argv[i+1]);
      }


      if (!strcmp(argv[i],"-portCounters") && i < argc-1){ 
	tcpPortCounters=atoi(argv[i+1]);
      }
      if (!strcmp(argv[i],"-hostCounters") && i < argc-1){
	sprintf(tcpHostCounters,"%s\n",argv[i+1]);
      }
    }


/* Create thread to retrieve data from counters */

    pthread_create(&apexCountersDataThread,NULL,apexCountersCommsThread,NULL);

    ACE_INET_Addr local (my_udp_port);

    cout << "The UDP port to the ACS Component: " << my_udp_port << "\n";


/* Open datagram for receiving */

    if (dgram.open (local) == -1) {
	ACE_ERROR_RETURN ((LM_ERROR,
			   "%p\n",
			   "open"),
			  -1);
    }


    ACE_DEBUG ((LM_DEBUG, "(%P|%t) starting up server daemon\n"));


/* Configure both counters first time through */

    CountersConfigure(CONFIGURE, 3);


    for (;;) {
        lineno = 0;
        if (yyparse() != 0) quittime = 1;;
        close(0);
	if (quittime) _exit(0);
    }
}
