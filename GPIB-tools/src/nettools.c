/*
 * TCP connection library
 * To be used to query GPIB devices behind an GPIB-Ethernet converter
 */

#include "nettools.h"

void err(const char *fmt, ...)
{
  fprintf(stderr, "%s\n", fmt);
  exit(1);
}

int tcp_client(const char *servIP, const char *servPort, int timeout_secs)
{
  int sockfd, errn, port;
  struct sockaddr_in servaddr;
  char buff[1024];
  struct timeval tv;

  port =atoi(servPort);

  /* Create a TCP socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    snprintf(buff, sizeof(buff), "socket error: %s", strerror(errno));
    err(buff);
  }

  /* Set up the server's address structure */
  bzero(&servaddr, sizeof(servaddr));   /* initialize server address to zero */
  servaddr.sin_family = AF_INET;        /* internet address is IPv4 format */
  servaddr.sin_port   = htons(port);   /* set port number for our server */


  /* Convert address from presentation format eg "134.104.28.45" given on the
   * command line to numeric format and insert it into the server address
   * structure 
   */
  errn = inet_pton(AF_INET, servIP, &servaddr.sin_addr);
  if (errn <= 0)
    err("inet_pton error for %s", servIP);

  /* Establish connection with server */
  errn = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
  if (errn < 0) {
    snprintf(buff, sizeof(buff), "connect error: %s", strerror(errno));
    printf("%s\n", buff);
    close(sockfd);
    return -1;
  }

  /* Set specified timeout for socket read and write operations */
  tv.tv_sec = timeout_secs;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  tv.tv_sec = timeout_secs;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

  return(sockfd);
}


int sendMessage(int sockfd, char *buff)
{
  int messageLength, n;

  #ifdef VERBOSE
  printf("Request: %s\n", buff);
  #endif
  
  /* Write the string to the socket */
  messageLength = strlen(buff);
  n = write(sockfd, buff, messageLength);

  /* If not all characters were sent then report an error */
  if (n != messageLength) {
    snprintf(buff, sizeof(buff), "write error: %s", strerror(errno));
  }
  return n;  
}


int readMessage(int sockfd, char *recvline, size_t maxlen)
{
  int n;

  /* Instruct GPIB that we want to read */
  sendMessage(sockfd, "++read eoi\n"); 

  /* Perform the read */
  n = recvfrom(sockfd, recvline, maxlen, 0, NULL, NULL);
  if (n > 0) {
    recvline[n] = '\0';
    #ifdef VERBOSE
    printf("Reply: %s", recvline);
    #endif
    return n;
  } else {
    #ifdef VERBOSE
    printf("Reply: (none)");
    #endif
  }

  return -1;
}


int readMessage2(int sockfd, char *recvline, size_t maxlen)
{
  int n;

  /* Perform the read */
  n = recvfrom(sockfd, recvline, maxlen, 0, NULL, NULL);
  if (n > 0) {
    recvline[n] = '\0';
    #ifdef VERBOSE
    printf("Reply: %s", recvline);
    #endif
    return n;
  } else {
    #ifdef VERBOSE
    printf("Reply: (none)");
    #endif
  }

  return -1;
}
