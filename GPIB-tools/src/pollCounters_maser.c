/* ALR 26.01.11
 * based on ~/EB/1mm/APEX/DownconverterComms/pingVectorVoltmeter.c
 *
 * Client that talks to HP 53131A universal counter
 *  uses TCP socket to talk to Prologix media converter at port 1234
 *
 * Based on example code by Richard Stevens 1998, p295
 *  "UNIX Network Programming: Networking APIs: Sockets and XTI" Prentice Hall
 *  http://www.kohala.com/start/
 *
 * Compile with:  'gcc pollCounters.c'
 *
 */

#include "nettools.h"
#include "version.h"
#include <time.h>

#define MAXLINE 4096          /* max text line length */
#define OUTFILE "pollCounters_maser.out"

// TCP connection settings
char* _ipaddr;
char* _port;
int _timeout_secs;

const char* _device_names[] = {
  "none", "none", "none",
  "gps-fmout", "gps-maser", "none"
};

int main(int argc, char **argv) {
  int sockfd;
  char recvline[MAXLINE + 1];
  char buff[MAXLINE];
  int i, nMeasurements, samplePeriod;
  FILE *fp;
  double timeIntervals[10];
  time_t timerStart, timerNow;
  double elapsedTime;
  int device;
  const int base_device = 4;
  const int max_device = 4;
  int index;
  int logformat = 0;

  char *PrologixConfigCommands[] = {
    "++savecfg 0\n",
    "++auto 0\n",
    "++mode 1\n",
    "++read_tmo_ms 3000\n",
    "++ifc\n",
    "++eot_enable 0\n",
    "++eos 3\n"
  };
  int nPrologixConfigCommands = 7;

  /* Print a helpful message if the number of command line arguments is wrong */
  printf("\n*** pollCounters version %s ***\n\n", SOFTWARE_VERSION);

  if (argc != 5 && argc != 6)
    err("usage: pollCounters <IPaddress> <port> <number of measurements> <sample period / s> [0=old log format|1=FieldSystem format|2=unix time]");

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
  if (argc == 6) {
    logformat = atoi(argv[5]);
  }
  
  /* Create a TCP socket */
  sockfd = tcp_client(argv[1], argv[2], 30);
  if (sockfd < 0) {
    snprintf(buff, sizeof(buff), "socket error: %s", strerror(errno));
    err(buff);
  }

  /* Inform user */
  printf("Configuring the Prologix GPIB-LAN adaptor: ");
  fflush(stdout);

  /* Send configuration commands to the Prologix media converter */
  for (index = 0; index < nPrologixConfigCommands; index++)
    sendMessage(sockfd, PrologixConfigCommands[index]);
  printf("done.\n");

  /* Configure each counter */
  for (device = base_device; device <= max_device; device++) {

    /* Choose GPIB controller with device address of the counter */
    sprintf(buff, "++addr %u\n", device);
    sendMessage(sockfd, buff);

    /* Query the GPIB controller to confirm device address setting */
    sendMessage(sockfd, "++addr\n");
    readMessage(sockfd, recvline, MAXLINE);

    /* Query the counter identification */
    sendMessage(sockfd, "*IDN?\n");
    readMessage(sockfd, recvline, MAXLINE);

    printf("\n");
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

  if (logformat==0) {
    fprintf(fp, "-----------------------------------------------\n");
    fprintf(fp, "Elapsed      Time Interval (GPIB#3)  Time Interval (GPIB#4)\n");
    fprintf(fp, "Time / s          / s                     / s\n");
    fprintf(fp, "--------------------------------\n");
  } else if (logformat==1) {
    fprintf(fp, "2012.001.12:00:00.00\"pollCounters 1.04 in FieldSystem format\n");
  } else {
    fprintf(fp, "pollCounters 1.04 log in Unix Timestamp format\n");
  }

  time(&timerStart);
  for (i = 1; i <= nMeasurements || (nMeasurements==-1); i++) {

    for (device = base_device; device <= max_device; device++) {

      int rc;
      timeIntervals[device] = 0.0f;

      printf("Measurement %d of %d @%u:   ", i, nMeasurements, device);
      fflush(stdout);

      sprintf(buff, "++addr %u\n", device);
      sendMessage(sockfd, buff);
      rc = sendMessage(sockfd, "READ?\n");
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

        /* Jump back to data "READ?" */
        continue;
      }
    
      /* Handle the data point */
      sscanf(recvline, "%lf", &timeIntervals[device]);

    }

    time(&timerNow);
    elapsedTime = difftime(timerNow, timerStart);
    if (logformat == 0) { 
      fprintf(fp, "%.0lf           ", elapsedTime);
      for (device = base_device; device <= max_device; device++) {
        fprintf(fp, "%.9le          ", timeIntervals[device]);
      }
      fprintf(fp, "\n");
    } else if (logformat ==1) {
        // FieldSystem format: 2012.057.04:30:06.89/gps-fmout/9.8176e-05
        struct tm* ts = gmtime(&timerNow);
        for (device = base_device; device <= max_device; device++) {
          fprintf(fp, "%04u.%03u.", (1900+ts->tm_year), (ts->tm_yday+1));
          fprintf(fp, "%02u:%02u:%02u.00", ts->tm_hour, ts->tm_min, ts->tm_sec);
          fprintf(fp, "/%s/%.9le\n", _device_names[device], timeIntervals[device]);
        }
    } else {
      fprintf(fp, "%zu           ", (size_t)timerNow);
      for (device = base_device; device <= max_device; device++) {
        fprintf(fp, "%.9le          ", timeIntervals[device]);
      }
      fprintf(fp, "\n");
    }
    printf("Elapsed time: %.0lf s\t  Time now: %zu s\t ", elapsedTime, (size_t)timerNow);
    for (device = base_device; device <= max_device; device++) {
      printf("%s:  %.9le s\t", _device_names[device], timeIntervals[device]);
    }
    printf("\n");
    fflush(fp);
    sleep(samplePeriod);
  }

  /* Normal completion */
  printf("\n");
  close(sockfd);
  fclose(fp);
  exit(0);
}
