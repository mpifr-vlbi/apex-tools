#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "apecs_iface.h"

/**
 * apecs_connect(..)
 * @args host     The IP of the APECS control system
 * @args port     The UDP port of the APECS control system
 * @args timeout  Read/write timeout value in microseconds
 * @return  File descriptor for connection or NULL on errors
 */
apecs_fd_t* apecs_connect(const char* host, const int port, const long timeout_usecs, const int localport)
{
   apecs_fd_t* afd;
   int so_enable = 1;

   /* Custom descriptor */
   afd = (apecs_fd_t*)(malloc(sizeof(apecs_fd_t)));
   bzero(afd, sizeof(apecs_fd_t));

   /* UDP link to the remote server */
   afd->sd = socket(AF_INET,SOCK_DGRAM,0);
   afd->timeout_usecs = timeout_usecs;

   bzero(&(afd->servaddr), sizeof(afd->servaddr));
   afd->servaddr.sin_family = AF_INET;
   afd->servaddr.sin_addr.s_addr = inet_addr(host);
   afd->servaddr.sin_port = htons(port);

   if (setsockopt(afd->sd, SOL_SOCKET, SO_REUSEADDR, &so_enable, sizeof(so_enable)) != 0) {
      fprintf(stdout, "Setting SO_REUSEADDR failed : %s\n", strerror(errno));
   }

   /* UDP local listening port */
   afd->sdin = socket(AF_INET,SOCK_DGRAM,0);

   bzero(&(afd->listenaddr), sizeof(afd->listenaddr));
   afd->listenaddr.sin_family = AF_INET;
   afd->listenaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   afd->listenaddr.sin_port = htons(localport);

   if (bind(afd->sdin, &(afd->listenaddr), sizeof(afd->listenaddr)) != 0) {
      fprintf(stdout, "Bind to local port failed (%u) : %s\n", localport, strerror(errno));
   }

   return afd;
}


/**
 * apecs_send(..)
 * Sends a string to APECS and receives a reply, with timeout
 * @args afd       APECS file descriptor from apecs_connect()
 * @args request   Pointer to string to send to APECS
 * @args response  Pointer to buffer where to store the response from APECS
 * @args n_in      Allocated size in chars of the 'response' buffer
 * @return Length of the received response string
 */
ssize_t apecs_send(apecs_fd_t* afd, const char* request, char* response, size_t n_in)
{
   ssize_t n;

   /* send first ... */
   if (afd == NULL) { return -1; }
   if (request == NULL) { return -1; }
   sendto(afd->sd, request, strlen(request), 0,
          (struct sockaddr*)&(afd->servaddr),
          sizeof(afd->servaddr)
   );
   usleep(10000); // Daves magic number

   /* ... ask later (reply should be an "OK" or nothing) */
   n = apecs_recv_req(afd, request, response, n_in);
   return n;
}


/**
 * apecs_recv_req(..)
 * Receive a UDP packet from the APECS system
 * @args afd       APECS file descriptor from apecs_connect()
 * @args request   Pointer to string that was sent earlier to APECS
 * @args response  Pointer to buffer where to store the response from APECS
 * @args n_in      Allocated size in chars of the 'response' buffer
 * @return Length of the received response string
 */
ssize_t apecs_recv_req(apecs_fd_t* afd, const char* request, char* response, size_t n_in)
{
   ssize_t n;

   /* prepare mask and timeout */
   if (afd == NULL) { return -1; }
   FD_ZERO(&(afd->in_fdset));
   FD_SET(afd->sdin, &(afd->in_fdset));
   afd->timevalue.it_value.tv_usec = afd->timeout_usecs;
   afd->timevalue.it_value.tv_sec = 0;

   /* get response with timeout */
   memset(response, '\0', n_in);
   if (response==NULL || n_in<=0) { return 0; }
   n = select(FD_SETSIZE, &(afd->in_fdset), NULL, NULL, 
              &(afd->timevalue.it_value) ); /*timevalue gets overwritten!*/
   if(n != 0) {
      n = recvfrom(afd->sdin, response, n_in-1, 0, NULL, NULL);
      if (n <= 0) { 
         snprintf(response, n_in, "RECV_ERR for %s", request);
         // remove '\n' from end of ((char*)response)
         response[strlen(response) - 1] = '\0'; 
      } else {
         response[n]= '\0'; // in case UDP response string didn't contain terminating '\0'
      }
   } else {
      snprintf(response, n_in, "TIMEOUT(%.1fs) for %s", 1e-6*(afd->timeout_usecs), request);
      // remove '\n' from end of ((char*)request)
      response[strlen(response) - 1] = '\0'; 
   }
   return n;
}

ssize_t apecs_recv(apecs_fd_t* afd, char* response, size_t n_in)
{
   return apecs_recv_req(afd, "(original query not stored)\n", response, n_in);
}
