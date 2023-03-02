/**********************************************************
  $Header$
***********************************************************/

#include <stdio.h>
#include <strings.h>
#include <string.h>

#define MAX_EVENT 16
#define EVENT_LENGTH 256
 
extern int lineno;
char *timestamp();

struct {
   int spos;		/* store index,should begin with 0             */
   int rpos;		/* retrieve index,should begin with MAX_EVENT  */
   char queue[MAX_EVENT][EVENT_LENGTH];	/* circular event queue buffer */
} event = { 0, MAX_EVENT };

/* retrieve an event from queue */
char *retrieve_event()  		
{
    if (event.rpos == MAX_EVENT)      event.rpos = 0;
    if (event.rpos == event.spos)     return "\0";
    event.rpos++;
    return event.queue[event.rpos-1];
}

/* store an event in queue */
void store_event(char *s)			
{
    if (event.spos+1 == event.rpos) (void)retrieve_event();
    strncpy(event.queue[event.spos], s, EVENT_LENGTH-1);
    event.spos++;
    if (event.spos == MAX_EVENT) event.spos = 0;
}

/* enter events */
void enter_event(char *s,int line)			
{
    char p[EVENT_LENGTH];
    char online[16];

    strcpy(p, timestamp());
    strcat(p, s);
    if (line > 0) {
        sprintf( online," on line %d.", lineno);
        strcat(p, online);
    }
    if (*s) store_event(p);
}

void events(char *t, char *s)  		/* Called by yacc.y on events */
{
    char r[256];

    switch (t[0]) {
      case 'e': enter_event(s, lineno); break;		/* error */
      case 'w': enter_event(s, lineno); break;		/* warning */
      case 'u': strcpy(r, "undefined symbol '");	/* undefined symbol */
	        strcat(r, s); strcat(r, "'");	
	        enter_event(r, lineno); break;
      case 'c': enter_event(s, 0); break;		/* comment */
      case 'm': enter_event(s, 0); break;		/* message */
    }
}

/* see what's in the event queue */
void view_event()			
{		
    register int    t;

    if (event.rpos > event.spos) {
	for (t = event.rpos; t < MAX_EVENT; ++t)
	  printf("%d. %s\n", t, event.queue[t]);
	for (t = 0; t < event.spos; ++t)
	  printf("%d. %s\n", t, event.queue[t]);
    } else {
	for (t = event.rpos; t < event.spos; ++t)
	  printf("%d. %s\n", t, event.queue[t]);
    }
}
