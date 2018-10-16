/* Function for DBBC Communication */
/* Functions for sockets and client in simple_socket.c */
#include <stdio.h> 
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "/usr2/fs/include/dpi.h"
#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"         /* fs-shared memory definition */
#include "/usr2/fs/include/shm_addr.h"      /* fs-shared memory pointer */
#include "/usr2/st/include/stcom.h"         /* st-shared memory definition */
#include "/usr2/st/include/stm_addr.h"      /* st-shared memory pointer */
#include "simple_socket.h"
#define SIMPLE_SOCKET_PROTOCOL SIMPLE_SOCKET_TCP    /* UDP or TCP */
/*#define SIMPLE_SOCKET_PROTOKOL SIMPLE_SOCKET_UDP*/
#define SIMPLE_SOCKET_PORT     1500

/*#define BUFSIZE 2048*/ /*Attn: BUFFSIZE client and server must be the same size */
#define DEBUG 0  /* 1 for additional printf, 0 no printf */
#define CONTROLFILE "/usr2/st/control/dbbc.ctl" /* D BBC controlfile */

#define DBBC_NAME "dbbc0" /* Name of dbbc used in st/conrol/dbbc.ctl */

/********************************************************/
int dbbc_com(int bufsize, char *send_buf, char *rcv_buf)
/********************************************************/

{
    int iClientMessageCounter = 0, i, ierr;
    char acClientBuffer[256];
    char ip_buf[64], port_buf[64];
    unsigned long ulClientReadBufferLength, ulPort;
    SimpleSocketType SClientSocket;
    SimpleSocketType SServerSocket;

    /*****  get IP-Nr. and Port-Nr.  from CONTROLFILE with find_control() *****/
    ierr=find_control(CONTROLFILE, DBBC_NAME, ip_buf, port_buf,0);/*0 No debug*/
    if (ierr != 0)
        {
        logitf("ST dbbc_com: [ERROR] find_control\a\n");
        return -4;
        }   
    sscanf(port_buf,"%ld",&ulPort);
    
    
    if (DEBUG) printf ("dbbc_com: Client started\n");

    /*! Open socket */
    if (DEBUG) printf ("Client open socket\n");

     if (uiClientOpenSocket (ip_buf,
                            ulPort,
                            SIMPLE_SOCKET_PROTOCOL,
                            &SClientSocket))   
     {
        logitf ("ST [ERROR] Client opens socket");
        return 1;
    }
    strcpy(acClientBuffer, send_buf);

    /*! Send message */
     if (DEBUG) printf ("Client sent msg: %s\n", acClientBuffer);
    if (uiSend (&SClientSocket, acClientBuffer, strlen(acClientBuffer)+1))
    {
        if (DEBUG) printf ("ST Client [ERROR] Client send msg: %s\n", acClientBuffer);
        return 1;
    }

    if (DEBUG) printf ("Client receive msg: ");
    /*! Receive acknowledge message */
    ulClientReadBufferLength = 256;
   *(acClientBuffer) = '\0';
    
    if (uiReceive (&SClientSocket, acClientBuffer, &ulClientReadBufferLength))
    {
        if (DEBUG) printf ("\nClient [ERROR] Client receive msg\n");
        return 1;
    }
    if (DEBUG) printf ("receive form server; %s\n", acClientBuffer);
    strcpy(rcv_buf, acClientBuffer);
    /*! Sleep until next send */
    /*if (DEBUG) printf ("Client sleeps 1 sec\n");*/
    /*sleep (1);*/

    if (DEBUG) printf ("Client close socket\n");

    if (uiCloseSocket (&SClientSocket))
    {
        printf ("[ERROR] Client closes socket\n");
        return 1;
    }

    return 0;
}
 
