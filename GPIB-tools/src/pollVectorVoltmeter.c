/* ALR 23.09.10
 * based on ~/EB/1mm/APEX/DownconverterComms/pingCounters.c
 *
 * Client that talks to HP 8508A vector voltmeter
 *  uses TCP socket to talk to Prologix media converter at port 1234
 *
 * Based on example code by Richard Stevens 1998, p295
 *  "UNIX Network Programming: Networking APIs: Sockets and XTI" Prentice Hall
 *  http://www.kohala.com/start/
 *
 * Compile with:  'gcc -Wall -O0 pollVectorVoltmeter.c -o pollVectorVoltmeter'
 *
 */

#include "nettools.h"
#include "version.h"

#define MAXLINE 4096          /* max text line length */
#define OUTFILE "pollVectorVoltmeter.out"

#define _PRINT 1

// TCP connection settings
char* _ipaddr;
char* _port;
int _timeout_secs;
 
int main(int argc, char **argv) {
  int sockfd;
  char recvline[MAXLINE + 1];
  //int errn;
  //socklen_t salen;
  //struct sockaddr *sa;
  char buff[MAXLINE];
  int i, nMeasurements, samplePeriod;
  FILE *fp;
  float amp, phase;
  time_t timerStart, timerNow;
  double elapsedTime;

/* Print a helpful message if the number of command line arguments is wrong */

  printf("\n*** pollVectorVoltmeter version %s ***\n\n", SOFTWARE_VERSION);
  if (argc != 5)
    err("usage: pollVectorVoltmeter <IPaddress> <port> <number of measurements> <sample period / s>");

  //=============================================================================================
  //=== CONFIGURE THE GPIB DEVICE                                                             ===
  //=== We assume there are no TCP timeouts etc during the configuration phase                ===
  //=============================================================================================

  /* Get parameters */
  _ipaddr = strdup(argv[1]);
  _port = strdup(argv[2]);
  _timeout_secs = 10;
  nMeasurements = atoi(argv[3]);
  samplePeriod = atoi(argv[4]);

  /* Create a TCP socket */
  sockfd = tcp_client(_ipaddr, _port, _timeout_secs);
  if (sockfd < 0) {
    snprintf(buff, sizeof(buff), "socket error: %s", strerror(errno));
    err(buff);
  }

  /* Configure GPIB controller with device address of the vector voltmeter */
  sendMessage(sockfd, "++addr 8\n");

  /* Query the GPIB controller to confirm device address setting */
  sendMessage(sockfd, "++addr\n");
  readMessage(sockfd, recvline, MAXLINE);

  /* Query the vector voltmeter identification */
  sendMessage(sockfd, "*IDN?\n");
  readMessage(sockfd, recvline, MAXLINE);

  /* Configure the vector voltmeter */
  sendMessage(sockfd, "TRIGGER:SOURCE FREE\n");
  sendMessage(sockfd, "FORMAT LIN\n");
  sendMessage(sockfd, "SENSE TRANSMISSION\n");

  /* Check for errors */
  sendMessage(sockfd, "SYSTEM:ERR?\n");
  readMessage(sockfd, recvline, MAXLINE);
  if (recvline[0]!='0') {
    // zero character ("0,NO ERROR") means everything is ok
    // non-zero is an error ("-110,CMD HEADER ERROR, ...")
    printf("Request 'SYSTEM:ERR?' reply was '%s', error?\n", recvline);
  }


  //=============================================================================================
  //=== MEASUREMENT PHASE                                                                     ===
  //=== On TCP timeouts, we reconnect                                                         ===
  //=============================================================================================

  fp = fopen(OUTFILE, "w");
  if (fp == NULL) {
    printf("Trouble opening output file %s\n", OUTFILE);
    exit(1);
  }

  fprintf(fp, "--------------------------------\n");
  fprintf(fp, "Elapsed      Amp         Phase\n");
  fprintf(fp, "Time / s     / V         / deg\n");
  fprintf(fp, "--------------------------------\n");
  if (_PRINT) {
    printf("--------------------------------\n");
    printf("Elapsed      Amp         Phase\n");
    printf("Time / s     / V         / deg\n");
    printf("--------------------------------\n");
  }

  time(&timerStart);
  for (i = 1; i <= nMeasurements; i++) {
    int rc = 0;

    if (_PRINT == 1)
      printf("\nMeasurement %d of %d\n", i, nMeasurements);

    /* rc = sendMessage(sockfd, "*TRG\n"); */
    rc = sendMessage(sockfd, "FETCH?\n");
    rc = readMessage(sockfd, recvline, MAXLINE);

    /* on errors, do TCP reconnect */
    if (rc < 0) {
 
       shutdown(sockfd, SHUT_RDWR);
       close(sockfd);
       printf("Socket error during read! (%s)\n", strerror(errno));

       /* Infinite reconnect reattempts */
       while (1) {
          printf("Waiting 5 sec until reconnect attempt...\n");
          sleep(5);
          printf("Reconnecting...\n");
          sockfd = tcp_client(_ipaddr, _port, _timeout_secs);
          if (sockfd >= 0) {
             printf("Reconnect SUCCESS!\n");
             break;
          }
       }

       /* Jump back to "FETCH?" */
       continue;
    }

    /* Handle the data point */
    sscanf(recvline, "%f,%f", &amp, &phase);
  
    time(&timerNow);
    elapsedTime = difftime(timerNow, timerStart);
    fprintf(fp, "%.0lf        %10.3e   %10.3e\n", elapsedTime, amp, phase);
    if (_PRINT)
      printf("%.0lf        %10.3e   %10.3e\n", elapsedTime, amp, phase);
    fflush(fp);
    sleep(samplePeriod);
  }

  /* Normal completion */
  close(sockfd);
  fclose(fp);
  exit(0);
}
