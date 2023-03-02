/**********************************************************
  $Header$
***********************************************************/

/***********************************************************************
 *
 * file: errors.c
 *
 * The functions in this file are independent of any application
 * variables, and may be used with any C program.
 *
 *	 err_dump(str)	Fatal error.  Print a message,	dump core (for
 *			debugging) and terminate.
 *	 err_sys(str)	Fatal error related to a system call.
 *			Print a message and terminate.
 *			Don't dump core, but do print the system's errno
 *			value and its associated message.
 *
 * who      when         what
 * -------- ------------ -----------------------------------------------
 * lundahl   7 Sep 1992  Modified for OS9
 * olberg   15 Sep 1992  included 'errno' support
 *
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#ifndef OSK
extern int code;
#endif

extern char *pname;

void err_dump(char *str)
{
    if (pname != NULL) fprintf(stderr, "%s: ", pname);
    fprintf(stderr, "%s", str);
    fprintf(stderr, " (errno = %d)\n", errno);

    fflush(stdout);		/* abort doesn't flush stdio buffers */
    fflush(stderr);

    abort();			/* dump core and terminate */
    _exit(errno);		/* shouldn't get here */
}

void err_sys(char *str)
{
    if (pname != NULL) fprintf(stderr, "%s: ", pname);
    fprintf(stderr, "%s", str);
    fprintf(stderr, " (errno = %d)\n", errno);

    _exit(errno);
}
