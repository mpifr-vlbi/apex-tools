#ifndef APECS_IFACE_H
#define APECS_IFACE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/** APECS socket descriptor struct */
typedef struct apecs_fd_tt {
   struct sockaddr_in servaddr;
   struct sockaddr_in listenaddr;
   struct itimerval timevalue;
   fd_set in_fdset;
   long timeout_usecs;
   int sd;
   int sdin;
} apecs_fd_t;

apecs_fd_t* apecs_connect(const char* host, const int port, const long timeout_usecs, const int localport);
ssize_t apecs_send(apecs_fd_t* afd, const char* request, char* response, size_t n_in);
ssize_t apecs_recv_req(apecs_fd_t* afd, const char* request, char* response, size_t n_in);
ssize_t apecs_recv(apecs_fd_t* afd, char* response, size_t n_in);

#endif // APECS_IFACE_H
