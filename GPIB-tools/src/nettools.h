#ifndef NETTOOLS_H
#define NETTOOLS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

void err(const char *, ...);

/** Opens a TCP connection to a server
 * Also sets a socket operation timeout.
 */
int tcp_client(const char *servIP, const char *servPort, int timeout_secs);

/** Sends a message 
 * @return number of bytes sent
 */
int sendMessage(int sockfd, char *buff);

/** Try to read message from GPIB.
 * @return length of received message or -1 on errors
 */
int readMessage(int sockfd, char *buff, size_t maxlen);

/** Try to read message from Lantronix.
 * @return length of received message or -1 on errors
 */
int readMessage2(int sockfd, char *buff, size_t maxlen);

#endif
