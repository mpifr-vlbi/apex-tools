/*******************************************************************************
* APEX project
*
* "@(#) $Id: counters.cpp 5762 2011-01-26 10:40:21Z aroy $"
*
* who       when       what
* --------  ---------  ----------------------------------------------
* aroy     2010-01-13  Initial version for maser housing
* aroy     2010-02-10  Adapted for VLBI downconverter
* aroy     2010-11-03  Adapted for counters
*/

/************************************************************************
*   NAME
*   
* 
*   SYNOPSIS  Talks to VLBI counters via ethernet to Prologix 
*              media converter then GPIB to the HP 53131A counters
*
*             Can read time interval between 1 PPS ticks
*
*             Counters use SCPI-format messages.
*               See VLBI at APEX Project Book.
*
*             A thread in this program reads the time interval every 10 s
*               and stores the latest values in a data structure in memory.
*
*             When a query is received from APECS, the monitor values are
*               returned from memory without contacting the counters.
*
*             When a command to configure is received from APECS,
*               a series of messages are sent to the counters
*               to set default configuration.
*               Replies are received to confirm the operation
* 
*   DESCRIPTION
*
*   FILES
*
*   ENVIRONMENT
*
*   COMMANDS
*
*   RETURN VALUES
*
*   CAUTIONS 
*
*   EXAMPLES
*
*   SEE ALSO
*
*   BUGS   
* 
*------------------------------------------------------------------------
*/


static char *rcsId="@(#) $Id: counters.cpp 5762 2011-01-26 10:40:21Z aroy $"; 
static void *use_rcsId = ((void)&use_rcsId,(void *) &rcsId);


/* 
 * System Headers 
 */

/*
 * Local Headers 
 */

/* 
 * Signal catching functions  
 */

/* 
 *Local functions  
 */



#include <stdio.h>
#include <time.h>

#include "keyword_counters.h"
#include "struct.h"
#include "cid_counters.h"
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <math.h>
#include <unistd.h>           /* for socket close() */
#include <arpa/inet.h>        /* for inet_pton() */

#define MAXLINE 4096          /* max text line length */
#define SA struct sockaddr


/* Program control */

#define _DEBUG 0
#define READPERIOD 30         /* time in s between reading the counters */
#define COMMERRORTIME READPERIOD*3  /* timeout in s, to return ERROR COMM */
#define SOCKETREADTIMEOUT  3   /*  timeout in s when read from socket blocks */
#define SOCKETWRITETIMEOUT 3   /*  timeout in s when write to socket blocks */

/* Counters properties */

/*#define IPADDRESS "134.104.17.38"   /* Prologix in Bonn, DHCP usual address */
#define IPADDRESS "10.0.2.93"       /* Prologix in APEX */
#define PORT "1234"                 /* Prologix port */


/* Data structure to hold in memory the latest time interval measurements
   read from the counters every 10 s, along with the timestamp
   and status flags to remember whether counters have been initialized */

struct counterParameters {char *counterName[3];
                          int counterGPIBaddress[3];
                          double TimeInterval[3];
                          double ExtRefFreq[3];
                          int Initialized[3];
                          int Enabled[3];
                          int HardwareFault[3];
                          time_t Timestamp[3];
                          int Command;
                          int nbytesReceived;
                         };

counterParameters apexCounterParameters = 
  {{"", "GPSMINUSFMOUT", "GPSMINUSMASER"},
   {0,3,4},
   {0,0,0},
   {0,-1.0,-1.0},
   {0,0,0},
   {0,0,0},
   {0,0,0},
   {time((time_t*)NULL),time((time_t*)NULL),time((time_t*)NULL)},
   0,
   -999};

pthread_mutex_t apexCounterParametersMutex = PTHREAD_MUTEX_INITIALIZER;

void CountersStatus(int token, int ncounter);

void CountersComm(int token, int ncounter);

void CountersConfigure(int token, int ncounter);

void CountersEnable(int token, int ncounter);

void CountersCheckExtRef(int token, int ncounter);

int tcp_client(const char *servIP, const char *servPort);

int sendMessage(int *sockfd, const char *buff);

int readMessage(int *sockfd, char *recvline);

void replyToACS(char *messageString, int token);


void CountersStatus(int token, int ncounter)
{
  char dev_string[80];
  char messageString[400];
  extern counterParameters apexCounterParameters;

/* Input tokens:
 * STATUS, STATE
 *
 * Reply with present status.
 */


/* Prepare to access apexCounterParameters.  Need to lock it against 
 * posible simultaneous access by apexCountersCommsThread
 */

  pthread_mutex_lock(&apexCounterParametersMutex);


/* Prepare the start of the return string */

  snprintf(dev_string, sizeof(dev_string), "APEX:COUNTERS:");

  switch (token) {

    case STATUS:
      if (ncounter == 0)
        snprintf(messageString, sizeof(messageString), "%sSTATUS", dev_string);
      if (ncounter > 0)
        snprintf(messageString, sizeof(messageString), "%s%s:STATUS", dev_string,
                               apexCounterParameters.counterName[ncounter]);
      break;

    case STATE:
      if (ncounter == 0)
        snprintf(messageString, sizeof(messageString), "%sSTATE", dev_string);
      if (ncounter > 0)
        snprintf(messageString, sizeof(messageString), "%s%s:STATE", dev_string, 
                               apexCounterParameters.counterName[ncounter]);
      break;

    default:
        snprintf(messageString, sizeof(messageString), "%sERROR", dev_string);
  }


  if (ncounter == 0) {

    if ((apexCounterParameters.HardwareFault[1] == 1) ||
        (apexCounterParameters.HardwareFault[2] == 1))
      strcat(messageString, " FAULTED");

    else if ((apexCounterParameters.Initialized[1] != 1) ||
        (apexCounterParameters.Initialized[2] != 1))
      strcat(messageString, " INITIALIZE");

    else if ((apexCounterParameters.Enabled[1] == 1) &&
             (apexCounterParameters.Enabled[2] == 1))
      strcat(messageString, " ENABLED");

    else if ((apexCounterParameters.Enabled[1] == 0) ||
             (apexCounterParameters.Enabled[2] == 0))
      strcat(messageString, " DISABLED");

  } else if ((ncounter == 1) || (ncounter == 2)) {

    if (apexCounterParameters.HardwareFault[ncounter] == 1)
      strcat(messageString, " FAULTED");

    else if (apexCounterParameters.Initialized[ncounter] != 1)
      strcat(messageString, " INITIALIZE");

    else if (apexCounterParameters.Enabled[ncounter] == 1)
      strcat(messageString, " ENABLED");

    else if (apexCounterParameters.Enabled[ncounter] == 0)
      strcat(messageString, " DISABLED");

  }


/* Finished accessing apexCounterParameters so unlock it for others. */

  pthread_mutex_unlock(&apexCounterParametersMutex);


/* Send return messge to ACS */

  replyToACS(messageString, token);
}


void CountersComm(int token, int ncounter)
{
  char dev_string[80];
  char messageString[80];
  extern counterParameters apexCounterParameters;
  time_t now, then;


/* Input tokens: GPSMINUSFMOUT, GPSMINUSMASER
 *
 * Reply: present time interval reading, or INITIALIZE if not valid.
 */


/* Prepare to access apexCounterParameters.  Need to lock it against 
 * posible simultaneous access by apexCountersCommsThread
 */

  pthread_mutex_lock(&apexCounterParametersMutex);


/* Prepare the start of the return string */

  snprintf(dev_string, sizeof(dev_string), "APEX:COUNTERS:%s:%s", 
          apexCounterParameters.counterName[ncounter],
          apexCounterParameters.counterName[ncounter]);


/* Record the times for later checking for a comm error */

  now = time((time_t*)NULL);
  then = apexCounterParameters.Timestamp[ncounter];


/* If this counter has not been initialized, return -999 (for 'INITIALIZE') */

  if (apexCounterParameters.Initialized[ncounter] != 1)
      snprintf(messageString, sizeof(messageString), "%s -2000000999", dev_string);


/* If this counter has not been enabled, return -998 (for 'DISABLED') */

  else
    if (apexCounterParameters.Enabled[ncounter] != 1)
      snprintf(messageString, sizeof(messageString), "%s -2000000998", dev_string);


/* If this counter has not responded recently, return 'ERROR COMM' */

  else
    if (now - then > COMMERRORTIME)
        snprintf(messageString, sizeof(messageString), "%s ERROR COMM", dev_string);


/* Otherwise, all is well and we can return the time interval measurement,
   converting from seconds to nanoseconds */

   else
     snprintf(messageString, sizeof(messageString), "%s %.0lf", dev_string, 
             apexCounterParameters.TimeInterval[ncounter] * 1e9);


/* Finished accessing apexCounterParameters so unlock it for others. */

  pthread_mutex_unlock(&apexCounterParametersMutex);


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */

}


void CountersConfigure(int token, int ncounter)
{
  char dev_string[80];
  char messageString[80];
  int count, finished;
  extern counterParameters apexCounterParameters; /* Store present state */
  struct timespec sleepTime; 
  int counter, startCounter = 1, endCounter = 2;
  int errorComm = 0;
  int Initialized;

/* Input tokens: CONFIGURE, RESET
 *
 * Action: sends commands to set default configuration to the counters
 *         replies to APECS with confirmation
 * 
 * ncounter = 0: configure both counters
 * ncounter = 1: configure counter 1
 * ncounter = 2: configure counter 2
 * ncounter = 3: configure both counters but don't reply to APECS since
 *               this is first initialization on startup triggered by
 *               cid_counters.cpp and not by a command from APECS.
 */


/* Prepare to access apexCounterParameters.  Need to lock it against 
 * posible simultaneous access by apexCountersCommsThread
 */

  pthread_mutex_lock(&apexCounterParametersMutex);

/* Prepare start of return message to ACS */

  if (ncounter == 0)
    snprintf(dev_string, sizeof(dev_string), "APEX:COUNTERS:");
  else if (ncounter == 1 || ncounter == 2)
    snprintf(dev_string, sizeof(dev_string), "APEX:COUNTERS:%s:",
            apexCounterParameters.counterName[ncounter]);

  pthread_mutex_unlock(&apexCounterParametersMutex);

  switch (token) {

    case CONFIGURE:
      snprintf(messageString, sizeof(messageString), "%sCONFIGURE", dev_string);
      break;

    case RESET:
      snprintf(messageString, sizeof(messageString), "%sRESET", dev_string);
      break;
  }


/* Set counter numbers for configuring */

  switch (ncounter) {
    case 0: 
      startCounter = 1;
      endCounter = 2;
      break;
    case 1:
      startCounter = 1;
      endCounter = 1;
      break;
    case 2:
      startCounter = 2;
      endCounter = 2;
      break;
    case 3: 
      startCounter = 1;
      endCounter = 2;
      break;
  }


/* For each counter */

  for (counter = startCounter; counter <= endCounter; counter++) {


/* Reset Initialized flag so we can recognize when the initialization
   is complete */

    pthread_mutex_lock(&apexCounterParametersMutex);

    apexCounterParameters.Initialized[counter] = 0;


/* Trigger configuration by setting
   apexCounterParameters.Command non-zero, so that the read thread will
    send the values to the counters.
   apexCounterParameters.Command = 0  do nothing
   apexCounterParameters.Command = 1  configure GPSminusFMOUT counter
   apexCounterParameters.Command = 2  configure GPSminusMASER counter
 */


    apexCounterParameters.Command = counter;

    pthread_mutex_unlock(&apexCounterParametersMutex);


/* Wait for message to be sent and return values to be valid */

    count = 0;
    finished = 0;


/* 0.1 s sleep time */

    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 100000000; 

    while (!finished) {

/* Check for initialization to be complete */

      pthread_mutex_lock(&apexCounterParametersMutex);
      Initialized = apexCounterParameters.Initialized[counter];
      pthread_mutex_unlock(&apexCounterParametersMutex);


/* Check for initialization 200 times at 0.1 s intervals before timing out */

      if (Initialized == 0 && count++ < 200)
        nanosleep(&sleepTime, (timespec*)NULL);
      else
        finished = 1;
    }


/* Report any error */

    if (Initialized == 0)
      errorComm = 1;

    if (_DEBUG)
      printf("Initialized = %d\n", Initialized);

  }

  if (errorComm == 1)
    strcat(messageString, " ERROR COMM");


/* Send return messge to ACS 
   unless this is the first initialization (ncounter == 3) */

  if (ncounter != 3)
    replyToACS(messageString, token);


/* Finished. */

}


void CountersEnable(int token, int ncounter)
{
  char dev_string[80];
  char messageString[80];
  extern counterParameters apexCounterParameters; /* Store present state */
  int counter, startCounter = 1, endCounter = 2;

/* Input tokens: ON, OFF
 *
 * Action: sets a bit in apexCounterParameters.Enabled[]
 *         to say whether or not the counter should be polled.
 *         replies to APECS with confirmation
 */


/* Prepare to access apexCounterParameters.  Need to lock it against 
 * posible simultaneous access by apexCountersCommsThread
 */

  pthread_mutex_lock(&apexCounterParametersMutex);



/* Prepare start of return message to ACS */

  if (ncounter == 0)
    snprintf(dev_string, sizeof(dev_string), "APEX:COUNTERS:");
  else if (ncounter == 1 || ncounter == 2)
    snprintf(dev_string, sizeof(dev_string), "APEX:COUNTERS:%s:", 
            apexCounterParameters.counterName[ncounter]);

  pthread_mutex_unlock(&apexCounterParametersMutex);

  switch (token) {

    case ON:
      snprintf(messageString, sizeof(messageString), "%sON", dev_string);
      break;

    case OFF:
      snprintf(messageString, sizeof(messageString), "%sOFF", dev_string);
      break;
  }


/* Configure the counter/s */

  switch (ncounter) {
    case 0: 
      startCounter = 1;
      endCounter = 2;
      break;
    case 1:
      startCounter = 1;
      endCounter = 1;
      break;
    case 2:
      startCounter = 2;
      endCounter = 2;
      break;
  }


/* For each counter */

  pthread_mutex_lock(&apexCounterParametersMutex);

  for (counter = startCounter; counter <= endCounter; counter++) {

    switch (token) {

/* Enable polling by CORBA only if the counter has been initialized */

      case ON:
        if (apexCounterParameters.Initialized[counter] == 1)
          apexCounterParameters.Enabled[counter] = 1;
        else
          strcat(messageString, " INITIALIZE");
        break;

/* Switch off polling by CORBA if requested */

      case OFF:
        apexCounterParameters.Enabled[counter] = 0;
        break;
    }
  }

  pthread_mutex_unlock(&apexCounterParametersMutex);


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */

}


void CountersCheckExtRef(int token, int ncounter)
{
  char dev_string[80];
  char messageString[400];
  int count, finished;
  extern counterParameters apexCounterParameters; /* Store present state */
  struct timespec sleepTime; 
  int errorComm = 0;
  double ExtRefFreq;
  time_t now, then;
  int Initialized, Enabled;

/* Input tokens: REFERENCE
 *
 * Action: sends commands to query the external reference validity and freq
 *         replies to APECS with frequency of external reference
 */


/* Prepare to access apexCounterParameters.  Need to lock it against 
 * posible simultaneous access by apexCountersCommsThread
 */




/* Prepare start of return message to ACS */

  pthread_mutex_lock(&apexCounterParametersMutex);

  snprintf(dev_string, sizeof(dev_string), "APEX:COUNTERS:%s:REFERENCE", 
          apexCounterParameters.counterName[ncounter]);


/* Record the times for later checking for a comm error */

  now = time((time_t*)NULL);
  then = apexCounterParameters.Timestamp[ncounter];

  Initialized = apexCounterParameters.Initialized[ncounter];
  Enabled = apexCounterParameters.Enabled[ncounter];
  pthread_mutex_unlock(&apexCounterParametersMutex);


/* If this counter has not been initialized, return 'INITIALIZE' */

  if (Initialized != 1)
      snprintf(messageString, sizeof(messageString), "%s INITIALIZE", dev_string);


/* If this counter has not been enabled, return 'DISABLED' */

  else
    if (Enabled != 1)
      snprintf(messageString, sizeof(messageString), "%s DISABLED", dev_string);


/* If this counter has not responded recently, return 'ERROR COMM' */

  else
    if (now - then > COMMERRORTIME)
      snprintf(messageString, sizeof(messageString), "%s ERROR COMM", dev_string);


/* Otherwise, all is well and we can query the counter for its
   external reference status */

  else {


/* Trigger the query of external reference status by setting
   apexCounterParameters.Command non-zero, so that the read thread will
    query the counter.
   apexCounterParameters.Command = 0  do nothing
   apexCounterParameters.Command = 1  configure GPSminusFMOUT counter
   apexCounterParameters.Command = 2  configure GPSminusMASER counter
   apexCounterParameters.Command = 3  check external reference GPSminusFMOUT
   apexCounterParameters.Command = 4  check external reference GPSminusMASER
 */

    pthread_mutex_lock(&apexCounterParametersMutex);
    apexCounterParameters.ExtRefFreq[ncounter] = -1;
    apexCounterParameters.Command = ncounter + 2;
    pthread_mutex_unlock(&apexCounterParametersMutex);


/* Wait for message to be sent and return values to be valid */

    count = 0;
    finished = 0;


/* 0.1 s sleep time */

    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 100000000; 

    while (!finished) {


/* Check for query to be complete */

      pthread_mutex_lock(&apexCounterParametersMutex);
      ExtRefFreq = apexCounterParameters.ExtRefFreq[ncounter];
      pthread_mutex_unlock(&apexCounterParametersMutex);


/* Check for initialization 200 times at 0.1 s intervals before timing out */

      if (ExtRefFreq < 0 && count++ < 200)
        nanosleep(&sleepTime, (timespec*)NULL);
      else
        finished = 1;
    }


/* Report any error */

    if (ExtRefFreq < 0)
      errorComm = 1;

    if (_DEBUG)
      printf("ExtRefFreq = %lf\n", ExtRefFreq);

  }

  if (errorComm == 1)
    snprintf(messageString, sizeof(messageString), "%s ERROR COMM", dev_string);


/* If all is well, report the reference source and frequency */

  else {
    pthread_mutex_lock(&apexCounterParametersMutex);
    if (apexCounterParameters.ExtRefFreq[ncounter] > 1e30)
      snprintf(messageString, sizeof(messageString), "%s INTERNAL-10MHz", dev_string);
    if (apexCounterParameters.ExtRefFreq[ncounter] == 1e7)
      snprintf(messageString, sizeof(messageString), "%s EXTERNAL-10MHz", dev_string);
    if (apexCounterParameters.ExtRefFreq[ncounter] == 5e6)
      snprintf(messageString, sizeof(messageString), "%s EXTERNAL-5MHz", dev_string);
    if (apexCounterParameters.ExtRefFreq[ncounter] == 1e6)
      snprintf(messageString, sizeof(messageString), "%s EXTERNAL-1MHz", dev_string);
    pthread_mutex_unlock(&apexCounterParametersMutex);
  }


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */

}


void replyToACS(char *messageString, int token)
{
  char outstring[800];
  char timestring[80];
  extern counterParameters apexCounterParameters; /* Store present state */
  time_t now, then[3];
  struct tm *tdat;


/* Prepare the timestamp from the most recent monitor message */

  pthread_mutex_lock(&apexCounterParametersMutex);
  then[1] = apexCounterParameters.Timestamp[1];
  then[2] = apexCounterParameters.Timestamp[2];
  pthread_mutex_unlock(&apexCounterParametersMutex);

  now = time((time_t*)NULL);


/* Default timestamp for reply is present time */

  tdat = localtime(&now);


/* Timestamp for reply depends on requested operation */

  if ((token == STATUS) || (token == STATE))
    tdat = localtime(&now);

  else if (strstr(messageString, "ERROR COMM") != NULL)
    tdat = localtime(&now);

  else if (token == GPSMINUSFMOUT)
    tdat = localtime(&then[1]);

  else if (token == GPSMINUSMASER)
    tdat = localtime(&then[2]);

  else if (token == CONFIGURE)
    tdat = localtime(&now);

  strftime(timestring,sizeof(timestring),"%FT%T.0000%z",tdat);


/* Add timestamp to the return message */

  snprintf(outstring, sizeof(outstring), "%s %s", messageString, timestring);


/* Send return message string to ACS */

  print_dat(outstring);

}


int tcp_client(const char *servIP, const char *servPort)
{
  int sockfd, errn, port;
  struct sockaddr_in servaddr;
  struct timeval tv;

  port = atoi(servPort);


/* Create a TCP socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    if (_DEBUG)
      printf("socket error: %s", strerror(errno));
    return(-1);
  }


/* Set up the server's address structure */

  bzero(&servaddr, sizeof(servaddr));   /* initialize server address to zero */
  servaddr.sin_family = AF_INET;        /* internet address is IPv4 format */
  servaddr.sin_port   = htons(port);   /* set port number for our server */


/* Convert address from presentation format eg "134.104.28.45" given on the
 * command line to numeric format and insert it into the server address
 * structure */

  errn = inet_pton(AF_INET, servIP, &servaddr.sin_addr);

  if (errn <= 0) {
    if (_DEBUG)
      printf("inet_pton error for %s", servIP);
    return -1;
  }


/* Establish connection with server */

  errn = connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
  if (errn < 0) {
    if (_DEBUG)
      printf("connect error: %s", strerror(errno));
    return -1;
  }


/* Set timeout for socket read and write operations */

  tv.tv_sec = SOCKETREADTIMEOUT;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  tv.tv_sec = SOCKETWRITETIMEOUT;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

  return sockfd;
}


int sendMessage(int *sockfd, const char *buff)
{
  int messageLength, n;
  int sent = 0;

/* Write the string to the socket - reconnect of connection was reset or not all data sent at once */

/* If the TCP connection was reset by the Prologix (eg power cycled)
 * then read or write operations set errno to ECONNRESET (104)
 * If power down a counter or disconnect the network cable,
 * then read or write operations set errno to EAGAIN (11)
 */

  while (!sent) {

    if (_DEBUG)
      printf("Send:  %s", buff);

    messageLength = strlen(buff);
    n = send(*sockfd, buff, messageLength, MSG_NOSIGNAL);

    if (_DEBUG)
      printf("errno after write = %d %s\n", errno, strerror(errno));

    if (n == messageLength) {

        sent = 1;

    } else if ((n != messageLength) || (errno == ECONNRESET) || (errno == EAGAIN) || (errno == EPIPE)) {

      if (_DEBUG) {
        printf("Short write or write error: %d bytes, %s\n", n, strerror(errno));
        printf("Reopening TCP connection\n");
      }

/* Close and reopen the TCP connection */

      errno = 0;

      close(*sockfd);

      while ( (*sockfd = tcp_client(IPADDRESS, PORT)) < 0) {

/* If socket not correctly opened then wait for READPERIOD and retry */

        if (_DEBUG)
          printf("\nFailed to open socket to counters in apexCountersCommsThread.\nRetrying.\n");

        sleep(READPERIOD);
      }

    }

  }

  return n;
}


int readMessage(int *sockfd, char *recvline)
{
  int n;

/* Initiate a read */

  sendMessage(sockfd, "++read eoi\n");

/* Receive the reply */

  memset(recvline, 0x00, MAXLINE);
  n = recvfrom(*sockfd, recvline, MAXLINE, 0, NULL, NULL);

  if (_DEBUG)
    printf("errno after read = %d %s\n", errno, strerror(errno));

  if (n > 0) {
    recvline[n] = '\0';
    if (_DEBUG)
      printf("%s", recvline);
  } else {
    if (_DEBUG)
      printf("Comm error\n");
  }

/* Reconnect if TCP connection was reset - but do not attempt to re-read, since unknown state */

  if (errno == ECONNRESET || errno == EAGAIN) {

    if (_DEBUG) {
      printf("write error: %s\n", strerror(errno));
      printf("Reopening TCP connection\n");
    }

/* Close and reopen the TCP connection */

    close(*sockfd); 

    while ( (*sockfd = tcp_client(IPADDRESS, PORT)) < 0) {

/* If socket not correctly opened then wait for READPERIOD and retry */

      if (_DEBUG)
        printf("\nFailed to open socket to counters in apexCountersCommsThread.\nRetrying.\n");

      sleep(READPERIOD);
    }

  }


  return n;
}


/* Thread to send and receive commands and measurements with counters
   Invoked by cid_counters.cpp */

void *apexCountersCommsThread(void *threadargc) 
{
  int sockfd, n;
  char buff[MAXLINE];
  char recvline[MAXLINE];
  int messageLength;
  extern counterParameters apexCounterParameters;
  int Command;
  int counter;
  int index;
  time_t now, then[3];
  int initialized;
  int enabled;
  int prologixGPIBaddress = 0;
  int errorQueueIndex;
  int errorComm;
  int errorCode;
  char errorString[100];


/* Command sequence for configuring the Prologix GPIB-LAN converter
 * ++rst       Perform a power-on reset.  Takes 5 s, so omit for now.
 * ++savecfg 0 Disable automatic saving of configuration parameters to 
 *             EPROM to save wear 
 * ++auto 0    disable automatic read-after-write; causes error with 
                non-query commands 
 * ++mode 1    Set Prologix in CONTROLLER mode
 * ++read_tmo_ms 3000  Set timeout period to 3000 ms for read operations
 * ++ifc       Assert GPIB IFC signal for 150 us to make this Prologix 
               Controller-in-charge on the GPIB bus (not necessary?)
 * ++eot_enable 0  Disable appending a character to reply when EOI is detected
 * ++eos 3:    Disable stripping off <CR> or <LF> and append <CR><LF> to 
 *             network message before sending to GPIB instrument
 */

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


/* Command sequence for configuring a counter */

  char *CounterConfigCommands[] = {
    "++clr\n",
    "*RST\n",
    "*CLS\n",
    "*SRE 0\n",
    "*ESE 0\n",
    ":STAT:PRES\n",
    ":SENS:ROSC:SOURCE:AUTO ON\n",
    ":INP1:ATT 1\n",
    ":INP1:COUP DC\n",
    ":INP1:IMP 1E6 OHM\n",
    ":EVEN1:LEVEL 1.0\n",
    ":EVEN1:SLOP POS\n",
    ":INP2:ATT 1\n",
    ":INP2:COUP DC\n",
    ":INP2:IMP 50 OHM\n",
    ":EVEN2:LEVEL 1.0\n",
    ":EVEN2:SLOP POS\n",
    ":FUNC 'TINT'\n",
    ":TINT:ARM:STAR:SOUR IMM\n",
    ":TINT:ARM:STOP:SOUR IMM\n",
    ":INIT:CONT ON\n"};


/* Number of elements in the preceding array */

  int nCounterConfigCommands = 21;


/* Command for reading a time interval from a counter */

/*  char *CounterReadTimeIntervalCommand = "FETCH:TINT?\n"; */

/*
 * FETCH:TINT? was recommended by Dave Graham but shows the
 * problem that messages are queued, not the latest measurement
 * is returned.
 * READ? reports the latest measurement and discards the queue.
 */

  char *CounterReadTimeIntervalCommand = ":READ?\n";


/* Command for checking for errors from a counter */

  char *CounterReadStatusCommand = ":SYSTEM:ERR?\n";


/* Command for checking validity and presence of external reference */

  char *CounterReadExtRefStatusCommand = ":ROSC:EXT:FREQ?\n";


/* Create a TCP socket */

  while ( (sockfd = tcp_client(IPADDRESS, PORT)) < 0) {


/* If socket not correctly opened then wait for READPERIOD and retry */

    if (_DEBUG)
      printf("\nFailed to open socket to counters in apexCountersCommsThread.\nRetrying.\n");

    sleep(READPERIOD);
  }


/* If we reach here, socket was opened correctly */

  if (_DEBUG)
    printf("\nConnected.\n");


/* Whenever commands become available, send them to the counters. */

/* Do forever */

  while (1) {


/* Clear previous socket errors so we can detect new errors */

    errno = 0;


/* Check whether a new command has been written into 
 * apexCounterParameters.Command for configuring the counters */

    Command = 0;
    pthread_mutex_lock(&apexCounterParametersMutex);
    Command = apexCounterParameters.Command;
    apexCounterParameters.Command = 0;
    pthread_mutex_unlock(&apexCounterParametersMutex);


/* Commands:
   apexCounterParameters.Command = 0  do nothing
   apexCounterParameters.Command = 1  configure GPSminusFMOUT counter
   apexCounterParameters.Command = 2  configure GPSminusMASER counter
   apexCounterParameters.Command = 3  check external reference GPSminusFMOUT
   apexCounterParameters.Command = 4  check external reference GPSminusMASER
 */


/* Handle the configure command */

    if (Command == 1 || Command == 2) {

      counter = Command;


/* Send configuration commands to the Prologix media converter */

      for (index = 0; index < nPrologixConfigCommands; index++)
        n = sendMessage(&sockfd, PrologixConfigCommands[index]);


/* Set the Prologix media converter to talk to the right GPIB address */

      pthread_mutex_lock(&apexCounterParametersMutex);
      snprintf(buff, sizeof(buff), "++addr %d\n", 
                    apexCounterParameters.counterGPIBaddress[counter]);
      messageLength = strlen(buff);


/* Remember the GPIB address being sent */

      prologixGPIBaddress = apexCounterParameters.counterGPIBaddress[counter];

      pthread_mutex_unlock(&apexCounterParametersMutex);


/* Send the GPIB address */

      n = sendMessage(&sockfd, buff);


/* Send the configuration commands to the counter */

      for (index = 0; index < nCounterConfigCommands; index++)
        n = sendMessage(&sockfd, CounterConfigCommands[index]);


/* Wait a couple of seconds for initialization to be complete */

      sleep(2);


/* Read all entries from the error queue (queue length is maximum 30 entries)
 * Check for any hardware errors (have a +ve error code)
 */

      errorCode = -999;
      errorQueueIndex = 0;
      errorComm = 0;


/* For every entry in the counter error queue */

      while (errorCode != 0 && errorQueueIndex < 30 && errorComm == 0) {

/* Send command to read an entry from the counter error queue */

       n = sendMessage(&sockfd, CounterReadStatusCommand);

/* Read the reply */

        n = readMessage(&sockfd, recvline);

/* If the reply was sensible */

        if (n > 0) {

/* Parse the error code and error string from the reply */

          sscanf(recvline, "%d,%s", &errorCode, errorString);

/* If there is a hardware error (errorCode > 0), remember it */

          if (errorCode > 0) {
            pthread_mutex_lock(&apexCounterParametersMutex);
            apexCounterParameters.HardwareFault[counter] = 1;
            pthread_mutex_unlock(&apexCounterParametersMutex);
	  }


/* Otherwise, clear the HardwareFault flag */

          else {
            pthread_mutex_lock(&apexCounterParametersMutex);
            apexCounterParameters.HardwareFault[counter] = 0;
            pthread_mutex_unlock(&apexCounterParametersMutex);
	  }
	}


/* If we didn't get  a reply, count it as a comm error */

        else
          errorComm = 1;


/* Keep count of the number of queue entries we've read */

        errorQueueIndex++;
      }


/* If no hardware errors and no comm errors, 
   then mark this counter as initialized and enable the measurements */

      pthread_mutex_lock(&apexCounterParametersMutex);
      errorCode = apexCounterParameters.HardwareFault[counter];
      pthread_mutex_unlock(&apexCounterParametersMutex);

      if (errorCode != 1 && errorComm == 0) {
        pthread_mutex_lock(&apexCounterParametersMutex);
        apexCounterParameters.Initialized[counter] = 1;
        apexCounterParameters.Enabled[counter] = 1;
        pthread_mutex_unlock(&apexCounterParametersMutex);
      }


/* Otherwise, mark this counter as not initialized and disable the 
   measurements */

      else {
        pthread_mutex_lock(&apexCounterParametersMutex);
        apexCounterParameters.Initialized[counter] = 0;
        apexCounterParameters.Enabled[counter] = 0;
        pthread_mutex_unlock(&apexCounterParametersMutex);
      }


/* Finished configuring this counter */

    }


/* Handle the query external reference status command */

    if (Command == 3 || Command == 4) {

      counter = 0;
      if (Command == 3)
        counter = 1;
      if (Command == 4)
        counter = 2;


/* Set the Prologix media converter to talk to the right GPIB address */

      pthread_mutex_lock(&apexCounterParametersMutex);
      snprintf(buff, sizeof(buff), "++addr %d\n", 
                    apexCounterParameters.counterGPIBaddress[counter]);
      messageLength = strlen(buff);


/* Remember the GPIB address being sent */

      prologixGPIBaddress = apexCounterParameters.counterGPIBaddress[counter];

      pthread_mutex_unlock(&apexCounterParametersMutex);


/* Send the GPIB address */

      n = sendMessage(&sockfd, buff);


/* Send the query external reference status comand to the counter */

      n = sendMessage(&sockfd, CounterReadExtRefStatusCommand);


/* Read the reply */

      n = readMessage(&sockfd, recvline);


/* If valid, parse the reply */

      if (n > 0) {
        pthread_mutex_lock(&apexCounterParametersMutex);
        sscanf(recvline, "%le", 
               &apexCounterParameters.ExtRefFreq[counter]);
        pthread_mutex_unlock(&apexCounterParametersMutex);
      }
      else {
        pthread_mutex_lock(&apexCounterParametersMutex);
        apexCounterParameters.ExtRefFreq[counter] = 0;
        pthread_mutex_unlock(&apexCounterParametersMutex);
      }


/* Finished handling the query external reference status command */

    }



/* Check how long has elapsed since we last read the counters */

    pthread_mutex_lock(&apexCounterParametersMutex);
    then[1] = apexCounterParameters.Timestamp[1];
    then[2] = apexCounterParameters.Timestamp[2];
    pthread_mutex_unlock(&apexCounterParametersMutex);
    now = time((time_t*)NULL);


/* If one counter is switched off and the other on, then
   the READPERIOD will always be exceeded by one counter,
   but we don't want to re-read every time around this loop
   (ie once per second), so require that both counters exceed 
   the READPERIOD */

    if (((now - then[1] > READPERIOD) && (now - then[2] > READPERIOD)) ||
        Command == 1 || Command == 2) {


/* Read time intervals from the counters that have been initialized 
   and enabled */

      for (counter = 1; counter <= 2; counter++) {
      
        pthread_mutex_lock(&apexCounterParametersMutex);
        initialized = apexCounterParameters.Initialized[counter];
        enabled = apexCounterParameters.Enabled[counter];
        pthread_mutex_unlock(&apexCounterParametersMutex);


/* Check if this counter is initialized and enabled.
 * If Command == 0, this is the regular reading loop so read both counters.
 * If Command == 1 or 2, then read only the counter that has just been 
 *                       configured (ie require that Command == counter)
 */

        if ((initialized == 1 && enabled == 1 && Command == 0) ||
            (initialized == 1 && enabled == 1 && Command == counter)) {


/* Set the Prologix media converter to talk to the right GPIB address,
   if it's not already set right */
        
          pthread_mutex_lock(&apexCounterParametersMutex);

          if (prologixGPIBaddress != 
              apexCounterParameters.counterGPIBaddress[counter]) {
            snprintf(buff, sizeof(buff), "++addr %d\n", 
                    apexCounterParameters.counterGPIBaddress[counter]);
            messageLength = strlen(buff);


/* Send the GPIB address */

            n = sendMessage(&sockfd, buff);


/* Remember the GPIB address sent */

            prologixGPIBaddress = apexCounterParameters.counterGPIBaddress[counter];

	  }
          pthread_mutex_unlock(&apexCounterParametersMutex);



/* Send command to read back the time interval */

          n = sendMessage(&sockfd, CounterReadTimeIntervalCommand);


/* Read the reply from the counter with the time interval measurement */

          n = readMessage(&sockfd, recvline);

          
/* If reply is valid, parse and store the return values
   Measurement reply is 17 characters long if trailing zero is suppressed */

          if (n > 8 && 
              n <= 18 && 
              strstr(recvline, "E") != NULL &&
              strstr(recvline,".") != NULL) {
            pthread_mutex_lock(&apexCounterParametersMutex);
            sscanf(recvline, "%le", 
                   &apexCounterParameters.TimeInterval[counter]);


/* Store timestamp with the present time */

            apexCounterParameters.Timestamp[counter] = time((time_t*)NULL);
            pthread_mutex_unlock(&apexCounterParametersMutex);

          }

/* If the measurement was not valid, maybe the read timed out 
   or messages are out of sync */

	  else {


/* Try reading again in case reply messages are out of sync with 
   the queries */

            n = readMessage(&sockfd, recvline);


/* Send a Device Clear message from the Prologix to the counter
 * to abort the measurement before attempting another one */

            n = sendMessage(&sockfd, "++clr\n");

	  }


/* Read all entries from the error queue (queue length is maximum 30 entries)
 * Check for any hardware errors (have a +ve error code)
 */

          errorCode = -999;
          errorQueueIndex = 0;
          errorComm = 0;

          while (errorCode != 0 && errorQueueIndex < 30 && errorComm == 0) {


/* Send command to read an entry from the counter error queue */

            n = sendMessage(&sockfd, CounterReadStatusCommand);


/* Read the reply */

            n = readMessage(&sockfd, recvline);


/* If the reply was sensible */

            if (n > 0) {


/* Parse the error code and error string from the reply */

              sscanf(recvline, "%d,%s", &errorCode, errorString);


/* If there is a hardware error (errorCode > 0), remember it */

              if (errorCode > 0) {
                pthread_mutex_lock(&apexCounterParametersMutex);
                apexCounterParameters.HardwareFault[counter] = 1;
                pthread_mutex_unlock(&apexCounterParametersMutex);
    	      }

/* Otherwise, clear the HardwareFault flag */

              else {
                pthread_mutex_lock(&apexCounterParametersMutex);
                apexCounterParameters.HardwareFault[counter] = 0;
                pthread_mutex_unlock(&apexCounterParametersMutex);
	      }
  	    }


/* If we didn't get  a reply, count it as a comm error */

            else
              errorComm = 1;


/* Keep count of the number of queue entries we've read */

            errorQueueIndex++;
          }


/* Report the number of bytes received.  This informs the other thread
   that the send/receive exchange is finished. */

          pthread_mutex_lock(&apexCounterParametersMutex);
          apexCounterParameters.nbytesReceived = n;
          pthread_mutex_unlock(&apexCounterParametersMutex);


/* Finished reading the time interval from this counter */

	}


/* Finished reading the time interval from both counters */

      }
    }


/* Wait a second before repeating the cycle, to save CPU time */ 

    sleep(1);


/* End of while (1) loop */

  }


/* Execution should never reach here.
   In case it does, exit from thread */

  printf("Execution should never reach here.  Exiting apexCountersCommsThread.\n");
  close(sockfd); 
  pthread_exit(NULL);
}

