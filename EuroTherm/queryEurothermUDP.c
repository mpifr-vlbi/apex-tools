/* ALR 09.07.10
 * based on ~/EB/3mm/WVR/Source/Sockets/client2.c ALR 11.06.02
 *
 * Client that talks to Eurotherm 3504 controller in maser housing:
 *  uses UDP socket to talk to lantronix UDS1100 media converter port 10001
 *
 * Based on example code by Richard Stevens 1998, p295
 *  "UNIX Network Programming: Networking APIs: Sockets and XTI" Prentice Hall
 *  http://www.kohala.com/start/
 *
 * Compile with:  'gcc -lm -lc pingMaserHousingUDP.c'
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <math.h>


#define MAXLINE 4096          /* max text line length */
#define SA struct sockaddr
int sockfd;
socklen_t salen;
struct sockaddr *sa;

void readStatus();
void readValue(int cmdIdx);
void writeValue(int cmdIdx, char **argv);
void diagnosticLoopBack();

void err(const char *, ...);
unsigned short int calculate_crc(char *z_p, unsigned short int z_message_length);

int udp_client(const char *host, const char *serv, void **saptr, socklen_t *lenp);

struct commands {
  char *command;
  int address;
  unsigned char MSB;
  unsigned char LSB;
  float scaleFactor;
  char *parameter;
} commandList[] = {
  {"readTvalue",                   1, 0x00, 0x01, 10.0, "Loop.1.Main.PV"},
  {"readTdutycycle",               4, 0x00, 0x04, 10.0, "Loop.1.Main.ActiveOut"},
  {"readTsetpoint",                2, 0x00, 0x02, 10.0, "Loop.1.Main.TargetSP"},
  { "cmdTsetpoint",                2, 0x00, 0x02, 10.0, "Loop.1.Main.TargetSP"},
  {"readRHvalue",               1025, 0x04, 0x01,  1.0, "Loop.2.Main.PV"},
  {"readRHdutycycle",           1028, 0x04, 0x04, 10.0, "Loop.2.Main.ActiveOut"},
  {"readRHsetpoint",            1026, 0x04, 0x02,  1.0, "Loop.2.Main.TargetSP"},
  { "cmdRHsetpoint",            1026, 0x04, 0x02,  1.0, "Loop.2.Main.TargetSP"},
  {"readStatus",                   0, 0x00, 0x00,  0.0, ""},
  {"diagnosticLoopBack",           0, 0x00, 0x00,  0.0, ""},
  {"readTsetpointLowLimit",      112, 0x00, 0x70, 10.0, "Loop.1.SP.SPLowLimit"},
  { "cmdTsetpointLowLimit",      112, 0x00, 0x70, 10.0, "Loop.1.SP.SPLowLimit"},
  {"readTsetpointHighLimit",     111, 0x00, 0x6F, 10.0, "Loop.1.SP.SPHighLimit"},
  { "cmdTsetpointHighLimit",     111, 0x00, 0x6F, 10.0, "Loop.1.SP.SPHighLimit"},
  {"readRHsetpointLowLimit",    1136, 0x04, 0x70,  1.0, "Loop.2.SP.SPLowLimit"},
  { "cmdRHsetpointLowLimit",    1136, 0x04, 0x70,  1.0, "Loop.2.SP.SPLowLimit"},
  {"readRHsetpointHighLimit",   1135, 0x04, 0x6F,  1.0, "Loop.2.SP.SPHighLimit"},
  { "cmdRHsetpointHighLimit",   1135, 0x04, 0x6F,  1.0, "Loop.2.SP.SPHighLimit"},
  {"readTalarmLowThreshold",   10241, 0x28, 0x01, 10.0, "Alarm.1.Threshold"},
  { "cmdTalarmLowThreshold",   10241, 0x28, 0x01, 10.0, "Alarm.1.Threshold"},
  {"readTalarmHighThreshold",  10273, 0x28, 0x21, 10.0, "Alarm.3.Threshold"},
  { "cmdTalarmHighThreshold",  10273, 0x28, 0x21, 10.0, "Alarm.3.Threshold"},
  {"readRHalarmLowThreshold",  10257, 0x28, 0x11,  1.0, "Alarm.2.Threshold"},
  { "cmdRHalarmLowThreshold",  10257, 0x28, 0x11,  1.0, "Alarm.2.Threshold"},
  {"readRHalarmHighThreshold", 10289, 0x28, 0x31,  1.0, "Alarm.4.Threshold"},
  { "cmdRHalarmHighThreshold", 10289, 0x28, 0x31,  1.0, "Alarm.4.Threshold"},
  { "end", 0, 0, 0, 0.0, ""}
};
/* MODBUS addresses come from using iTools to examine the clone file. */


int main(int argc, char **argv)
{
  int errn, n;
  unsigned char recvline[MAXLINE + 1];
  unsigned char buff[MAXLINE];
  int messageLength, messageIndex;
  unsigned char MSB, LSB;
  float value;
  int nArgsCorrect;
  int i, cmdIdx;


/* Print a helpful message if the number of command line arguments is wrong */

  nArgsCorrect = 0;
  if (argc == 4)
    if (strstr(argv[3], "read") != NULL || 
        strstr(argv[3], "diagnostic") != NULL)
    nArgsCorrect = 1;

  if (argc == 5)
  if (strstr(argv[3], "cmd") != NULL)
    nArgsCorrect = 1;

  if (nArgsCorrect == 0 || strstr(argv[3], "help") != NULL) {
    printf("usage: pingMaserHousingUDP <IPaddress> <port> <command: \n");
    i = 0;
    while (strstr("end", commandList[i].command) == NULL) {
      printf(" %s,\n", commandList[i].command);
      i++;
    }
    printf("  [<cmdValue>]\n");
    exit(0);
  }


/* Check the command and remember the corresponding MODBUS address */

  i = 0;
  cmdIdx = -1;
  while (strstr("end", commandList[i].command) == NULL) {
    if (strstr(argv[3], commandList[i].command) != NULL)
      cmdIdx = i;
    i++;
  }
  if (cmdIdx == -1) {
    printf("Command %s not known.\n", argv[3]);
    exit(1);
  } 


/* Create a UDP socket */

  sockfd = udp_client(argv[1], argv[2], (void **) &sa, &salen);
  if (sockfd < 0) {
    snprintf(buff, sizeof(buff), "socket error: %s", strerror(errno));
    err(buff);
  }


/* Call the appropriate function according to the operation requested */

  if (strstr(argv[3], "readStatus") != NULL)
    readStatus();
  else if (strstr(argv[3], "read") != NULL)
    readValue(cmdIdx);
  else if (strstr(argv[3], "cmd") != NULL)
    writeValue(cmdIdx, argv);
  else if (strstr(argv[3], "diagnosticLoopBack") != NULL)
    diagnosticLoopBack();


/* Finished.  exit() closes the socket */

  exit(0);
}


void readStatus()
{
  int n;
  unsigned char recvline[MAXLINE + 1];
  unsigned char buff[MAXLINE];
  int messageLength;
  int valueInt;
  unsigned char MSBSTATUS[8] = {0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0};
  unsigned char LSBSTATUS[8] = {0x26, 0x27, 0x28, 0x29, 0x2C, 0x69, 0x6A, 0};
  int bitpos;


/* Read out Alarm.1.out = MODBUS address 0x0126 = 294 decimal (temp low)
 *          Alarm.2.out                  0x0127 = 295 decimal (humidity low)
 *          Alarm.3.out                  0x0128 = 296 decimal (temp high)
 *          Alarm.4.out                  0x0129 = 297 decimal (humidity high)
 *          Loop.1.OP.ManualMode         0x022C = 556 decimal (loop in manual)
 *          LgcIO.LA.PV                  0x0169 = 361 decimal (low water)
 *          LgcIO.LB.PV                  0x016A = 362 decimal (anlage stoerung)
 */


/* Prepare MODBUS message "Read N Words" p42 */

  valueInt = 0;
  for (bitpos = 0; bitpos < 7; bitpos++) {

    buff[0] = 1;             /* MODBUS device address */
    buff[1] = 3;             /* function code */
    buff[2] = MSBSTATUS[bitpos];   /* MSB address of Alarm status */
    buff[3] = LSBSTATUS[bitpos];   /* LSB address of Alarm status */
    buff[4] = 0;             /* MSB number of words to read */
    buff[5] = 1;             /* LSB number of words to read */
    calculate_crc(buff, 6);  /* Calculate 2 byte CRC and append to the message */
    messageLength = 8;


/* Write the string to the socket */

    sendto(sockfd, buff, messageLength, 0, sa, salen);


/* Read the server's reply */

    n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);


/* Set bits in the return value corresponding to the alarms
 *  bit 0 set = Alarm 1 (temp low)
 *  bit 1 set = Alarm 2 (humidity low)
 *  bit 2 set = Alarm 3 (temp high)
 *  bit 3 set = Alarm 4 (humidity high)
 *  bit 4 set = Alarm 5 (regulator loop in manual mode)
 *  bit 5 reset = Alarm 6 (low water)
 *  bit 6 reset = Alarm 7 (anlage stoerung)
 */


/* Invert bit 6 so that normal state = 0 alarm = 1
 * like with the other bits */

    if (bitpos == 6)
      recvline[4] = 1 - recvline[4];

    if (n > 0)
      valueInt += recvline[4] * (int)pow(2.0, bitpos);
  }


/* Interpret the status codes using plain text */

  printf("Status byte = %d means:\n", valueInt);
  if (valueInt & 0x01)
    printf(" Alarm 1 is active: Temperature too low.\n");
  if (valueInt & 0x02)
    printf(" Alarm 2 is active: Humidity too low.\n");
  if (valueInt & 0x04)
    printf(" Alarm 3 is active: Temperature too high.\n");
  if (valueInt & 0x08)
    printf(" Alarm 4 is active: Humidity too high.\n");
  if (valueInt & 0x10)
    printf(" Alarm 5 is active: Loop is broken (regulator is in manual mode)\n");
  if (valueInt & 0x20)
    printf(" Alarm 6 is active: Low water.\n");
  if (valueInt & 0x40)
    printf(" Alarm 7 is active: Anlage Stoerung.\n");
  if (valueInt == 0)
    printf(" No alarms.\n");
}



void diagnosticLoopBack()
{
  int n;
  unsigned char recvline[MAXLINE + 1];
  unsigned char buff[MAXLINE];
  int messageLength, messageIndex;

/* Prepare MODBUS message "Diagnostic Loopback" p50 */

  buff[0] = 1;             /* MODBUS device address */
  buff[1] = 8;             /* function code: 8 = diagnostic loopback */
  buff[2] = 0;
  buff[3] = 0;
  buff[4] = 12;            /* MSB data to be looped back */
  buff[5] = 34;            /* LSB data to be looped back */
  calculate_crc(buff, 6);  /* Calculate 2 byte CRC and append to the the message */
  messageLength = 8;


/* Write the string to the socket */

  sendto(sockfd, buff, messageLength, 0, sa, salen);


/* Read the server's reply */

  n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

  /* Recvfrom reads the incoming byte stream into the character array 'recvline'.
   * If read encounters an error then n < 0.  Handle this.   */

  if (n < 0) {                            /* error occurred */
    snprintf(buff, sizeof(buff), "read error: %s", strerror(errno));
    err(buff);
  }

  else if (n >= 0)                         /* normal read */
    recvline[n] = 0;                      /* terminate the string with null */


/* Display the server's reply */

  printf("Received n = %d bytes\n", n);
  for (messageIndex = 0; messageIndex < n; messageIndex++) 
    printf("recvline[%d] = %d\n", messageIndex, recvline[messageIndex]);

}



void readValue(int cmdIdx)
{
  int errn, n;
  unsigned char recvline[MAXLINE + 1];
  unsigned char buff[MAXLINE];
  int messageLength, messageIndex;
  float value;

/*  printf ("argv[3] = %s MSB = %d LSB = %d\n", argv[3], MSB, LSB); */


/* Prepare MODBUS message "Read N Words" p42 */

  buff[0] = 1;             /* MODBUS device address */
  buff[1] = 3;             /* function code */
  buff[2] = commandList[cmdIdx].MSB;
            /* MSB address of first word 0 = loop 1 (temp), 4 = loop 2 (hum)) */
  buff[3] = commandList[cmdIdx].LSB;
                           /* LSB address of first word 1 = PV 2 = SP 3 = OP */
  buff[4] = 0;             /* MSB number of words to read */
  buff[5] = 1;             /* LSB number of words to read */
  calculate_crc(buff, 6);  /* Calculate 2 byte CRC and append to the the message */
  messageLength = 8;



  printf("Sending n = %d bytes\n", messageLength);
  for (messageIndex = 0; messageIndex < messageLength; messageIndex++) {
    printf("buff[%d] = %d\n", messageIndex, buff[messageIndex]);
  }


/* Write the string to the socket */

  sendto(sockfd, buff, messageLength, 0, sa, salen);


/* Read the server's reply */

  n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

  /* Recvfrom reads the incoming byte stream into the character array 'recvline'.
   * If read encounters an error then n < 0.  Handle this.   */

  if (n < 0) {                            /* error occurred */
    snprintf(buff, sizeof(buff), "read error: %s", strerror(errno));
    err(buff);
  }

  else if (n >= 0)                         /* normal read */
    recvline[n] = 0;                      /* terminate the string with null */


/* Display the server's reply */

  printf("Received n = %d bytes\n", n);
  for (messageIndex = 0; messageIndex < n; messageIndex++) {
    printf("recvline[%d] = %d\n", messageIndex, recvline[messageIndex]);
  }


/* If normal read, check the CRC in the reply */

  if (n > 0) {
    for (messageIndex = 0; messageIndex < n; messageIndex++)
      buff[messageIndex] = recvline[messageIndex];

    printf("n = %d\n", n);


/* Fudge the first byte of CRC in recvline to make it correct */

    if (strstr(commandList[cmdIdx].command, "readTvalue") != NULL ||
        strstr(commandList[cmdIdx].command, "readTsetpoint") != NULL ||
        strstr(commandList[cmdIdx].command, "readTdutycycle") != NULL ||
        strstr(commandList[cmdIdx].command, "readRHdutycycle") != NULL ||
        strstr(commandList[cmdIdx].command, "readStatus") != NULL)
      recvline[n - 2] = 255 - recvline[n - 2];


/* Calculate 2 byte CRC and append to the the message */

    calculate_crc(buff, n - 2);


    for (messageIndex = 0; messageIndex < n; messageIndex++) {
      printf("buff[%d] = %d\n", messageIndex, buff[messageIndex]);
    }


    if ((buff[n - 2] == recvline[n - 2]) && (buff[n - 1] == recvline[n - 1]))
      printf("CRC check pass\n");
    else
      printf("CRC check fail\n");

  }


/* Convert return value into sensible units */

  value = (buff[3] * 256 + buff[4]) / commandList[cmdIdx].scaleFactor;

  printf("Return value = %f\n", value);

}


void writeValue(int cmdIdx, char **argv)
{
  int errn, n;
  unsigned char recvline[MAXLINE + 1];
  unsigned char buff[MAXLINE];
  int messageLength, messageIndex;
  unsigned char MSB, LSB, MSBvalue, LSBvalue;
  float value;


/* Check bounds */

  value = (float)atof(argv[4]);

  if (strstr(argv[3], "cmdTsetpoint") != NULL)
    if (value < 15 || value > 30) 
      err("Error: Tsetpoint should be in range 15 C to 30 C");

  if (strstr(argv[3], "cmdRHsetpoint") != NULL)
    if (value < 8 || value > 90) 
      err("Error: RHsetpoint should be in range 8% to 90%");


/* Convert value from float into two MODBUS 8-bit bytes */

  value *= commandList[cmdIdx].scaleFactor;

/*
  if (strstr(argv[3], "cmdTsetpoint") != NULL)
    value *= 10.0;

  if (strstr(argv[3], "cmdRHsetpoint") != NULL)
    value *= 1.0;
*/
  MSBvalue = ((int)value / 256);
  LSBvalue = ((int)value % 256);



/* Prepare MODBUS message "Write A Word" p3-20 */

  buff[0] = 1;             /* MODBUS device address */
  buff[1] = 6;             /* function code */
  buff[2] = commandList[cmdIdx].MSB;
               /* MSB address of first word 0 = loop 1 (temp), 4 = loop 2 (hum)) */
  buff[3] = commandList[cmdIdx].LSB;
                           /* LSB address of first word 1 = PV 2 = SP 3 = OP */
  buff[4] = MSBvalue;      /* MSB value of word */
  buff[5] = LSBvalue;      /* LSB value of word */
  calculate_crc(buff, 6);  /* Calculate 2 byte CRC and append to the the message */
  messageLength = 8;

  printf("Sending n = %d bytes\n", messageLength);
  for (messageIndex = 0; messageIndex < messageLength; messageIndex++) {
    printf("buff[%d] = %d\n", messageIndex, buff[messageIndex]);
  }

/* Write the string to the socket */

  sendto(sockfd, buff, messageLength, 0, sa, salen);


/* Read the server's reply */

  n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);


/* Recvfrom reads the incoming byte stream into the character array 'recvline'.
   * If read encounters an error then n < 0.  Handle this.   */

  if (n < 0) {                            /* error occurred */
    snprintf(buff, sizeof(buff), "read error: %s", strerror(errno));
    err(buff);
  }

  else if (n >= 0)                         /* normal read */
    recvline[n] = 0;                      /* terminate the string with null */


/* Display the server's reply */

  printf("Received n = %d bytes\n", n);
  for (messageIndex = 0; messageIndex < n; messageIndex++) {
    printf("recvline[%d] = %d\n", messageIndex, recvline[messageIndex]);
  }


/* Check for possible error message in the return message
 * See Series 2000 Communications Handbook sec 2.20 "Error Response"
 */

  if (recvline[1] == 6 + 128) {
    printf("Eurotherm regulator retunred error code: %d = ", recvline[2]);
    if (recvline[2] == 2)
      printf("Illegal Data Address\n");
    else if (recvline[2] == 3)
      printf("Illegal Data Value\n");
    else
      printf("Unknown Error\n");
  }
}



unsigned short int calculate_crc(char *z_p, unsigned short int z_message_length)
/* From Eurotherm "2000 series Communications Handbook" p37
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


void err(const char *fmt, ...)
{
  fprintf(stderr, "%s\n", fmt);
  exit(1);
}


int udp_client(const char *host, const char *serv, void **saptr, socklen_t *lenp)
{
  int sockfd, n;
  struct addrinfo hints, *res, *ressave;

  bzero(&hints, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
    err("udp_client error for %s, %s: %s",
              host, serv, gai_strerror(n));
  ressave = res;

  do {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd >= 0)
      break;          /* success */
  } while ( (res = res->ai_next) != NULL);

  if (res == NULL)        /* errno set from final socket() */
    err("udp_client error for %s, %s", host, serv);

  *saptr = malloc(res->ai_addrlen);
  memcpy(*saptr, res->ai_addr, res->ai_addrlen);
  *lenp = res->ai_addrlen;

  freeaddrinfo(ressave);

  return(sockfd);
}
