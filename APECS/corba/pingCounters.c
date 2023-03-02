/* ALR 23.09.10
 * based on ~/EB/1mm/APEX/DownconverterComms/pingVectorVoltmeter.c
 *
 * Client that talks to HP 53131A universal counter
 *  uses TCP socket to talk to Prologix media converter at port 1234
 *
 * Based on example code by Richard Stevens 1998, p295
 *  "UNIX Network Programming: Networking APIs: Sockets and XTI" Prentice Hall
 *  http://www.kohala.com/start/
 *
 * Compile with:  'gcc pingCounters.c'
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>           /* for socket close() */
#include <time.h>

#define MAXLINE 4096          /* max text line length */
#define SA struct sockaddr

#define _DEBUG 0

#define OUTFILE "pingCounters.out"

void err(const char *, ...);
int tcp_client(const char *servIP, const char *servPort);
void sendMessage(int sockfd, char *buff);
void readMessage(int sockfd, char *buff);

int main(int argc, char **argv) {
  int sockfd, errn;
  unsigned char recvline[MAXLINE + 1];
  socklen_t salen;
  struct sockaddr *sa;
  unsigned char buff[MAXLINE];
  int i, n, nMeasurements, samplePeriod;
  FILE *fp;
  double timeInterval;
  time_t timerStart, timerNow;
  double elapsedTime;


/* Print a helpful message if the number of command line arguments is wrong */

  if (argc != 5)
    err("usage: pingCounters <IPaddress> <port> <number of measurements> <sample period / s>");


  printf("\n*** pingCounters ***\n\n");


/* Create a TCP socket */

  sockfd = tcp_client(argv[1], argv[2]);
  if (sockfd < 0) {
    snprintf(buff, sizeof(buff), "socket error: %s", strerror(errno));
    err(buff);
  }


/* Configure GPIB controller with device address of the counter */

  sendMessage(sockfd, "++addr 4\n");


/* Query the GPIB controller to confirm device address setting */

  sendMessage(sockfd, "++addr\n");
  readMessage(sockfd, recvline);


/* Query the counter identification */

  sendMessage(sockfd, "*IDN?\n");
  readMessage(sockfd, recvline);


/* Configure the counter */

  sendMessage(sockfd, ":SENS:ROSC:SOURCE:AUTO ON\n");
  sendMessage(sockfd, ":INP1:ATT 1\n");
  sendMessage(sockfd, ":INP1:COUP DC\n");
  sendMessage(sockfd, ":EVEN1:LEVEL 1.0\n");
  sendMessage(sockfd, ":EVEN1:SLOP NEG\n");
  sendMessage(sockfd, ":INP2:ATT 1\n");
  sendMessage(sockfd, ":INP2:COUP DC\n");
  sendMessage(sockfd, ":EVEN2:LEVEL 1.0\n");
  sendMessage(sockfd, ":EVEN2:SLOP NEG\n");
  sendMessage(sockfd, ":FUNC 'TINT'\n");
  sendMessage(sockfd, ":TINT:ARM:STAR:SOUR IMM\n");
  sendMessage(sockfd, ":TINT:ARM:STOP:SOUR IMM\n");
  sendMessage(sockfd, ":INIT:CONT ON\n");

  /*
  sendMessage(sockfd, "FETCH:TINT?\n");
  sleep(2);
  readMessage(sockfd, recvline);
  */

/* Check for errors */

  sendMessage(sockfd, "SYSTEM:ERR?\n");
  readMessage(sockfd, recvline);


/* Make measurements */

  nMeasurements = atoi(argv[3]);

  samplePeriod = atoi(argv[4]);

  fp = fopen(OUTFILE, "w");
  if (fp == NULL) {
    printf("Trouble opening output file %s\n", OUTFILE);
    exit(1);
  }

  fprintf(fp, "--------------------------------\n");
  fprintf(fp, "Elapsed      Time Interval\n");
  fprintf(fp, "Time / s          / s\n");
  fprintf(fp, "--------------------------------\n");

  time(&timerStart);
  for (i = 1; i <= nMeasurements; i++) {
    printf("\nMeasurement %d of %d\n", i, nMeasurements);
    sendMessage(sockfd, "FETCH:TINT?\n");
    readMessage(sockfd, recvline);
    sscanf(recvline, "%lf", &timeInterval);
  
    time(&timerNow);
    elapsedTime = difftime(timerNow, timerStart);
    fprintf(fp, "%.0lf           %.9le\n", elapsedTime, timeInterval);
    sleep(samplePeriod);
  }


/* Normal completion */

  close(sockfd);
  fclose(fp);
  exit(0);
}


void err(const char *fmt, ...)
{
  fprintf(stderr, "%s\n", fmt);
  exit(1);
}


int tcp_client(const char *servIP, const char *servPort)
{
  int sockfd, errn, port;
  struct sockaddr_in servaddr;
  unsigned char buff[MAXLINE];
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
 * structure */

  errn = inet_pton(AF_INET, servIP, &servaddr.sin_addr);
  if (errn <= 0)
    err("inet_pton error for %s", servIP);


/* Establish connection with server */

  errn = connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
  if (errn < 0) {
    snprintf(buff, sizeof(buff), "connect error: %s", strerror(errno));
    err(buff);
  }


/* Set 30 s timeout for socket read operations */

  tv.tv_sec = 30;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  return(sockfd);
}


void sendMessage(int sockfd, char *buff)
{
  int messageLength, n;
  unsigned char recvline[MAXLINE + 1];

  
/* Write the string to the socket */

  printf("Send:  %s", buff);
  messageLength = strlen(buff);
  n = write(sockfd, buff, messageLength);


/* If not all characters were sent then report an error */

  if (n != messageLength) {
    snprintf(buff, sizeof(buff), "write error: %s", strerror(errno));
    err(buff);
  }
}


void readMessage(int sockfd, char *recvline)
{
  int n;

/* Receive the reply */

  printf("Reply: ");
  n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
  if (n > 0) {
    recvline[n] = '\0';
    printf("%s", recvline);
  }
  else {
    printf("Comm error\n");
    close(sockfd);
    exit(1);
  }
}
