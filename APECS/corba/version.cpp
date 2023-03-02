/**********************************************************
  $Header$
***********************************************************/

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <time.h>

char *version()
{
    struct tm *ptr;
    time_t lt;
    static char vers[] = "0000-00-00";

    time( &lt );
    ptr = localtime(&lt);
    sprintf(vers,"%4d-%02d-%02d ", 
	    1900+ptr->tm_year, ptr->tm_mon+1, ptr->tm_mday);
    return (vers);
}

char *timestamp()
{
    time_t lt;
    static char stamp[30];
    char *p;

    lt = time(NULL);
    strcpy(stamp, asctime(localtime(&lt)));
    if ((p = rindex(stamp, '\n')) != NULL) *p = '\0';
    return (stamp);
}
