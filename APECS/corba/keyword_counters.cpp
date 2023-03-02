/**********************************************************
  $Header$
***********************************************************/

/***********************************************************************
 *
 * file: keyword.c 
 *
 * Take a text string and determine if it is, in fact, a valid keyword.  
 * If it is, return the value of the keyword, if not, return zero.
 * N.B.:  The token values must be nonzero.
 *
 * who      when         what
 * -------- ------------ -----------------------------------------------
 * lundahl  26 Aug 1991  Original form
 *
 ***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "keyword_counters.h"

/*
 * static struct keyword {
 *	char	*name;		 text string in upper case
 *	int	value;		 token as defined in yacc grammer
 *	int    length;		 length of keyword name 
 *	int shortform;		 length of keyword in short form 
 *  } keywords[] = { "keyword", TOKEN, length, shortform, };
 */

int keyword(char *string)
{
    struct keyword *ptr = keywords;
    int len = strlen(string);
    int i;

    /* checking keywords stringlength against declared lengths */
    /*
     *  while (ptr->length != 0) {
     *      if (strlen(ptr->name) != ptr->length)
     *          fprintf(stderr, "keyword %s, length is %d should be %d\n",
     *          ptr->name, strlen(ptr->name), ptr->length);
     *      ptr++;
     *  }
     *  ptr = keywords;
     */

    /* convert to upper case */
    for (i = 0; i < len; i++) string[i] = toupper(string[i]);

    while (ptr->length != 0) {
	/* compare with LONGFORM */
	if (len == ptr->length) {
	    if (strncmp(ptr->name, string, len) == 0) {
		return ptr->value;
	    }
	} 
	/* and then with SHORTFORM */
	else if (len == ptr->shortform) {
	    if (strncmp(ptr->name, string, ptr->shortform) == 0) {
		return ptr->value;
	    }
	}
	ptr++;
    }
    return 0;
}

/*
 * char *p=token(int value)
 * Take a token value and determine if it is a valid text string keyword.  
 * If it is, return a pointer to the string keyword, if not, return zero.
 * Version: Dec 25, 1991
 * Programmer: Lars Lundahl
 */

char *token(int value)
{
    struct keyword *ptr = keywords;

    while (ptr->length != 0) {
	if (value == ptr->value) return ptr->name;
	ptr++;
    }
    return NULL;
}
