/*******************************************************************************
* APEX project
*
* "@(#) $Id: maserhousing.cpp 5646 2010-07-15 09:59:47Z aroy $"
*
* who       when       what
* --------  ---------  ----------------------------------------------
* aroy     2010-07-15  Substate and state are returning error even though 
                        no status bits are set.  
                       Attempted fix: convert status word from float to 
                        integer before comparing to zero in replyToACS().
* aroy     2010-06-29  Remove inversion of low water bit so 1 = alarm
* aroy     2010-06-25  Add alarm bits for low water and system fault to status
* aroy     2010-05-18  Close UDP socket after transaction.
* aroy     2010-01-13  Initial version
*/

/************************************************************************
*   NAME
*   
* 
*   SYNOPSIS  Talks to Eurotherm 3504 temperature and humidity regulator
*             in the maser environmental housing via RS232 and Lantronix
*             using MODBUS messages, to read out key monitor points:
*              temperature, temperature setpoint, temperature dutycycle,
*              humidity, humidity setpoint, humidity dutycycle,
*              status
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


static char *rcsId="@(#) $Id: maserhousing.cpp 5646 2010-07-15 09:59:47Z aroy $"; 
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

#include "keyword_maserhousing.h"
#include "struct.h"
#include "cid_maserhousing.h"

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

#define IPADDRESS "10.0.2.97" /* Lantronix in maser housing at APEX */
/*#define IPADDRESS "134.104.28.33"*/  /* control2 in Bonn for eurothermSimulator */
#define PORT "10001"          /* Lantronix port */

#define _DEBUG 0

void MHCommStatus(int token);

void MHCommTH(int token);

int MHComm(unsigned char buff[MAXLINE], unsigned char recvline[MAXLINE],
            int messageLength);

unsigned short int calculate_crc(unsigned char *z_p, 
                                 unsigned short int z_message_length);

int udp_client(const char *host, const char *serv, void **saptr, 
               socklen_t *lenp);

void replyToACS(float value, int n, int token);


void MHCommStatus(int token)
{
  unsigned char buff[MAXLINE];
  unsigned char recvline[MAXLINE];
  int messageLength, n;
  unsigned char MSB[8] = {0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0};
  unsigned char LSB[8] = {0x26, 0x27, 0x28, 0x29, 0x2C, 0x69, 0x6A, 0};
  int value, bitpos;

/* Read out Alarm.1.out = MODBUS address 0x0126 = 294 decimal (temp low)
 *          Alarm.2.out                  0x0127 = 295 decimal (humidity low)
 *          Alarm.3.out                  0x0128 = 296 decimal (temp high)
 *          Alarm.4.out                  0x0129 = 297 decimal (humidity high)
 *          Loop.1.OP.ManualMode         0x022C = 556 decimal (loop in manual)
 *          LgcIO.LA.PV                  0x0169 = 361 decimal (low water)
 *          LgcIO.LB.PV                  0x016A = 362 decimal (anlage stoerung)
 */


/* Prepare MODBUS message "Read N Words" p42 */

  value = 0;
  for (bitpos = 0; bitpos < 7; bitpos++) {
  
    buff[0] = 1;             /* MODBUS device address */
    buff[1] = 3;             /* function code */
    buff[2] = MSB[bitpos];   /* MSB address of Alarm status */
    buff[3] = LSB[bitpos];   /* LSB address of Alarm status */
    buff[4] = 0;             /* MSB number of words to read */
    buff[5] = 1;             /* LSB number of words to read */
    calculate_crc(buff, 6);  /* Calculate 2 byte CRC and append to the message */
    messageLength = 8;


/* Send the message in buff and receive the reply in recvline */

    n = MHComm(buff, recvline, messageLength);


/* Set bits in the return value corresponding to the alarms 
 *  bit 0 set = Alarm 1 (temp low)
 *  bit 1 set = Alarm 2 (humidity low)
 *  bit 2 set = Alarm 3 (temp high)
 *  bit 3 set = Alarm 4 (humidity high)
 *  bit 4 set = Alarm 5 (regulator loop in manual mode)
 *  bit 5 set = Alarm 6 (low water)
 *  bit 6 reset = Alarm 7 (anlage stoerung)
 */


/* Invert bit 6 so that normal state = 0 alarm = 1 
 * like with the other bits */

    if (bitpos == 6)
      recvline[4] = 1 - recvline[4];

    if (n > 0) 
      value += recvline[4] * (int)pow(2.0, bitpos);

  }



/* Interpret the status codes using plain text */

  if (_DEBUG) {
    printf("Status byte = %d means:\n", value);
    if (value & 0x01)
      printf(" Alarm 1 is active: Temperature too low.\n");
    if (value & 0x02)
      printf(" Alarm 2 is active: Humidity too low.\n");
    if (value & 0x04)
      printf(" Alarm 3 is active: Temperature too high.\n");
    if (value & 0x08)
      printf(" Alarm 4 is active: Humidity too high.\n");
    if (value & 0x10)
      printf(" Alarm 5 is active: Loop is broken (regulator is in manual mode)\n");
    if (value & 0x20)
      printf(" Alarm 6 is active: Low water.\n");
    if (value & 0x40)
      printf(" Alarm 7 is active: Anlage Stoerung.\n");
  }


/* Interpret the status codes using Eurotherm Series 2000 Communications 
 * Handbook HA026230 p3-22
 */

/*
  if (_DEBUG) {
    printf("Status byte = %d means:\n", recvline[2]);
    if (recvline[2] && 0x01)
      printf(" Alarm 1 is active\n");
    if (recvline[2] && 0x02)
      printf(" Alarm 2 is active\n");
    if (recvline[2] && 0x04)
      printf(" Alarm 3 is active\n");
    if (recvline[2] && 0x08)
      printf(" Alarm 4 is active\n");
    if (recvline[2] && 0x10)
      printf(" Sensor is broken\n");
    if (recvline[2] && 0x20)
      printf(" Loop is broken (regulator is in manual mode)\n");
    if (recvline[2] && 0x40)
      printf(" Heater Fail (Load fault detected)\n");
    if (recvline[2] && 0x80)
      printf(" Tune Active (Auto tune active)\n");
  }
*/

/* Send return messge to ACS */

/*  replyToACS((float)recvline[2], n, token); */

  replyToACS((float)value, n, token);


/* Finished.  exit() closes the socket */

}


void MHCommTH(int token)
{
  unsigned char MSB = 0, LSB = 1;
  unsigned char recvline[MAXLINE + 1];
  unsigned char buff[MAXLINE];
  int n, messageLength;
  float value;


/* MODBUS MSB address of word depends on what monitor point is selected 
 *  with token:
 *  MSB = 0 for first loop (temperature)
 *  MSB = 4 for second loop (humidity)
 */

  switch (token) {

    case TEMPERATURE:
      MSB = 0;
      break;

    case CMDTEMPERATURE:
      MSB = 0;
      break;

    case TEMPDUTYCYCLE:
      MSB = 0;
      break;

    case HUMIDITY:
      MSB = 4;
      break;

    case CMDHUMIDITY:
      MSB = 4;
      break;

    case HUMIDDUTYCYCLE:
      MSB = 4;
      break;
  }



/* MODBUS LSB address of word depends on what monitor point is selected 
   with token */

  switch (token) {

    case TEMPERATURE:
      LSB = 1;
      break;

    case CMDTEMPERATURE:
      LSB = 2;
      break;

    case TEMPDUTYCYCLE:
      LSB = 4;
      break;

    case HUMIDITY:
      LSB = 1;
      break;

    case CMDHUMIDITY:
      LSB = 2;
      break;

    case HUMIDDUTYCYCLE:
      LSB = 4;
      break;
  }


/* Prepare MODBUS message "Read N Words" p42 */

  buff[0] = 1;             /* MODBUS device address */
  buff[1] = 3;             /* function code */
  buff[2] = MSB;           /* MSB address of first word 0 = loop 1 (temp), 
                                                        4 = loop 2 (hum)) */
  buff[3] = LSB;           /* LSB address of first word 1 = PV 2 = SP 3 = OP */
  buff[4] = 0;             /* MSB number of words to read */
  buff[5] = 1;             /* LSB number of words to read */
  calculate_crc(buff, 6);  /* Calculate 2 byte CRC and append to the message */
  messageLength = 8;


/* Send the message in buff and receive the reply in recvline */

  n = MHComm(buff, recvline, messageLength);


/* Convert return value into sensible units */

  if (buff[3] < 128)                           /* handle positive values */
    value = (buff[3] * 256 + buff[4]);
  else                                         /* handle negative values */
    value = (buff[3] * 256 + buff[4] - 65536);

  if (token == TEMPERATURE ||
      token == CMDTEMPERATURE ||
      token == TEMPDUTYCYCLE ||
      token == HUMIDDUTYCYCLE)
    value /= 10.0;


  if (_DEBUG)
    printf("Return value = %f\n", value);


/* Send return messge to ACS */

  replyToACS(value, n, token);

}


int MHComm(unsigned char buff[MAXLINE], unsigned char recvline[MAXLINE], 
	   int messageLength)
{
  int sockfd, messageIndex, n;
  socklen_t salen;
  struct sockaddr *sa;
  struct timeval tv;
  

/* Create a UDP socket */

  sockfd = udp_client(IPADDRESS, PORT, (void **) &sa, &salen);
  if (sockfd < 0)
    return(-1);


/* Set 1 s timeout for socket read operations */

  tv.tv_sec = 1;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));


/* Inspect the message to be sent */

  if (_DEBUG) {
    printf("Sending n = %d bytes\n", messageLength);
    for (messageIndex = 0; messageIndex < messageLength; messageIndex++)
      printf("buff[%d] = %d\n", messageIndex, buff[messageIndex]);
  }


/* Write the string to the socket */

  sendto(sockfd, buff, messageLength, 0, sa, salen);


/* Read the server's reply */

/* recvfrom reads the incoming byte stream into the character array 'recvline'.
 * If read encounters an error then n < 0.
 */

  n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);


/* Display the server's reply */

  if (_DEBUG) {
    printf("Received n = %d bytes\n", n);
    for (messageIndex = 0; messageIndex < n; messageIndex++)
      printf("recvline[%d] = %d\n", messageIndex, recvline[messageIndex]);
  }


/* If normal read, check the CRC in the reply */

  if (n > 0) {
    for (messageIndex = 0; messageIndex < n; messageIndex++)
      buff[messageIndex] = recvline[messageIndex];


/* Calculate 2 byte CRC and append to the the message */

    calculate_crc(buff, n - 2);

    if (_DEBUG) {
      printf("n = %d\n", n);
      for (messageIndex = 0; messageIndex < n; messageIndex++)
        printf("buff[%d] = %d\n", messageIndex, buff[messageIndex]);


      printf("buff %d, recvline %d, buff==recvline %d\n", 
  	     buff[n-2], recvline[n-2], buff[n - 2] == recvline[n - 2]);
      printf("buff %d, recvline %d, buff==recvline %d\n", 
	     buff[n-1], recvline[n-1], buff[n - 1] == recvline[n - 1]);
      if ((buff[n - 2] == recvline[n - 2]) && (buff[n - 1] == recvline[n - 1]))
        printf("CRC check pass\n");
      else
        printf("CRC check fail\n");
    }

/* If we detect a CRC error, then set n = -2 to signal a comms error */

    if (!(buff[n - 2] == recvline[n - 2]) && (buff[n - 1] == recvline[n - 1]))
      n = -2;
  }

  close(sockfd);

  return(n);
}


void replyToACS(float value, int n, int token)
{
  char outstring[800];
  char monitorPoint[800];
  char timestring[80];
  time_t now;
  struct tm *tdat;


/* Get system time */

  now = time((time_t*)NULL);
  tdat = localtime(&now);
  strftime(timestring,sizeof(timestring),"%FT%T.0000%z",tdat);


/* Prepare the return message */

  switch (token) {

    case STATUS:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:STATUS");
      sprintf(outstring, "%s %d %s", monitorPoint, (int)value, timestring);
      break;

    case TEMPERATURE:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:TEMPERATURE");
      sprintf(outstring, "%s %.1f %s", monitorPoint, value, timestring);
      break;

    case CMDTEMPERATURE:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:CMDTEMPERATURE");
      sprintf(outstring, "%s %.1f %s", monitorPoint, value, timestring);
      break;

    case TEMPDUTYCYCLE:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:TEMPDUTYCYCLE");
      sprintf(outstring, "%s %.1f %s", monitorPoint, value, timestring);
      break;

    case HUMIDITY:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:HUMIDITY");
      sprintf(outstring, "%s %.1f %s", monitorPoint, value, timestring);
      break;

    case CMDHUMIDITY:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:CMDHUMIDITY");
      sprintf(outstring, "%s %.1f %s", monitorPoint, value, timestring);
      break;

    case HUMIDDUTYCYCLE:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:HUMIDDUTYCYCLE");
      sprintf(outstring, "%s %.1f %s", monitorPoint, value, timestring);
      break;

    case STATE:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:STATE");
      if ((int)value == 0)
        sprintf(outstring, "%s ENABLED %s", monitorPoint, timestring);
      else
        sprintf(outstring, "%s FAULTED %s", monitorPoint, timestring);
      break;

    case SUBSTATE:
      sprintf(monitorPoint, "APEX:MASER:HOUSING:SUBSTATE");
      if ((int)value == 0)
        sprintf(outstring, "%s CLOSED_LOOP %s", monitorPoint, timestring);
      else
        sprintf(outstring, "%s ALARM %s", monitorPoint, timestring);
      break;
  }

  if (n < 0)
    sprintf(outstring, "%s ERROR COMM %s", monitorPoint, timestring);


/* Send return message string to ACS */

  print_dat(outstring);

}


unsigned short int calculate_crc(unsigned char *z_p, unsigned short int z_message_length)
/* From Eurotherm "2000 series Communications Handbook" p37 */
/* CRC runs cyclic Redundancy Check Algorithm on input z_p */
/* Returns value of 16 bit CRC after completion and */
/* always adds 2 crc bytes to message */
/* returns 0 if incoming message has correct CRC */

{
  unsigned short int CRC= 0xffff;
  unsigned short int next;
  unsigned short int carry;
  unsigned short int n;
  unsigned char crch, crcl;
  unsigned short int messageByteIndex, messageByteCounter;

  messageByteCounter = z_message_length;
  messageByteIndex = 0;
  while (messageByteCounter--) {
    /*  while (z_message_length--) { */
    next = (unsigned short int)z_p[messageByteIndex];
    CRC ^= next;
    for (n = 0; n < 8; n++) {
      carry = CRC & 1;
      CRC >>= 1;
      if (carry) {
        CRC ^= 0xA001;
      }
    }
    messageByteIndex++;
  }
  crch = CRC / 256;
  crcl = CRC % 256;
  z_p[z_message_length++] = crcl;
  z_p[z_message_length] = crch;
  return CRC;
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


