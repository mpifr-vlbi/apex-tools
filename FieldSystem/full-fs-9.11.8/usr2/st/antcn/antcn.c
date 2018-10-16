
#define DEBUG_MODE     0  // if 1, nothing is sent to APECS, everything is directed to LOCALHOST
#define QUERY_ONSOURCE 0  // if 1, APECS is queried for onsource status, otherwise always onsource
#define QUERY_ONLY     0  // if 1, APECS is _not_ told to change source
#define USE_APECS_TRACK 1 // if 1, APECS uses track() to follow source, else on() to follow and record Total Power

//Note:
//  The default APECS control uses track() to track the source without total power.
//  The alternative control uses go(source,time=99999) to track and write total power into MBFITS,
//  and must be cancel()'ed in the post-ob to stop tracking.

/////////////////////////////////////////////////////////////////////////////////////
// INTERFACE INFO
/////////////////////////////////////////////////////////////////////////////////////

/* antcn.c UDP for APEX*/
/* Input */
/* IP(1) = mode
       0 = initialize LU
       1 = pointing (from SOURCE command)
       4 = direct communications (from ANTENNA command)
       5 = on/off source status for pointing programs
100 - 32767 = for site specific use

   IP(2) = class number (mode 4 only)
   IP(3) = number of records in class (mode 4 only)
   IP(4) - not used
   IP(5) - not used
*/

/* Output */
/*  IP(1) = class with returned message
      (2) = number of records in class
      (3) = error number
            0 - ok
           -1 - illegal mode
           -2 - timeout
           -3 - wrong number of characters in response
           -4 - interface not set to remote
           -5 - error return from antenna
           -6 - error in pointing model initialization
            others as defined locally
      (4) = 2HAN for above errors, found in FSERR.CTL
          = 2HST for site defined errors, found in STERR.CTL
      (5) = not used
*/

/////////////////////////////////////////////////////////////////////////////////////
// INCLUDE and DEF
/////////////////////////////////////////////////////////////////////////////////////

/* Defined variables */
#define MINMODE 0  /* min,max modes for our operation */
#define MAXMODE 7

/* Include files */

#include <stdio.h>
#include <string.h>

#include "/usr2/fs/include/params.h" /* FS parameters            */
#include "/usr2/fs/include/fs_types.h" /* FS header files        */
#include "/usr2/fs/include/fscom.h"  /* FS shared mem. structure */
#include "/usr2/fs/include/shm_addr.h" /* FS shared mem. pointer */

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <grp.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "../apecs/apecs_iface.c" // TODO: use header file...

// APEX general control computer
//#define APEXTRANSHOST  "10.0.2.165" // IP for apexdev2.apex-telescope.org
#define APEXTRANSHOST    "10.0.2.170" // IP for observer3.apex-telescope.org
#define APEXTRANSPORT    22122        // port 22122 UDP remote control
#define APEXTRANSTIMEOUT 5000000      // 500e3 usec = 0.5s, 1e6 usec = 1s : default 3.0sec
#define ANTCNLOCALNAME   "10.0.2.95"  // IP for mark5c2.apex-telescope.org
#define ANTCNLOCALPORT   22122
#define DEBUGTRANSHOST   "127.0.0.1"

#if (DEBUG_MODE)
   #undef  APEXTRANSHOST
   #define APEXTRANSHOST DEBUGTRANSHOST
#endif

struct fscom *fs;

/* Subroutines called */
void setup_ids();
void putpname();
void skd_run(), cls_clr();
int nsem_test();
void logit();
void warn_if_debugmode();

/* locals */
#define ANTCN_STR_LEN 1024
static char* apecs_trackmode_prev;
static char* apecs_trackmode;
static char* apecs_gomode_prev;
static char* apecs_gomode;
static char* apecs_cmd;
static char* apecs_resp;

/////////////////////////////////////////////////////////////////////////////////////
// MAIN
/////////////////////////////////////////////////////////////////////////////////////

/* antcn main program starts here */
main()
{
  int ierr, nrec, nrecr;
  int dum = 0;
  int r1, r2;
  int imode,i,nchar;
  long ip[5], class, clasr;
  char buf[80], buf2[300];
  char buf3[300];
  char sorname[20];
  static char outra[15], outdec[15]; /*check len of these! */

  int n,iret,n1,n2,nepoch;
  apecs_fd_t* afd;

/* Set up IDs for shared memory, then assign the pointer to "fs", for readability.  */
  setup_ids();
  fs = shm_addr;

/* Allocations */
  apecs_trackmode_prev = malloc(ANTCN_STR_LEN);
  apecs_trackmode = malloc(ANTCN_STR_LEN);
  apecs_gomode_prev = malloc(ANTCN_STR_LEN);
  apecs_gomode = malloc(ANTCN_STR_LEN);
  apecs_cmd = malloc(ANTCN_STR_LEN);
  apecs_resp = malloc(ANTCN_STR_LEN);
  memset(apecs_trackmode_prev, 0, ANTCN_STR_LEN);
  memset(apecs_trackmode, 0, ANTCN_STR_LEN);
  memset(apecs_gomode_prev, 0, ANTCN_STR_LEN);
  memset(apecs_gomode, 0, ANTCN_STR_LEN);
  memset(apecs_cmd, 0, ANTCN_STR_LEN);
  memset(apecs_resp, 0, ANTCN_STR_LEN);

/* Put our program name where logit can find it. */
  putpname("antcn");

/* APECS hook-up */
  afd = apecs_connect(APEXTRANSHOST, APEXTRANSPORT, /*timeout usec=*/APEXTRANSTIMEOUT, ANTCNLOCALPORT);
  // n = apecs_send(afd, "execfile('commands/vlbi_commands.apecs')\n", apecs_resp,ANTCN_STR_LEN);

/* Return to this point to wait until we are called again */
Continue:
  skd_wait("antcn",ip,(unsigned)0);

  imode = ip[0];
  class = ip[1];
  nrec = ip[2];

  nrecr = 0;
  clasr = 0;

  if (imode < MINMODE || imode > MAXMODE) {
    ierr = -1;
    goto End;
  }

/* Safety during devel phase */
  warn_if_debugmode();

/* Handle each mode in a separate section */
  switch (imode) {

    case 0:             /* initialize */
      ierr = 0;
      strcpy(buf,"Initializing antenna interface");
      logit(buf,0,NULL);
      fs->ionsor = 0;
      sprintf(buf3,"  ");
      break;

    case 1:             /* source= command */
      ierr = 0;
      strcpy(buf,"Commanding to a new source");
      rad2str(fs->ra50,"h0.4",outra); /*in our case this is J2000, not 1950*/
      rad2str(fs->dec50,"d0.4",outdec);
      n1=0; 
      n2=strcut(fs->lsorna,sorname,' ',n1); /* look for space at end of source, max 10 char*/
      if (n2>9) { n2=9; }
      sorname[10]='\0'; // TODO: Dave, should this ne sorname[n2+1]='\0' perhaps...?
      nepoch = fs->ep1950;
      if (nepoch<1900) { nepoch=1950; }

#if (!QUERY_ONLY)

      /* send source coordinates to APECS */
      sprintf(apecs_cmd, "source('%s','%s','%s')\n", sorname, outra, outdec);
      n = apecs_send(afd, apecs_cmd, apecs_resp, ANTCN_STR_LEN-1);
      if (n>0) {
         logit(apecs_resp, 0, NULL);
      }

 // OLD 2017 BEHAVIOUR
 //     /* tell APECS to track the object */
 //        // normal tracking, no simult. Total Power recording
 //     n = apecs_send(afd, "track()\n", apecs_resp, ANTCN_STR_LEN-1);
 // CHANGED FOR 2018
 // still ToDo how to propagate the scan time!?
      sprintf(apecs_cmd, "vlbi_tp_onsource(src='%s',t=5)\n", sorname);
      n = apecs_send(afd, apecs_cmd, apecs_resp, ANTCN_STR_LEN-1);
      if (n>0) {
         logit(apecs_resp, 0, NULL);
      }

#endif

      fs->ionsor = 0;
      break;

    case 4:            /* direct antenna= command */
      if (class == 0)
        goto End;

      for (i=0; i<nrec; i++) {

        strcpy(buf2,"Received message for antenna: ");
        nchar = cls_rcv(class,buf,sizeof(buf),&r1,&r2,dum,dum);

        /* convert msg into LF&null-terminated string for APECS */
	//for(n1=0; n1<nchar; n1++) { buf[n1]=tolower(buf[n1]); }
	buf[nchar]='\n';
        buf[nchar+1] = '\0';

#if (!QUERY_ONLY)
        n = apecs_send(afd, buf, buf3,sizeof(buf3));
#endif

        strcat(buf2,buf3);
        logit(buf2,0,NULL);
        strcpy(buf,"ACK");
        cls_snd(&clasr,buf,3,dum,dum);
        nrecr += 1;
      }
      /* OR: cls_clr(class); */
      break;

    case 3:    /* on/off source status (from ONSOURCE command) */
    case 5:    /* on/off source status for pointing programs */
    case 7:    /* log tracking data (from TRACK command) */
      ierr = 0;
#if QUERY_ONSOURCE

      // send request and receive (hopefully) and "OK"
      sprintf(apecs_cmd, "vlbi_get_tracking_status('%s',%d)\n", ANTCNLOCALNAME, ANTCNLOCALPORT);
      n = apecs_send(afd, apecs_cmd, apecs_resp, ANTCN_STR_LEN);
      // if(strncasecmp(apecs_resp,"OK")||timeout) { /*try again N times?*/ }
      strncpy(apecs_trackmode, apecs_resp, ANTCN_STR_LEN);

      // wait for second reply with the tracking information (arrives hopefully...)
      //apecs_recv_req(afd, apecs_cmd, apecs_trackmode, ANTCN_STR_LEN);
      //logit(apecs_trackmode, 0, NULL);

      // copy if info changed from previous
      if (n>0 && strlen(apecs_trackmode)>0) {
          if (strncasecmp(apecs_trackmode, apecs_trackmode_prev, ANTCN_STR_LEN) != 0) {
              strncpy(apecs_trackmode_prev,apecs_trackmode,ANTCN_STR_LEN-1);
          }
      }

      // APECS system design problem, TRACKING is the same as SLEWING,
      // not sure what to do about this... 
      // query goal and actual az-el (very slow queries) and check if 
      // delta fits into error margin?
      if (strncasecmp(apecs_trackmode, "Track", ANTCN_STR_LEN) == 0) {
          fs->ionsor = 1;
      } else {
          fs->ionsor = 0;
      }
#else
      fs->ionsor = 1;
#endif
      break;

    case 6:            /* reserved */
      ierr = -1;
      goto End;

    default:
      sprintf(buf2,"unknown cls imode %d\n", imode);
      logit(buf2,0,NULL);
      goto End;

  }  /* end of switch */

End:
  ip[0] = clasr;
  ip[1] = nrecr;
  ip[2] = ierr;
  memcpy(ip+3,"AN",2);
  ip[4] = 0;
  goto Continue;

}
/* - - - - - - - - - - - - - - - - - - - -*/
int strcut(sin,sout,t,n)
char sin[],sout[],t; int n;
/* input is string sin, look for a string in this starting with
   (not ' '),terminated by t, copy to sout, null terminated and
    not including t . Start looking in sin at position n ,
    return finishing position*/
{
  int n1,n2,nn,i;
  n1=locaten(' ',sin,n); n2=locate(t,sin,n1);
  for (i=n1,nn=0; i<n2 ; i++,nn++) sout[nn]=sin[i]; sout[n2-n1]='\0';
  return (n2);
}
/* - - - - - - - - - - - - - - - - - - - -*/
int locate(t,s,n) /*find posn of char t in s starting at posn n*/
char s[],t; int n;
{
  int i,j,k;
  for (i=n;s[i] != '\0';i++) {
      if(s[i]==t)
         return(i);
      }
  return(-1);
}
/* - - - - - - - - - - - - - - - - - - - -*/
int locaten(t,s,n) /*find posn of not char t in s starting at posn n*/
char s[],t; int n;
{
  int i,j,k;
  for (i=n;s[i] != '\0';i++) {
      if(s[i]!=t)
         return(i);
      }
  return(-1);
}
/* - - - - - - - - - - - - - - - - - - - -*/

/* Always send info if we are in simulation mode */
void warn_if_debugmode() 
{
   if (strcmp(APEXTRANSHOST, DEBUGTRANSHOST) == 0) {
      logit("DEBUG! not controlling the antenna",0,NULL);
   }
   #if QUERY_ONLY
   logit("DEBUG! query-only commands get sent!",0,NULL);
   #endif
}
