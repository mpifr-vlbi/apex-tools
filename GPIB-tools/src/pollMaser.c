/* ALR 01.05.12
 * based on ~/EB/1mm/APEX/DownconverterComms/pingVectorVoltmeter.c
 *
 * Client that talks to maser RS-232 port on Electronics Package
 * Uses TCP socket to talk to Lantronix media converter at port 10001
 *
 * Based on example code by Richard Stevens 1998, p295
 *  "UNIX Network Programming: Networking APIs: Sockets and XTI" Prentice Hall
 *  http://www.kohala.com/start/
 *
 * Compile with:  'gcc pollMaser.c'
 *
 */

#include "nettools.h"
#include "version.h"

#define MAXLINE 4096          /* max text line length */
#define OUTFILE "pollMaser.out"

// TCP connection settings
char* _ipaddr;
char* _port;
int _timeout_secs;

int main(int argc, char **argv) {
  int sockfd;
  char recvline[MAXLINE + 1];
  char buff[MAXLINE];
  int i, j, k, idx, nMeasurements, samplePeriod;
  FILE *fp;
  time_t timerStart, timerNow;
  struct tm *calendarTime;
  char timeStamp[20];
  double elapsedTime;
  double FM0 = 1663121288.0;      /* p28 T4S_iMaser56_UserManualT4S-MAN-0012.pdf */
  double Fmas = 1420405751.0;     /* p28 T4S_iMaser56_UserManualT4S-MAN-0012.pdf */

  struct monitorData {
    char *header;
    int nbits;
    double gain;
    char value[9];
    double physicalValue;
  } monitorPoint[] = {
    {"U batt.A[V]",     12, 2.441e-2, "", 0.0},
    {"I batt.A[A]",     12, 1.221e-3, "", 0.0},
    {"U batt.B[V]",     12, 2.441e-2, "", 0.0},
    {"I batt.B[A]",     12, 1.221e-3, "", 0.0},
    {"Set H[V]",        12, 3.662e-3, "", 0.0},
    {"Meas. H[V]",      12, 1.221e-3, "", 0.0},
    {"I pur.[A]",       12, 1.221e-3, "", 0.0},
    {"I diss.[A]",      12, 1.221e-3, "", 0.0},
    {"H light[V]",      12, 1.221e-3, "", 0.0},
    {"IT heater[V]",    12, 4.883e-3, "", 0.0},
    {"IB heater[V]",    12, 4.883e-3, "", 0.0},
    {"IS heater[V]",    12, 4.883e-3, "", 0.0},
    {"UTC heater[V]",   12, 4.883e-3, "", 0.0},
    {"ES heater[V]",    12, 4.883e-3, "", 0.0},
    {"EB heater[V]",    12, 4.883e-3, "", 0.0},
    {"I heater[V]",     12, 4.883e-3, "", 0.0},
    {"T heater[V]",     12, 4.883e-3, "", 0.0},
    {"Boxes temp[?C]",  12, 2.441e-2, "", 0.0},
    {"I boxes[A]",      12, 1.221e-3, "", 0.0},
    {"Amb.temp.[?c]",   12, 1.221e-2, "", 0.0},
    {"C field[V]",      12, 2.441e-3, "", 0.0},
    {"U varactor[V]",   12, 2.441e-3, "", 0.0},
    {"U HT ext.[kV]",   12, 1.221e-3, "", 0.0},
    {"I HT ext[uA]",    12, 1.221e-1, "", 0.0},
    {"U HT int.[kV]",   12, 1.221e-3, "", 0.0},
    {"I HT int.[uA]",   12, 1.221e-1, "", 0.0},
    {"H st.pres.[bar]", 12, 4.883e-3, "", 0.0},
    {"H st. heat[V]",   12, 6.104e-3, "", 0.0},
    {"Pirani heat.[V]", 12, 6.104e-3, "", 0.0},
    {"Unused",          12, 0, "", 0.0},
    {"U 405kHz[V]",     12, 3.662e-3, "", 0.0},
    {"U OCXO[V]",       12, 2.441e-3, "", 0.0},
    {"+24 VDC[V]",       8, 9.766e-2, "", 0.0},
    {"+15 VDC[V]",       8, 7.813e-2, "", 0.0},
    {"-15 VDC[V]",       8, -7.813e-2, "", 0.0},
    {"+5 VDC[V]",        8, 3.906e-2, "", 0.0},
    {"-5 VDC[V]",        8, 0, "", 0.0},
    {"+8 VDC[V]",        8, 3.906e-2, "", 0.0},
    {"+18 VDC[V]",       8, 7.813e-2, "", 0.0},
    {"Unused",           8, 0, "", 0.0},
    {"Lock",             4, 1, "", 0.0},
    {"DDS",             16, 9.09495e-6, "", 0.0}
  };
  int nMonitorPoints = 42;

  /* Print a helpful message if the number of command line arguments is wrong */
  printf("\n*** pollMaser version %s ***\n", SOFTWARE_VERSION);

  if (argc != 5)
    err("usage: pollMaser <IPaddress> <port> <number of measurements> <sample period / s>");


  /* Get parameters */
  _ipaddr = strdup(argv[1]);
  _port = strdup(argv[2]);
  _timeout_secs = 10;
  nMeasurements = atoi(argv[3]);
  samplePeriod = atoi(argv[4]);
  
  /* Create a TCP socket */
  sockfd = tcp_client(argv[1], argv[2], 30);
  if (sockfd < 0) {
    snprintf(buff, sizeof(buff), "socket error: %s", strerror(errno));
    err(buff);
  }


  //==================================================================
  //=== MEASUREMENT PHASE                                          ===
  //=== On TCP timeouts, we reconnect                              ===
  //==================================================================

  fp = fopen(OUTFILE, "a");
  if (fp == NULL) {
    printf("Trouble opening output file %s\n", OUTFILE);
    exit(1);
  }

  time(&timerStart);

  for (i = 0; i < nMonitorPoints; i++)
    fprintf(fp, "%16s", monitorPoint[i].header);
  fprintf(fp, "\n");

  for (i = 1; i <= nMeasurements; i++) {
    time(&timerNow);
    calendarTime = gmtime(&timerNow);
    snprintf(timeStamp, 20, "%02d/%02d/%02d %02d:%02d:%02d", calendarTime->tm_mday,
                                                        calendarTime->tm_mon + 1,
                                                        calendarTime->tm_year - 100,
                                                        calendarTime->tm_hour,
                                                        calendarTime->tm_min,
       	                                                calendarTime->tm_sec);

    int rc;

    /*    printf("\nMeasurement %d of %d\n", i, nMeasurements); */
    rc = sendMessage(sockfd, "M\r\n");

    /* Maser needs a bit of time to reply */
    sleep(1);
    rc = readMessage2(sockfd, recvline, MAXLINE);

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

       /* Jump back to data "M" */
       continue;
    }

    /* Handle the monitor data record */
    /*    sscanf(recvline, "%lf", &timeInterval); */

    /* Example monitor reply from maser:
47305948B8F15626881FF159BEFA5B24AC629006D471144E86672E0677638C7DA0B47034B5102352B7BC67101CB00815FFBAC37F00CBDC021
    */

    /* Split the reply string into substrings for each monitor point */
    if (rc == 115) {
      idx = 0;
      for (j = 0; j < nMonitorPoints - 1; j++) {
        for (k = 0; k < monitorPoint[j].nbits / 4; k++) {
  	  monitorPoint[j].value[k] = recvline[idx];
          idx++;
        }
        monitorPoint[j].value[k+1] = '\0';
      }
    } else
      printf("Error: wrong length monitor reply from maser\n");

    /* Read DDS eight bytes with RXX\r\n */
    monitorPoint[j].value[0] = '\0';
    for (k = 0; k < 4; k++) {
      snprintf(buff, MAXLINE, "R%02d\r\n", k);
      rc = sendMessage(sockfd, buff);
      rc = readMessage2(sockfd, recvline, MAXLINE);

      if (rc == 4) {
        recvline[2] = '\0';
        strcat(monitorPoint[j].value, recvline);
      }
      else
        printf("Error: wrong length monitor reply from maser\n");
    }

    /* Convert from hex to physical value for each monitor point */
    for (j = 0; j < nMonitorPoints; j++) {
      if (j != 41)
        monitorPoint[j].physicalValue = strtol(monitorPoint[j].value, NULL, 16) *
                                        monitorPoint[j].gain;
      if (j == 41) {
        monitorPoint[j].physicalValue = Fmas -
	  ((double)strtol(monitorPoint[j].value, NULL, 16) - FM0) *
                                        monitorPoint[j].gain;
      }
    }      

    /* Print vertical format to screen */
    printf("\n         UT Date   %s\n", timeStamp);
    for (j = 0; j < nMonitorPoints; j++) {
      if (j != 41)
        printf("%16s   %.3lf\n", monitorPoint[j].header, 
                                  monitorPoint[j].physicalValue);
      if (j == 41)
        printf("%16s   %.6lf\n", monitorPoint[j].header, 
                                  monitorPoint[j].physicalValue);
    }

    elapsedTime = difftime(timerNow, timerStart);
    /*    fprintf(fp, "%.0lf           %.9le\n", elapsedTime, timeInterval); */

    /* Log to file */
    fprintf(fp, "%-24s", timeStamp);
    for (j = 0; j < nMonitorPoints; j++) {
      if (j != 41)
        fprintf(fp, "%-8.3lf", monitorPoint[j].physicalValue);
      if (j == 41)
        fprintf(fp, "%-.6lf\n", monitorPoint[j].physicalValue);
    }

    fflush(fp);
    sleep(samplePeriod);
  }

  /* Normal completion */
  close(sockfd);
  fclose(fp);
  exit(0);
}
