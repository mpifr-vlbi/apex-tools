/* Function for DBBC Communication */
/* Functions for sockets and client in simple_socket.c */
#include <stdio.h> 
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "/usr2/fs/include/dpi.h"
#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"         /* fs-shared memory definition */
#include "/usr2/fs/include/shm_addr.h"      /* fs-shared memory pointer */
#include "/usr2/st/include/stcom.h"         /* st-shared memory definition */
#include "/usr2/st/include/stm_addr.h"      /* st-shared memory pointer */

/*#define BUFSIZE 2048*/ /*Attn: BUFFSIZE client and server must be the same size */
#define DEBUG 1  /* 1 for additional printf, 0 no printf */
#define CONTROLFILE "/usr2/st/control/dbbc.ctl" /* D BBC controlfile */

#define DBBC_NAME "dbbc0" /* Name of dbbc used in st/conrol/dbbc.ctl */

/********************************************************/
int dbc_com(int bufsize, char *send_buf, char *rcv_buf)
/********************************************************/

{
    int sockfd;
    int len, i_len,ierr;
    struct sockaddr_in address;
    int result;
    char out_buf[256], in_buf[256], ip_buf[64], port_buf[64];
    unsigned short ulPort;

    /*****  get IP-Nr. and Port-Nr.  from CONTROLFILE with find_control() *****/
    
    
    ierr=find_control(CONTROLFILE, DBBC_NAME, ip_buf, port_buf,0);/*0 No debug*/
    if (DEBUG) printf("DBBC_NAME: %s, ip_buf %s, port_buf %s\n",DBBC_NAME, ip_buf, port_buf); 


    if (ierr != 0)
        {
        logitf("ST dbbc_com: [ERROR] find_control\a\n");
        return -4;
        }   
    sscanf(port_buf,"%ld",&ulPort);
    
    
    if (DEBUG) printf ("dbbc_com: start Client \n");

    /* 2.  make socket for client, AF_INET => UNIX-Socket, SOCK_STREAM=>TCP/IP 
    protocol 0=> standard*/
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
    /*  3. make Server_Address and Port */
    address.sin_family = AF_INET;
     
      address.sin_port = ulPort;
      address.sin_addr.s_addr = inet_addr(ip_buf);      
/*      address.sin_addr.s_addr = inet_addr("127.0.0.1");
      address.sin_port = 1500;*/
      len = sizeof(address);
      
      printf("sizeof(address): %d\n",len);
      
   /* 4. connect Socket with Server Socket */  
    result = connect(sockfd, (struct sockaddr *)&address, len);
    
      if(result == -1)  {
         perror("perror oops: client1");
         return 1;
      }
   /* now able to read/write via sockfd */
   
     strcpy(out_buf, send_buf);
     i_len = strlen(out_buf);
     ierr=write(sockfd, &out_buf,i_len);
     if (ierr < 0) {
        if (DEBUG) printf ("client write error: %d\n", ierr);
        close(sockfd);
        return ierr;
     }   
     /* server should answer */   
        
     ierr=read(sockfd, &in_buf, sizeof in_buf);
       if (ierr < 0) {
           if (DEBUG) printf ("client read error: %d\n", ierr);
           close(sockfd); 
           return ierr;
           
       }
       strcpy(rcv_buf, in_buf);     
       close(sockfd);
      return 0;
      
}
