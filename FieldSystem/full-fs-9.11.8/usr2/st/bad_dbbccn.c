/* dbbccn: based on mk5cn which in turn...  */
 
#include <stdio.h> 
#include <fcntl.h>
#include <string.h> /* For strrchr() */ 
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h> /* For socket() and connect() */
#include <sys/socket.h> /* For socket() and connect() */
#include <netinet/in.h> /* For socket() with PF_INET */ 
#include <netdb.h> /* For getservbyname() and gethostbyname() */ 
#include <unistd.h> /* For close() */ 
#include <stdlib.h>

extern int h_errno; /* From gethostbyname() for herror() */ 
extern void herror(const char * s); /* Needed on HP-UX */ 
    /* Why (!) isn't this in one of these includes on HP-UX? */ 

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/fs/include/fscom.h"

//#include "/usr2/st/include/stparams.h"
//#include "/usr2/st/include/stcom.h"
//#include "/usr2/st/include/stm_addr.h"   /* Station shared mem. pointer*/


extern struct fscom *shm_addr;

#define CONTROL_FILE "/usr2/st/control/dbbcad.ctl"
#define BUFSIZE 1024 /* size of the input and output buffers */
static unsigned char inbuf[BUFSIZE];   /* input message buffer */
static unsigned char outbuf[BUFSIZE];  /* output message buffer */

//char me[]="dbbccn" ; /* My name */ 
int iecho;
long fail;

static void nullfcn();
static jmp_buf sig_buf;

int sock; /* Socket */ 
FILE * fsock; /* Socket also as a stream */ 
//char host[sizeof(stm_addr->dbbchost)]; /* maximum width plus one */
char host[129];
int port;
int is_open=FALSE;
int time_out;
int is_init=FALSE;

int main(int argc, char * argv[])
{

  int i, len, result;
  long ip[5];
  printf("DEBUG: go to putpname\n"); 
  putpname("dbbccn");
  printf("DEBUG: to setup_ids\n"); 
  setup_ids();    /* attach to the shared memory */
//  printf("DEBUG: to setup_st\n"); 
//  setup_st();  /*putting this in station code*/
  printf("DEBUG: to rte_prior\n"); 
  rte_prior(FS_PRIOR);
  host[0]=0;
  printf("DEBUG dbbccn goes to wait\n");
  while (TRUE)    {
    skd_wait("dbbccn",ip,(unsigned) 0);
    iecho=shm_addr->KECHO;
    printf("DEBUG dbbccn what code %d \n",ip[0]);
    switch (ip[0]) {
    case 0:
      /* ** Initialize ** */ 
      printf("DEBUG dbbccn try to init\n");
      fail=TRUE;
      result = doinit();
      ip[4]=fail;
      break;
    case 1:
    case 4:
    case 5:
      if(!is_init) {
	cls_clr(ip[1]);
	ip[0]=ip[1]=0;
	result=-98;
      } else
	result = doproc(ip);
      break;
    case 2:
      if(!is_init) {
	cls_clr(ip[1]);
	ip[0]=ip[1]=0;
	result=-98;
      } else
	result = dorelink(ip);
      break;
    case 3:
      if(!is_init) {
	cls_clr(ip[1]);
	ip[0]=ip[1]=0;
	result=-98;
      } else
	result = doclose(ip);
      break;
    default:
      cls_clr(ip[1]);
      ip[0]=ip[1]=0;
      result = -99;
      break;
    }
    ip[2] = result;
    memcpy(ip+3,"m5",2);
  }
}


/* ********************************************************************* */

int doinit()
{
    FILE *fp;   /* general purpose file pointer */
    char check;
    int error;

    if ( (fp = fopen(CONTROL_FILE,"r")) == NULL) {
        return -1 ;
    }

    check=fgetc(fp);
    while(check == '*' && check != EOF) {
      check=fgetc(fp);
      while(check != '\n' && check != EOF)
	check=fgetc(fp);
      if(check != EOF) 
	check=fgetc(fp);
    }
    if (check == EOF)
      /* ended in comment */      
      return 0;
    else if(ungetc(check, fp)==EOF)
      return -2;
   
    if ( fscanf(fp,"%80s %d %d",host,&port, &time_out)!=3) /* read a line */
      return -3;

//    strcpy(stm_addr->dbbchost,host);

    if(time_out <200)
      time_out= 200;
    
    fclose (fp);
 
    is_init=TRUE;
    if (0 > (error = open_dbbc(host,port))) { /* open dbbc unit */
      fail=FALSE;
      return error;
    }
    return 0;
}

/* ********************************************************************* */
int open_dbbc(char *host, int port)
{   
  struct servent * set; /* From getservbyname() */ 
  struct sockaddr_in socaddin; /* For connect() info */ 
  struct hostent * hostinfo; /* From gethostbyname() */ 
  unsigned char * uc; /* To debug print IP address */ 
  int i;
  fd_set wfds;
  struct timeval tv;
  int retval, iret;
  int error,serror;
  long flags;

  /* * Create a socket * */ 
  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) { /* Errors? */ 

    logit(NULL,errno,"un");
    return -11; /* Error */ } 

  socaddin.sin_family = PF_INET; /* To agree with socket() */ 

  /* * Get service number for socket m5drive * */ 
  socaddin.sin_port = htons(port); /* Port m5drive's number */

  /* * Find IP address of host to connect to * */ 
  if(signal(SIGALRM,nullfcn) == SIG_ERR){
    fprintf( stderr,"dbbccn: setting up signals, gethostbyname()\n");
    exit(-1);
  }
  rte_alarm( time_out);
    
  if (setjmp(sig_buf)) {
    hostinfo=NULL;
    errno = EINTR;
    goto gethostbyname_return;
  }


  hostinfo = gethostbyname(host);

  gethostbyname_return:    

  rte_alarm((unsigned) 0);
  if(signal(SIGALRM,SIG_DFL) == SIG_ERR){
    fprintf( stderr,"dbbccn: setting default signals, gethostbyname()\n");
    exit(-1);
  }

  if( hostinfo == NULL && errno == EINTR ) {
    (void) close(sock); 
    logit(NULL,errno,"un");
    return -25 ; /* Error */ }
  else if (hostinfo == NULL) { /* Get IP, OK? */


    switch (h_errno) { /* Which error? */
    case HOST_NOT_FOUND :
      (void) close(sock); 
      return -17;
      break;
    case TRY_AGAIN :
      (void) close(sock); 
      return -18;
      break;
    case NO_RECOVERY :
      (void) close(sock); 
      return -19;
      break;
    case NO_ADDRESS : /* = NO_DATA */
      (void) close(sock); 
      return -20;
    } /* End of switch */ 
    (void) close(sock); 
    logit(NULL,errno,"un");
    return -13 ; /* Error */
  } /* End of if hostinfo NULL */
    
  if (hostinfo->h_addr_list[0] == NULL) { /* First IP address OK? */ 
    (void) close(sock); 
    return -14 ; /* Error */ } 
    
  
  socaddin.sin_addr.s_addr = *((unsigned long *) hostinfo->h_addr_list[0]); 
  /* Use first address */ 
  /* * Connect this socket to host * */ 

  /* set-up select() */
  flags=fcntl(sock,F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(sock,F_SETFL,flags);
  FD_ZERO(&wfds);
  FD_SET(sock, &wfds);
  /* Wait up to time_out centiseconds  */
  tv.tv_sec = time_out/100;
  tv.tv_usec = (time_out%100)*10000;

  iret=connect(sock, (const struct sockaddr *) &socaddin, 
	       sizeof(struct sockaddr_in));


  if(iret<0 && errno == EINPROGRESS) {
    retval = select(sock+1, NULL, &wfds, NULL, &tv);
    /* Don't rely on the value of tv now! */
    if(retval == -1) {
      close(sock);
      logit(NULL,errno,"un");
      return -24; /* Error */ } 
    else if (!retval) {
      close(sock);
      return -21;
    }
    
    serror=sizeof(error);
    if(getsockopt(sock, SOL_SOCKET, SO_ERROR,
		  (void *) &error,(socklen_t *) &serror) < 0) {
      close(sock);
      logit(NULL,errno,"un");
      return -22; /* Error */ } 
    if(error!=0) {
      close(sock);
      logit(NULL,errno,"un");
      return -23;
    }
  } else if (iret < 0) { /* Connect, errors? */ 
    logit(NULL,errno,"un");
    (void) close(sock); 
    return -15; /* Error */ } 

  
  /* * Open socket also to read as a stream * */ 
  if ((fsock = fdopen(sock, "r")) == NULL) { /* OK? */
    logit(NULL,errno,"un");
    (void) close(sock); /* Error */ 
    return -16; /* Error */ } 

  /* End of initialization */ 

  is_open=TRUE;
  return 0;
}
int doproc(ip)
long ip[5];
{

  int rtn1;    /* argument for cls_rcv - unused */
  int rtn2;    /* argument for cls_rcv - unused */
  int msgflg;  /* argument for cls_rcv - unused */
  int save;    /* argument for cls_rcv - unused */
  char secho[3*BUFSIZE];
  int itry;

  long in_class;
  long out_class=0;
  int in_recs;
  int out_recs=0;
  int i, j, nchars;

  fd_set rfds;
  struct timeval tv;
  int retval;
  int error;
  int flags;

  int time_out_local;
  static struct {
    char string[80];
    int to;
  } list[ ] = {
    "scan_set=",     500,
    "scan_set =",    500,
    "record on",      26,
    "record=on",      26,
    "record = on",      26,
    "record off",    400,
    "record=off",    400,
    "record = off",    400,
    "play on",       300,
    "play=on",       300,
    "play = on",     300,
    "data_check?",   189,
    "track_check?",  189,
    "scan_check?",   327,
    "bank_set inc",  500,
    "bank_set=inc",  500,
    "bank_set = inc",  500,
    "",                0
  };
  char *ptr,*ptrcolon;
  int ierr, mode;
  int centisec[6];             /* arguments of rte_tick rte_cmpt */

  secho[0]=0;
  mode=ip[0];
  in_class=ip[1];
  in_recs=ip[2];

  secho[0]=0;
    
  if(!is_open) {
    if (0 > (error = open_dbbc(host,port))) { /* open dbbc unit */
      ip[2]=error;
      goto error;
    }
    rte_sleep(100); /* seem to need a 100 centisecond sleep here, is this
                      a Mark 5 or Linux problem? */
  }

  msgflg = save = 0;
  for (i=0;i<in_recs;i++) {
    if ((nchars = cls_rcv(in_class,inbuf,BUFSIZE-1,&rtn1,&rtn2,msgflg,save)) <= 0) {
      ip[2] = -101;
      goto error;
    }
    inbuf[nchars]=0;  /* terminate in case not string */
    itry=0;
  try:
    if(iecho && strlen(secho)> 0) {
      logit(secho,0,NULL);
      secho[0]=0;
    }
  if(!is_open) {
    if (0 > (error = open_dbbc(host,port))) { /* open dbbc unit */
      ip[2]=error;
      goto error;
    }
    rte_sleep(100); /* seem to need a 100 centisecond sleep here, is this
                      a Mark 5 or Linux problem? */
  }
    if(iecho) {
      int in, out;
      strcpy(secho,"[");
      for(in=0,out=strlen(secho);in <nchars;in++) {
	if(inbuf[in]=='\n') {
	  secho[out++]='\\';
	  secho[out++]='n';
	} else
	  secho[out++]=inbuf[in];
      }
      secho[out]=0;
      strcat(secho,"]");
    }
    /* * Send command * */ 
    flags=0;
#ifdef MSG_NOSIGNAL
    flags|=MSG_NOSIGNAL;
#endif
#ifdef MSG_DONTWAIT
    flags|=MSG_DONTWAIT;
#endif
    if(mode==4) {
      rte_cmpt(centisec+2,centisec+4);
      rte_ticks (centisec);
    }
    if (send(sock, inbuf, nchars, flags) < nchars) { /* Send to socket, OK? */ 
      logit(NULL,errno,"un");
      ip[2] = -102; /* Error */
      goto error;
    }

      /* check for data to read */

    FD_ZERO(&rfds);
    FD_SET(fileno(fsock), &rfds);

    /* determine time-out */

    time_out_local=time_out;
    for (j=0;list[j].string[0]!=0;j++) {
      if(NULL!=strstr(inbuf,list[j].string)) {
	time_out_local+=list[j].to;
      }
    }
	 
    /* Wait up to time_out centiseconds. */
    tv.tv_sec = time_out_local/100;
    tv.tv_usec = (time_out_local%100)*10000;


    retval = select(fileno(fsock)+1, &rfds, NULL, NULL, &tv);
    /* Don't rely on the value of tv now! */
 
    if(retval == -1) {
      if(itry>0)
	logit(NULL,errno,"un");
      fclose(fsock);
      close(sock);
      is_open=FALSE;
      ip[2]=-105;
      if(++itry<2)
	goto try;
      goto error;
    } else if (!retval) {
      fclose(fsock);
      close(sock);
      is_open=FALSE;
      ip[2]=-104;
      if(++itry<2)
	goto try;
      goto error;
    }

    /* * Read reply * */ 
    if (fgets(outbuf, sizeof(outbuf), fsock) == NULL) { /* OK? */ 
      if(itry>0)
	logit(NULL,errno,"un");
      fclose(fsock);
      close(sock);
      is_open=FALSE;
      ip[2] = -103;
      if(++itry<2)
	goto try;
      goto error;
    }
    if(mode==4) {
      rte_ticks (centisec+1);
      rte_cmpt(centisec+3,centisec+5);
    }
    if(iecho) {
      int in, out;
      strcat(secho,"<");
      for(in=0,out=strlen(secho);outbuf[in]!=0;in++) {
	if(outbuf[in]=='\n') {
	  secho[out++]='\\';
	  secho[out++]='n';
	} else
	  secho[out++]=outbuf[in];
      }
      secho[out]=0;
      strcat(secho,">");
      logit(secho,0,NULL);
      secho[0]=0;
    }

    outbuf[511]=0; /* truncate to maximum class record size, cls_snd
                      can't do this because it doesn't know it is a string,
                      cls_rcv() calling should do it either since this 
                      would require many more changes */
    cls_snd(&out_class, outbuf, strlen(outbuf)+1 , 0, 0);
    out_recs++;
    if(mode==4) {
      cls_snd(&out_class, centisec, sizeof(centisec) , 0, 0);
      out_recs++;
    }

    /* check errors */

    if(mode==5)   /* no error report here */
      continue;

    ptrcolon=strchr(outbuf,':');
    ptr=strchr(outbuf,'=');
    if(ptr==NULL || (ptrcolon!=NULL && ptr >ptrcolon))
      ptr=strchr(outbuf,'?');
    if(ptr==NULL || (ptrcolon!=NULL && ptr >ptrcolon))
      ptr=strchr(outbuf,' ');
    if(ptr==NULL || (ptrcolon!=NULL && ptr >ptrcolon) ||
       1!=sscanf(ptr+1,"%d",&ierr)){
      ip[2]=-899;
      goto error;
    } else if(ierr != 0 && ierr != 1) {
      /* is there a trailing parameter that could contain an error message */
      ptr=strchr(outbuf,':');
      if(ptr!=NULL) {
	char *save, *ptr2;

	ptr2=strchr(ptr+1,';'); /* terminate the string at the ; */
	if(ptr2!=NULL)
	  *ptr2=0;

	save=NULL;              /* initialize, nothing yet */

	ptr=strtok(ptr+1,":");  /* find the last paramter */
	while (ptr!=NULL) {
	  save=ptr;
	  ptr=strtok(NULL,":");
	}
	if(save!=NULL) {         /* if there was soemthing there */
	  while(*save!=0 && *save==' ')
	    save++;
	  if(*save!=0)
	    logite(save,-900,"m5");
	}
      }
      ip[2]=-900-ierr;
      goto error;
    }
      

  } /* End of for loop  */ 

  ip[0]=out_class;
  ip[1]=out_recs;
  ip[2]=0;
  return 0;

error:
  if(iecho && strlen(secho)> 0) {
    logit(secho,0,NULL);
    secho[0]=0;
  }
error2:
  cls_clr(in_class);
  ip[0]=out_class;
  ip[1]=out_recs;
  return ip[2];
}
int dorelink(ip)
long ip[5];
{

  int error;

  ip[0]=ip[1]=0;

  if(is_open) {
    is_open=FALSE;
    fclose(fsock);
    close(sock);
  }

  if (0 > (error = open_dbbc(host,port))) { /* open dbbc unit */
    ip[2]=error;
    return ip[2];
  }

  ip[2]=0;
  return 0;

}
int doclose(ip)
long ip[5];
{

  ip[0]=ip[1]=0;

  if(is_open) {
    is_open=FALSE;
    fclose(fsock);
    close(sock);
  }

  ip[2]=0;
  return 0;

}
static void nullfcn(sig)
int sig;
{

  if(signal(sig,SIG_IGN) == SIG_ERR ) {
    perror("nullfcn: error ignoring signal");
    exit(-1);
  }
  
  longjmp (sig_buf, 1);

  fprintf(stderr,"nullfcn: can't get here\n");

  exit(-1);
}
