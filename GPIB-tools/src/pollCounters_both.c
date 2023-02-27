/*
 * Poll a pair of HP 53131A universal counters behing a Prologix GPIB-Ethernet converter.
 * Logs values periodically. Does not initialize the counters.
 *
 # Assumes the Proligix has IP 10.0.2.93 and port 1234.
 */

#include "nettools.h"
#include "version.h"
#include <time.h>

#define MAXLINE 4096
#define OUTFILE "pollCounters_both.out"

// TCP connection settings
const char* _ipaddr = "10.0.2.93";
const char* _port = "1234";
const int _timeout_secs = 10;

const char* _device_names[] = {
  "none", "none", "none",
  "gps-fmout", "gps-maser", "none"
};


int main(int argc, char **argv)
{
  int sockfd;
  char recvline[MAXLINE + 1];
  char buff[MAXLINE];

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

  int samplePeriod_sec;
  double timeIntervals[10];

  time_t timerStart, timerNow;
  double elapsedTime;
  int iter = 0;

  FILE *fp;

  int device;
  const int base_device = 3;
  const int max_device = 4;

  int i;

  if (argc != 2) {
    err("usage: pollCounters_both <sample period [seconds]>");
  }

  //=============================================================================================
  //=== CONFIGURE THE GPIB DEVICE                                                             ===
  //=== We assume there are no TCP timeouts etc during the configuration phase                ===
  //=============================================================================================

  /* Get parameters */
  samplePeriod_sec = atoi(argv[1]);

  /* Create a TCP socket */
  sockfd = tcp_client(_ipaddr, _port, _timeout_secs);
  if (sockfd < 0) {
    snprintf(buff, sizeof(buff), "socket error: %s", strerror(errno));
    err(buff);
  }

  /* Inform user */
  printf("Configuring the Prologix GPIB-LAN adaptor...\n");
  fflush(stdout);

  /* Send configuration commands to the Prologix media converter */
  for (i = 0; i < nPrologixConfigCommands; i++) {
    sendMessage(sockfd, PrologixConfigCommands[i]);
  }

  /* Confirm presence of counters */
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

    printf(recvline);
  }
  printf("done.\n");

  //=============================================================================================
  //=== MEASUREMENT PHASE                                                                     ===
  //=== On TCP timeouts, we reconnect                                                         ===
  //=============================================================================================

  fp = fopen(OUTFILE, "w");
  if (fp == NULL) {
    printf("Trouble opening output file %s\n", OUTFILE);
    exit(1);
  }

  fprintf(fp, "pollCounters log in Unix Timestamp format\n");

  time(&timerStart);

  while (1) {

    printf("Measurement %d ", iter);

    // Collect the reading of each GPIB counter
    for (device = base_device; device <= max_device; device++) {

      int rc;
      timeIntervals[device] = 0.0f;

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

    // Log the readings
    fprintf(fp, "%zu           ", (size_t)timerNow);
    for (device = base_device; device <= max_device; device++) {
        fprintf(fp, "%.9le          ", timeIntervals[device]);
    }
    fprintf(fp, "\n");
    fflush(fp);

    // Show readings on terminal as well
    printf("Elapsed time: %4.0lf s\t  Time now: %zu s\t ", elapsedTime, (size_t)timerNow);
    for (device = base_device; device <= max_device; device++) {
      printf("%s:  %.9le s\t", _device_names[device], timeIntervals[device]);
    }
    printf("\n");
    fflush(stdout);

    sleep(samplePeriod_sec);
    ++iter;
  }

  /* Normal completion */
  printf("\n");
  close(sockfd);
  fclose(fp);
  exit(0);
}
