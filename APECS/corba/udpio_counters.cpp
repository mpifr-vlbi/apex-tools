/**********************************************************
  $Header$
***********************************************************/
//initial version for APECS-2.0, removed cout 

/**********************************************************
 *
 *  get_char(), unget_char(), readn() and writen() routines
 *  to support socket communication via UDP ports.
 *
 **********************************************************/

#include <stdio.h>
#include <unistd.h>
#include "cid_counters.h"

#include "ace/Log_Msg.h"
#include "ace/SOCK_Dgram.h"
#include "ace/INET_Addr.h"

#define QUERY_BUF_LEN 24576
#define PARSE_BUF_LEN 256

/* 'query_buf' and 'query_len' need to be global variables, as they are  */
/* used by other routines as well.                                       */
char query_buf[QUERY_BUF_LEN];    
int query_len = 0;       /* numbers of characters stored in query_buf    */

/* 'parse_buf', 'parse_ptr' and 'parse_len' can be made static, as they  */
/* are only referenced by routines in this source file.                  */
static char parse_buf[PARSE_BUF_LEN];    
static int parse_ptr = 0;  /* numbers of characters in parse_buf         */
static int parse_len = 0;  /* pointer used by get_char, unget_char         */

extern ACE_SOCK_Dgram dgram;
extern ACE_INET_Addr remote;

int get_char()
{
    int c;
	
    if (parse_ptr == parse_len) {
	if (readn(parse_buf, PARSE_BUF_LEN) == -1) {
	    ACE_ERROR_RETURN((LM_ERROR,
			      "%p\n",
			      "get_char"),
			     -1); 
	    return -1;
	}
    }
    c = parse_buf[parse_ptr];
    parse_ptr++;
    return c;
}

int unget_char(char c)
{
    if (parse_ptr <= 0) {
	ACE_ERROR_RETURN((LM_ERROR,
			  "%p\n",
			  "unget_char"),
			 -1); 
	return -1;
    }
    parse_ptr--;
    parse_buf[parse_ptr] = c;
    return 0;
}	

int readn(char *ptr, int nbytes)
{

    bzero(ptr,nbytes);
    if (dgram.recv(ptr,nbytes, remote) == -1) {
	ACE_ERROR_RETURN((LM_ERROR,
			  "%p\n",
			  "receive"),
			 -1); 
	return -1;
    }

    parse_len = ACE_OS::strlen(ptr);
    parse_ptr = 0;
    return 0;
}

int writen(char *ptr, int nbytes)
{
    int nleft;
    nleft = nbytes;

    nbytes=ACE_OS::strlen(ptr);
    if (dgram.send (ptr,ACE_OS::strlen (ptr) + 1, remote) == -1) {
	ACE_ERROR_RETURN((LM_ERROR,
			  "%p\n",
			  "send1"),
			 -1); 
	return -1;
    }

    return 0;
}
