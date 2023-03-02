/*******************************************************************************
* APEX project
*
* "@(#) $Id: downconverter.cpp 5606 2010-06-22 14:07:57Z aroy $"
*
* who       when       what
* --------  ---------  ----------------------------------------------
* aroy     2010-01-13  Initial version for maser housing
* aroy     2010-02-10  Adapted for VLBI downconverter
*/

/************************************************************************
*   NAME
*   
* 
*   SYNOPSIS  Talks to VLBI downconverter via ethernet to Xport in 
*              downconverter which converts to RS232 to embedded controller
*              (BasicStamp)
*
*             Can set or read frequency, attenuator setting, 
*              phase lock indicator
*
*             Downconverter uses own format messages -
*               See VLBI at APEX Project Book.
*
*             Downconverter sends a monitor message every 10 s
*
*             A thread in this program receives every monitor message
*               and stores the latest values in a data structure in memory.
*
*             When a query is received from APECS, the monitor values are
*               returned from memory without contacting the downconverter.
*
*             When a command to set a value is received from APECS,
*               a message is prepared and sent to the downconverter
*               and the reply is received to confirm the operation
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


static char *rcsId="@(#) $Id: downconverter.cpp 5606 2010-06-22 14:07:57Z aroy $"; 
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

#include "keyword_downconverter.h"
#include "struct.h"
#include "cid_downconverter.h"
#include <pthread.h>

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <math.h>
#include <unistd.h>           /* for socket close() */


#define MAXLINE 4096          /* max text line length */
#define SA struct sockaddr


/* Program control */

#define _DEBUG 0
#define COMMERRORTIME 60             /* timeout in s, to return ERROR COMM */


/* Downconverter properties */

#define IPADDRESS "134.104.31.3"     /* Xport in Bonn */
#define PORT "10001"                 /* Xport port */
#define IFV1BANDWIDTH 2000           /* Bandwith in MHz */
#define CHAIN1FREQAXISFLIPPED "YES"  /* Sideband chain 1 flips freq axis */
#define CHAIN2FREQAXISFLIPPED "NO"   /* Sideband chain 2 not flip freq axis */
#define IFV1INPUT "RX1"              /* Input IF not selectable, always RX1 */
#define CHAIN1DEFAULTFREQ 7000       /* Chain 1 default synth freq in MHz */
#define CHAIN2DEFAULTFREQ 4000       /* Chain 2 default synth freq in MHz */
#define CHAIN1DEFAULTATTEN  10       /* Chain 1 default atten in dB*/
#define CHAIN2DEFAULTATTEN   6       /* Chain 2 default atten in dB*/
#define DEFAULTLOCALREMOTE   1       /* Default to remote, keypad disabled */


/* Data structure to hold in memory the latest monitor point values
   sent by the downconverter every 10 s, along with the timestamp
   and status flags to remember whether downconverter has been initialized */

struct converterParameters {int Freq[3];
                            int Atten[3];
                            int Alarm[3];
                            int RemoteControl;
                            int FreqInitialized[3];
                            int AttenInitialized[3];
                            time_t Timestamp;
                            int nbytesReceived;
                           };

converterParameters IFV1parameters = 
  {{0,0,0},
   {0,0,0},
   {0,0,0},
    0,
   {0,0,0},
   {0,0,0},
   time((time_t*)NULL),
   -999};

/*  {{0,7000,4000},{0,10,6},{0,0,0},1,{0,0,0},{0,0,0},time((time_t*)NULL),-999};*/

pthread_mutex_t IFV1parametersMutex = PTHREAD_MUTEX_INITIALIZER;

converterParameters newIFV1parameters =
  {{0,CHAIN1DEFAULTFREQ,CHAIN2DEFAULTFREQ},
   {0,CHAIN1DEFAULTATTEN,CHAIN2DEFAULTATTEN},
   {0,0,0},
   DEFAULTLOCALREMOTE,
   {0,0,0},
   {0,0,0},
   time((time_t*)NULL),
   -999};

pthread_mutex_t newIFV1parametersMutex = PTHREAD_MUTEX_INITIALIZER;

void IFV1Status(int token, int chain);

void IFV1Comm(int token, int chain);

void IFV1Set(int token, double value, int chain);

void IFV1Configure(int token);

void IFV1SetInput(int token, char *value, int chain);

void IFV1Reset(int token);

void IFV1AutoLevel(int token, int chain);

int udp_client(const char *host, const char *serv, void **saptr, 
               socklen_t *lenp);

void replyToACS(char *messageString, int token);


void IFV1Status(int token, int chain)
{
  char dev_string[80];
  char messageString[80];
  time_t now, then;
  extern converterParameters IFV1parameters;

/* Input tokens:
 * STATUS, STATE
 *
 * Reply with present status.
 */


/* Prepare the timestamp from the most recent monitor message */

  now = time((time_t*)NULL);
  then = IFV1parameters.Timestamp;


  sprintf(dev_string,"APEX:IFV1:");

  switch (token) {

    case STATUS:
      if (chain == 0)
        sprintf(messageString, "%sSTATUS ", dev_string);
      if (chain > 0)
        sprintf(messageString, "%sCHAIN%d:STATUS ", dev_string, chain);
      break;

    case STATE:
      if (chain == 0)
        sprintf(messageString, "%sSTATE ", dev_string);
      if (chain > 0)
        sprintf(messageString, "%sCHAIN%d:STATE ", dev_string, chain);
      break;
  }


/* Prepare to access IFV1parameters.  Need to lock it against 
 * posible simultaneous access by VLBIDownconverterCommsThread
 */

  pthread_mutex_lock(&IFV1parametersMutex);

  if (chain == 0) {
    if ((IFV1parameters.FreqInitialized[1] != 1) ||
        (IFV1parameters.FreqInitialized[2] != 1) ||
        (IFV1parameters.AttenInitialized[1] != 1) ||
        (IFV1parameters.AttenInitialized[2] != 1)) 

      strcat(messageString, "INITIALIZE");
    else if ((IFV1parameters.Alarm[1] == 1) ||
             (IFV1parameters.Alarm[2] == 1) ||
             (now - then) > COMMERRORTIME) 
      strcat(messageString, "FAULTED");

    else if (IFV1parameters.RemoteControl == 1)
      strcat(messageString, "ENABLED");

    else if (IFV1parameters.RemoteControl == 0)
      strcat(messageString, "DISABLED");
  }
  if ((chain == 1) || (chain == 2)) {
    if ((IFV1parameters.FreqInitialized[chain] != 1) ||
        (IFV1parameters.AttenInitialized[chain] != 1))
      strcat(messageString, "INITIALIZE");

    else if ((IFV1parameters.Alarm[chain] == 1) ||
             (now - then) > COMMERRORTIME)
      strcat(messageString, "FAULTED");

    else if (IFV1parameters.RemoteControl == 1)
      strcat(messageString, "ENABLED");

    else if (IFV1parameters.RemoteControl == 0)
      strcat(messageString, "DISABLED");
  }


/* Finished accessing IFV1parameters so unlock it for others. */

  pthread_mutex_unlock(&IFV1parametersMutex);


/* Send return messge to ACS */

  replyToACS(messageString, token);
}


void IFV1Comm(int token, int chain)
{
  char dev_string[80];
  char messageString[80];
  extern converterParameters IFV1parameters;
  time_t now, then;


/* Input tokens: CMDCENTERFREQ, FREQ,
 *               CMDATTEN, ATTEN
 *               CMDSYNTHFREQ, SYNTHFREQ,
 *               CMDMODE, MODE,
 *               CMDINPUT, INPUT,
 *               CMDIFINPUT, IFINPUT, 
 *               CMDBANDWIDTH, BANDWIDTH,
 *               FREQAXISFLIPPED,
 *               LOCK
 *
 * Reply: present setting, or INITIALIZE if not set.
 */


/* Prepare the start of the return string */

/*  if ((token == CMDMODE) || (token == MODE)) */
  if (chain == 0) 
    sprintf(dev_string,"APEX:IFV1:");
  else
    sprintf(dev_string,"APEX:IFV1:CHAIN%d:", chain);


  if (token == CENTERFREQ) 
    sprintf(messageString, "%sCENTERFREQ", dev_string);

  if (token == CMDCENTERFREQ)
    sprintf(messageString, "%sCMDCENTERFREQ", dev_string);

  if (token == SYNTHFREQ) 
    sprintf(messageString, "%sSYNTHFREQ", dev_string);

  if (token == CMDSYNTHFREQ)
    sprintf(messageString, "%sCMDSYNTHFREQ", dev_string);

  if (token == ATTEN)
    sprintf(messageString, "%sATTEN", dev_string);

  if (token == CMDATTEN)
    sprintf(messageString, "%sCMDATTEN", dev_string);

  if (token == MODE)
    sprintf(messageString, "%sMODE", dev_string);

  if (token == CMDMODE)
    sprintf(messageString, "%sCMDMODE", dev_string);

  if (token == INPUT)
    sprintf(messageString, "%sINPUT", dev_string);

  if (token == CMDINPUT) 
    sprintf(messageString, "%sCMDINPUT", dev_string);
  
  if (token == IFINPUT)
    sprintf(messageString, "%sIFINPUT", dev_string);

  if (token == CMDIFINPUT)
    sprintf(messageString, "%sCMDIFINPUT", dev_string);

  if (token == BANDWIDTH)
    sprintf(messageString, "%sBANDWIDTH", dev_string);

  if (token == CMDBANDWIDTH)
    sprintf(messageString, "%sCMDBANDWIDTH", dev_string);

  if (token == FREQAXISFLIPPED)
    sprintf(messageString, "%sFREQAXISFLIPPED", dev_string);

  if (token == LOCK)
    sprintf(messageString, "%sLOCK", dev_string);


/* Prepare to access IFV1parameters.  Need to lock it against 
 * posible simultaneous access by VLBIDownconverterCommsThread
 */

  pthread_mutex_lock(&IFV1parametersMutex);


/* If monitor data are too old, return a comm error */

  then = IFV1parameters.Timestamp;
  now = time((time_t*)NULL);

  if (now - then > COMMERRORTIME)
    strcat(messageString, " ERROR COMM");
  else {

    if (token == CENTERFREQ) {
      if (IFV1parameters.FreqInitialized[chain] != 1)
        strcat(messageString, " INITIALIZE");
      else
        sprintf(messageString, "%s %5.3f", messageString, 
		(float)(IFV1parameters.Freq[chain] + 
		pow(-1.0, chain) * IFV1BANDWIDTH / 2.0) / 1000.0);
    }

    if (token == CMDCENTERFREQ) {
      sprintf(messageString, "%s %5.3f", messageString, 
                             (float)(newIFV1parameters.Freq[chain] + 
		pow(-1.0, chain) * IFV1BANDWIDTH / 2.0) / 1000.0);
    }


    if (token == SYNTHFREQ) {
      if (IFV1parameters.FreqInitialized[chain] != 1)
        strcat(messageString, " INITIALIZE");

      else
        sprintf(messageString, "%s %5.3f", messageString, 
                (float)IFV1parameters.Freq[chain] / 1000.0);
    }

    if (token == CMDSYNTHFREQ) {
      sprintf(messageString, "%s %5.3f", messageString, 
              (float)newIFV1parameters.Freq[chain] / 1000.0);
    }


    if (token == ATTEN) {
      if (IFV1parameters.AttenInitialized[chain] != 1)
        strcat(messageString, " INITIALIZE");
      else
        sprintf(messageString, "%s %d", messageString,
                                         IFV1parameters.Atten[chain]);
    }

    if (token == CMDATTEN) {
      sprintf(messageString, "%s %d", messageString,
                                      newIFV1parameters.Atten[chain]);
    }

    if (token == MODE) {
      if (IFV1parameters.RemoteControl == 1) 
        strcat(messageString, " REMOTE");
      if (IFV1parameters.RemoteControl == 0) 
        strcat(messageString, " LOCAL");
    }

    if (token == CMDMODE) {
      if (newIFV1parameters.RemoteControl == 1) 
        strcat(messageString, " REMOTE");
      if (newIFV1parameters.RemoteControl == 0) 
        strcat(messageString, " LOCAL");
    }

    if (token == INPUT || token == CMDINPUT)
      sprintf(messageString, "%s %s", messageString, IFV1INPUT);

    if (token == BANDWIDTH || token == CMDBANDWIDTH)
      sprintf(messageString, "%s %5.3f", messageString,
                                         (float)IFV1BANDWIDTH / 1000.0);

    if (token == FREQAXISFLIPPED) {
      if (chain == 1)
        sprintf(messageString, "%s %s", messageString, CHAIN1FREQAXISFLIPPED);
      else if (chain == 2)
        sprintf(messageString, "%s %s", messageString, CHAIN2FREQAXISFLIPPED);
    }

    if (token == LOCK) {
      if (IFV1parameters.Alarm[chain] == 0)
        sprintf(messageString, "%s %s", messageString, "ON");
      else if (IFV1parameters.Alarm[chain] == 1)
        sprintf(messageString, "%s %s", messageString, "OFF");
    }

    if (token == CMDIFINPUT || token == IFINPUT)
      sprintf(messageString, "%s %d", messageString, chain);

  }


/* Finished accessing IFV1parameters so unlock it for others. */

  pthread_mutex_unlock(&IFV1parametersMutex);


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */

}



void IFV1Set(int token, double value, int chain)
{
  char dev_string[80];
  char messageString[80];
  char messagePartString[80];
  extern converterParameters newIFV1parameters; /* Store new values before send */


/* Input tokens: CMDSYNTHFREQ,
 *               CMDCENTERFREQ,
 *               CMDATTEN,
 *               CMDIFINPUT,
 *               CMDBANDWIDTH,
 *               REMOTE, 
 *               LOCAL,
 *               RX1
 *
 * Action: sends value to the downconverter to set the parameter
 *         replies to APECS with new monitor values returned by downconverter.
 */


/* Prepare to access newIFV1parameters.  
 * Need to lock against posible simultaneous access by 
 * VLBIDownconverterCommsThread
 */

  pthread_mutex_lock(&newIFV1parametersMutex);


/* Prepare start of return message to ACS */

  if (token == REMOTE || token == LOCAL)
    sprintf(dev_string,"APEX:IFV1:");
  else
    sprintf(dev_string,"APEX:IFV1:CHAIN%d:", chain);


/* Depending on token, update the corresponding value in newIFV1parameters
   in preparation for sending the new values to the downconverter */

  switch (token) {

    case CMDSYNTHFREQ:
      value *= 1000.0;
      sprintf(messageString, "%sCMDSYNTHFREQ ", dev_string);
      if (((value < 6000 || value > 9000) && chain == 1) ||
          ((value < 3000 || value > 6000) && chain == 2)) {
        if (chain == 1)
          strcpy(messagePartString, "ERROR SYNTHFREQ-OUT-OF-RANGE-6.000-9.000-GHz");
        else if (chain == 2)
          strcpy(messagePartString, "ERROR SYNTHFREQ-OUT-OF-RANGE-3.000-6.000-GHz");
      }
      else {
        newIFV1parameters.Freq[chain] = (int)value;
        sprintf(messagePartString, "%5.3f", 
                (float)newIFV1parameters.Freq[chain] / 1000.0);
      }
      break;

    case CMDCENTERFREQ:
      value *= 1000.0;
      sprintf(messageString, "%sCMDCENTERFREQ ", dev_string);
      if (((value < 5000 || value > 8000) && chain == 1) ||
          ((value < 4000 || value > 7000) && chain == 2)) {
        if (chain == 1)
          strcpy(messagePartString, "ERROR CENTERFREQ-OUT-OF-RANGE-5.000-8.000-GHz");
        else if (chain == 2)
          strcpy(messagePartString, "ERROR CENTERFREQ-OUT-OF-RANGE-4.000-7.000-GHz");
      }
      else {
        newIFV1parameters.Freq[chain] = (int)value - 
                                        (int)pow(-1.0, chain) * IFV1BANDWIDTH / 2;
        sprintf(messagePartString, "%5.3f", 
                (float)(newIFV1parameters.Freq[chain] + 
		pow(-1.0, chain) * IFV1BANDWIDTH / 2.0) / 1000.0);
      }
      break;

    case CMDATTEN:
      sprintf(messageString, "%sCMDATTEN ", dev_string);

      if (value < 0 || value > 31)
        strcpy(messagePartString, "ERROR ATTEN-OUT-OF-RANGE-0-31 dB");
      else {
        newIFV1parameters.Atten[chain] = (int)value;
        sprintf(messagePartString, "%d", newIFV1parameters.Atten[chain]);
      }
      break;

    case REMOTE:
      sprintf(messageString, "%sCMDMODE ", dev_string);
      newIFV1parameters.RemoteControl = 1;
      sprintf(messagePartString, "REMOTE");
      break;

    case LOCAL:
      sprintf(messageString, "%sCMDMODE ", dev_string);
      newIFV1parameters.RemoteControl = 0;
      sprintf(messagePartString, "LOCAL");
      break;

    case CMDIFINPUT:
      sprintf(messageString, "%sCMDIFINPUT ", dev_string);

      if (value != chain)
        sprintf(messagePartString, "ERROR IFINPUT-MUST-BE-%d", chain);
      else
        sprintf(messagePartString, "%d", chain);
      break;

    case CMDBANDWIDTH:
      sprintf(messageString, "%sCMDBANDWIDTH ", dev_string);
      if (value != IFV1BANDWIDTH / 1000.0)
        sprintf(messagePartString, "ERROR BANDWIDTH-MUST-BE-%5.3f",
                                   IFV1BANDWIDTH / 1000.0);
      else
        sprintf(messagePartString, "%5.3f", IFV1BANDWIDTH / 1000.0);
      break;

    case RX1:
      sprintf(messageString, "%sCMDINPUT ", dev_string);
      sprintf(messagePartString, "%s", IFV1INPUT);
      break;

  }


/* Prepare return string for ACS */

  strcat(messageString, messagePartString);


/* Finished accessing IFV1parameters so unlock */

  pthread_mutex_unlock(&newIFV1parametersMutex);


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */

}


void IFV1Configure(int token)
{
  char messageString[80];
  int count, n, finished;
  extern converterParameters IFV1parameters; /* Store present state */
  extern converterParameters newIFV1parameters; /* Store new values before send */
  struct timespec sleepTime; 


/* Input tokens: CONFIGURE, RESET
 *
 * Action: sends values to the downconverter
 *         replies to APECS with confirmation
 */


/* Prepare start of return message to ACS */

  if (token == CONFIGURE)
    sprintf(messageString,"APEX:IFV1:CONFIGURE");
  else if (token == RESET)
    sprintf(messageString,"APEX:IFV1:RESET");


/* Initialize number of bytes received so we can recognize when
   message return value is set */

  pthread_mutex_lock(&IFV1parametersMutex);
  IFV1parameters.nbytesReceived = -999;
  pthread_mutex_unlock(&IFV1parametersMutex);


/* Signal validity of new values by raising 
   newIFV1parameters.freq[0] to 1, so that the read thread will
   send the values to the downconverter. */

  pthread_mutex_lock(&newIFV1parametersMutex);
  newIFV1parameters.Freq[0] = 1;
  pthread_mutex_unlock(&newIFV1parametersMutex);


/* Wait for message to be sent and return values to be valid */

  count = 0;
  finished = 0;


/* 0.1 s sleep time */

  sleepTime.tv_sec = 0;
  sleepTime.tv_nsec = 100000000; 

  while (!finished) {

/* Check for reply - get the number of bytes returned in reply */

    pthread_mutex_lock(&IFV1parametersMutex);
    n = IFV1parameters.nbytesReceived;
    pthread_mutex_unlock(&IFV1parametersMutex);

/* Check for reply 30 times at 0.1 s intervals before timing out */

    if (n == -999 && count++ < 30)
      nanosleep(&sleepTime, (timespec*)NULL);
    else
      finished = 1;
  }


/* Report any error */

  pthread_mutex_lock(&IFV1parametersMutex);

  if (IFV1parameters.nbytesReceived <= 0)
    strcat(messageString, " ERROR COMM");

  pthread_mutex_unlock(&IFV1parametersMutex);


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */

}


void IFV1Reset(int token)
{
  extern converterParameters newIFV1parameters; /* Store new values before send */


/* Input tokens: RESET
 *
 * Action: set default command values and 
 *         call IFV1Configure to send them to the downconverter
 */


/* Command default values */

  newIFV1parameters.Freq[1] = CHAIN1DEFAULTFREQ;
  newIFV1parameters.Freq[2] = CHAIN2DEFAULTFREQ;
  newIFV1parameters.Atten[1] = CHAIN1DEFAULTATTEN;
  newIFV1parameters.Atten[2] = CHAIN2DEFAULTATTEN;
  newIFV1parameters.RemoteControl = DEFAULTLOCALREMOTE;


/* Call to IFV1Configure to send the default values to the downconverter */

  IFV1Configure(token);


/* Return messge to ACS was sent by IFV1Configure, so finish
   without sending it here. */

}



void IFV1AutoLevel(int token, int chain)

/* Set attenuator levels in IFV1 according to total power detectors
 * in DBBC
 *
 * 28.05.10: placeholder until DBBC finished for testing.
 */

{
  char dev_string[80];
  char messageString[80];
  int success = 0;

/* Prepare start of return message to ACS */

  if (chain == 1 || chain == 2)
    sprintf(dev_string,"APEX:IFV1:CHAIN%d:AUTOLEVEL", chain);
  if (chain == 0)
    sprintf(dev_string,"APEX:IFV1:AUTOLEVEL");


/* Report status */

  if (success == 0)
    sprintf(messageString, "%s ERROR", dev_string);
  else 
    sprintf(messageString, "%s", dev_string);


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */
}


void IFV1GetLevel(int token, int chain)

/* Read power level from total power detectors in DBBC
 *
 * 10.06.10: placeholder until DBBC finished for testing.
 */

{
  char dev_string[80];
  char messageString[80];
  int success = 0;

/* Prepare start of return message to ACS */

  sprintf(dev_string,"APEX:IFV1:CHAIN%d:LEVEL", chain);


/* Report status */

  if (success == 0)
    sprintf(messageString, "%s ERROR", dev_string);
  else 
    sprintf(messageString, "%s", dev_string);


/* Send return messge to ACS */

  replyToACS(messageString, token);


/* Finished. */
}


void replyToACS(char *messageString, int token)
{
  char outstring[800];
  char timestring[80];
  extern converterParameters IFV1parameters;
  time_t now, then;
  struct tm *tdat;


/* Prepare the timestamp from the most recent monitor message */

  then = IFV1parameters.Timestamp;
  now = time((time_t*)NULL);

  if ((token == STATUS) || (token == STATE))
    tdat = localtime(&now);

  else if (now - then > COMMERRORTIME)
    tdat = localtime(&now);

  else
    tdat = localtime(&then);

  strftime(timestring,sizeof(timestring),"%FT%T.0000%z",tdat);


/* Add timestamp to the return message */

  sprintf(outstring, "%s %s", messageString, timestring);


/* Send return message string to ACS */

  print_dat(outstring);

}


int udp_client(const char *host, const char *serv, void **saptr, socklen_t *lenp)
{
  int sockfd, n;
  struct addrinfo hints, *res, *ressave;

  bzero(&hints, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;


/* Open socket, if error, return immediately with value -1 */

  if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
    return(-1);

  ressave = res;

  do {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd >= 0)
      break;          /* success */
  } while ( (res = res->ai_next) != NULL);

  if (res == NULL)        /* errno set from final socket() */
    return(-1);

  *saptr = malloc(res->ai_addrlen);
  memcpy(*saptr, res->ai_addr, res->ai_addrlen);
  *lenp = res->ai_addrlen;

  freeaddrinfo(ressave);

  return(sockfd);
}


/* Thread to receive data broadcasts from downconverter
   Invoked by cid.cpp */

void *VLBIDownconverterCommsThread(void *threadargc) 
{
  int sockfd, messageIndex, n;
  socklen_t salen;
  struct sockaddr *sa;
  struct timeval tv;
  char buff[MAXLINE];
  char recvline[MAXLINE];
  int messageLength;
  extern converterParameters IFV1parameters;
  int newValues;


/* Create a UDP socket */

  sockfd = udp_client(IPADDRESS, PORT, (void **) &sa, &salen);


/* If socked not correctly opened then exit thread */

  if (sockfd < 0) {

    printf("Failed to open socket to downconverter in VLBIDownconverterCommsThread.\nThread exiting.\n");


/* Exit from thread */

    pthread_exit(NULL);
  }


/* If we reach here, socket was opened correctly */

/* Set 3 s timeout for socket read operations */

  tv.tv_sec = 3;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));


/* Prepare message containing initial values for synth and atten */

  pthread_mutex_lock(&IFV1parametersMutex);
  sprintf(buff, "c %4d %02d %4d %02d %1d \r\n", 
	  newIFV1parameters.Freq[1],
	  newIFV1parameters.Atten[1],
	  newIFV1parameters.Freq[2],
	  newIFV1parameters.Atten[2],
	  newIFV1parameters.RemoteControl);
  pthread_mutex_unlock(&IFV1parametersMutex);

  messageLength = strlen(buff);


/* Inspect the message to be sent */

  if (_DEBUG) {
    printf("Send buffer = %s\n", buff);
    printf("strlen(Send buffer) = %d\n", messageLength);
  }


/* Write the string to the socket.
   This sets default values and lets the XPORT in the downconverter 
   know the IP and port number where it should send heartbeat messages */

  sendto(sockfd, buff, messageLength, 0, sa, salen);


/* Server's reply will be read in the following while loop */

/* Up to here we have initialized the connection and sent the default values.
   Now receive the reply and then heartbeat messages forever.
   Whenever new settings become available, send them to the downconverter. */

/* Do forever */


  while (1) {


/* Check whether new settings have been written into newIFV1parameters
   for sending to the downconverter */

    newValues = 0;
    pthread_mutex_lock(&newIFV1parametersMutex);
    newValues = newIFV1parameters.Freq[0];
    pthread_mutex_unlock(&newIFV1parametersMutex);

    if (newValues == 1) {


/* Prepare the message to be sent */

      pthread_mutex_lock(&newIFV1parametersMutex);
      newIFV1parameters.Freq[0] = 0;
      sprintf(buff, "c %4d %02d %4d %02d %1d \r\n", 
	      newIFV1parameters.Freq[1],
	      newIFV1parameters.Atten[1],
	      newIFV1parameters.Freq[2],
	      newIFV1parameters.Atten[2],
	      newIFV1parameters.RemoteControl);
      pthread_mutex_unlock(&newIFV1parametersMutex);

      messageLength = strlen(buff);


/* Inspect the message to be sent */

      if (_DEBUG) {
        printf("Send buffer = %s\n", buff);
        printf("strlen(Send buffer) = %d\n", messageLength);
      }


/* Write the string to the socket */

      sendto(sockfd, buff, messageLength, 0, sa, salen);


/* Server's reply will be read by the following code that 
   reads the regular downconverter messages */


    }
   


/* Read the regular downconverter messages */

/* recvfrom reads the incoming byte stream into the character array 'recvline'.
 * If read encounters an error then n < 0.
 */

    n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);


/* Display the server's reply */

    if (_DEBUG) {
      printf("Rx received n = %d bytes\n", n);
      for (messageIndex = 0; messageIndex < n; messageIndex++)
        printf("Rx thread recvline[%d] = %c %d\n", messageIndex, 
                                                   recvline[messageIndex],
                                                   recvline[messageIndex]);
    }


/* If reply is valid, parse and store the return values */

    if (n > 0) {
      pthread_mutex_lock(&IFV1parametersMutex);
      sscanf(recvline, "m %d %d %d %d %d %d %d", &IFV1parameters.Freq[1],
                                                 &IFV1parameters.Atten[1],
                                                 &IFV1parameters.Alarm[1],
                                                 &IFV1parameters.Freq[2],
                                                 &IFV1parameters.Atten[2],
                                                 &IFV1parameters.Alarm[2],
                                                 &IFV1parameters.RemoteControl);


/* Store timestamp with the present time */

      IFV1parameters.Timestamp = time((time_t*)NULL);


/* Mark frequency and attenuator values as initialized */

      IFV1parameters.FreqInitialized[1] = 1;
      IFV1parameters.FreqInitialized[2] = 1;
      IFV1parameters.AttenInitialized[1] = 1;
      IFV1parameters.AttenInitialized[2] = 1;
      pthread_mutex_unlock(&IFV1parametersMutex);
    }


/* If we have sent new values to the downconverter */

    if (newValues == 1) {


/* Report the number of bytes received.  This informs the other thread
   that the send/receive exchange is finished. */

      pthread_mutex_lock(&IFV1parametersMutex);
      IFV1parameters.nbytesReceived = n;
      pthread_mutex_unlock(&IFV1parametersMutex);
    }

  }


/* Execution should never reach here.
   In case it does, exit from thread */

  close(sockfd); 
  pthread_exit(NULL);
}

