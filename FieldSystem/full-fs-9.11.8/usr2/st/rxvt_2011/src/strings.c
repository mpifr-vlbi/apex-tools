/*--------------------------------*-C-*---------------------------------*
 * File:	strings.c
 *----------------------------------------------------------------------*
 * $Id: strings.c,v 1.14 2001/07/27 06:38:37 gcw Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997-2001 Geoff Wing <gcw@pobox.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "strings.intpro"	/* PROTOS for internal routines */

#ifndef NO_STDLIB
/*----------------------------------------------------------------------*/
/*
 * a replacement for strcasecmp() to avoid linking an entire library.
 * Mark Olesen added this in 2.15 but for which OS & library? - Geoff Wing
 */
/* EXTPROTO */
int
strcasecmp(const char *s1, const char *s2)
{
    for ( ; tolower(*s1) == tolower(*s2); s1++, s2++)
	if (!*s1)
	    return 0;
    return (int)(tolower(*s1) - tolower(*s2));
}

/* EXTPROTO */
int
strncasecmp(const char *s1, const char *s2, size_t n)
{
    for ( ; n-- && (tolower(*s1) == tolower(*s2)); s1++, s2++)
	if (!*s1)
	    return 0;
    if (n == 0)
	return 0;
    return (int)(tolower(*s1) - tolower(*s2));
}

/* EXTPROTO */
char           *
strcpy(char *d, const char *s)
{
    char          *r = d;

    for ( ; (*r++ = *s++) != '\0'; ) ;
    return d;
}

/* EXTPROTO */
char           *
strncpy(char *d, const char *s, size_t len)
{
    char          *r = d;

    if (len)
	for ( ; len; len--)
	    if ((*r++ = *s++) == '\0') {
		for ( ; --len; )
		    *r++ = '\0';
		break;
	    }
    return d;
}

/* EXTPROTO */
int
strcmp(const char *s1, const char *s2)
{
    for ( ; (*s1 == *s2++); )
	if (*s1++ == '\0')
	    return 0;
    return (int) ((unsigned char) *s1 - (unsigned char) *--s2);
}

/* EXTPROTO */
int
strncmp(const char *s1, const char *s2, size_t len)
{
    if (len) {
	for ( ; len-- && (*s1++ == *s2++); ) ;
	if (++len)
	    return (int) ((unsigned char) *--s1 - (unsigned char) *--s2);
    }
    return 0;
}

/* EXTPROTO */
char           *
strcat(char *s1, const char *s2)
{
    char           *r = s1;

    if (*r != '\0')
	for ( ; *++r != '\0'; ) ;
    for ( ; (*r++ = *s2++) != '\0'; ) ;

    return s1;
}

/* EXTPROTO */
char           *
strncat(char *s1, const char *s2, size_t len)
{
    char           *r = s1;

    if (*r != '\0')
	for ( ; *++r != '\0'; ) ;
    for ( ; len-- && ((*r++ = *s2++) != '\0'); ) ;
    *r = '\0';

    return s1;
}

/* EXTPROTO */
size_t
strlen(const char *s)
{
    size_t         len = 0;

    for ( ; *s++ != '\0'; len++) ;
    return len;
}

/* EXTPROTO */
char           *
strdup(const char *s)
{
    size_t         len = STRLEN(s) + 1;
    char          *c;

    if ((c = malloc(len)) != NULL)
	MEMCPY(c, s, len);
    return c;
}

/* EXTPROTO */
char           *
index(const char *s, int c)
{
    return STRCHR(s, c);
}

/* EXTPROTO */
char           *
strchr(const char *s, int c)
{
    char          *p = NULL;

    for (;;) {
	if (*s == (char)c) {
	    p = (char *)s;
	    break;
	}
	if (*s++ == '\0')
	    break;
    }
    return p;

}

/* EXTPROTO */
char           *
rindex(const char *s, int c)
{
    return STRRCHR(s, c);
}

/* EXTPROTO */
char           *
strrchr(const char *s, int c)
{
    char          *p = NULL;

    for (;;) {
	if (*s == (char)c)
	    p = (char *)s;
	if (*s++ == '\0')
	    break;
    }
    return p;
}

/* EXTPROTO */
void           *
memcpy(void *s1, const void *s2, size_t len)
{
    /* has extra stack and time but less code space */
    return MEMMOVE(s1, s2, len);
}

/*--------------------------------------------------------------------------*
 * Possibly faster memmove() by Geoff Wing <mason@primenet.com.au>
 *--------------------------------------------------------------------------*/
/* EXTPROTO */
void           *
memmove(void *d, const void *s, size_t len)
{
    u_intp_t        i;
    unsigned char  *dst = (unsigned char *)d;
    const unsigned char *src = (const unsigned char *)s;

    if (len && d != s) {
	if ((u_intp_t)d < (u_intp_t)s) {
	/* forwards */
	    i = (-(u_intp_t)dst) & (SIZEOF_INT_P - 1);
	    if (len >= 16 && i == ((-(u_intp_t)src) & (SIZEOF_INT_P - 1))) {
	    /* speed up since src & dst are offset correctly */
		len -= (size_t)i;
		for ( ; i--; )
		    *dst++ = *src++;
		for (i = (u_intp_t)(len / SIZEOF_INT_P); i--; )
		    *((u_intp_t *)dst)++ = *((const u_intp_t *)src)++;
		len &= (SIZEOF_INT_P - 1);
	    }
	    for ( ; len--; )
		*dst++ = *src++;
	} else {
	/* backwards */
	    dst += len;
	    src += len;
	    i = ((u_intp_t)dst) & (SIZEOF_INT_P - 1);
	    if (len >= 16 && i == (((u_intp_t)src) & (SIZEOF_INT_P - 1))) {
	    /* speed up since src & dst are offset correctly */
		len -= (size_t)i;
		for ( ; i--; )
		    *--dst = *--src;
		for (i = (u_intp_t)(len / SIZEOF_INT_P); i--; )
		    *--((u_intp_t *)dst) = *--((const u_intp_t *)src);
		len &= (SIZEOF_INT_P - 1);
	    }
	    for ( ; len--; )
		*--dst = *--src;
	}
    }
    return d;
}

/*--------------------------------------------------------------------------*
 * Possibly faster memset() by Geoff Wing <mason@primenet.com.au>
 * presumptions:
 *   1) intp_t write the best
 *   2) SIZEOF_INT_P == power of 2
 *--------------------------------------------------------------------------*/

/* EXTPROTO */
void
bzero(void *b, size_t len)
{
    MEMSET(b, 0, len);
}

/* EXTPROTO */
void           *
memset(void *p, int c1, size_t len)
{
    u_intp_t        i, val;
    unsigned char   c = (unsigned char) c1;
    unsigned char  *lp = (unsigned char *) p;

    if (len) {
	if (len >= 16) { /* < 16 probably not worth all the calculations */
/* write out preceding characters so we align on an integer boundary */
	    if ((i = ((-(u_intp_t)p) & (SIZEOF_INT_P - 1)))) {
		len -= (size_t)i;
		for (; i--;)
		    *lp++ = c;
	    }

/* do the fast writing */
	    val = (c << 8) + c;
#if SIZEOF_INT_P >= 4
	    val |= (val << 16);
#endif
#if SIZEOF_INT_P >= 8
	    val |= (val << 32);
#endif
#if SIZEOF_INT_P == 16
	    val |= (val << 64);
#endif
	    for (i = (u_intp_t)(len / SIZEOF_INT_P); i--;)
		*((u_intp_t *)lp)++ = val;
	    len &= (SIZEOF_INT_P - 1);
	}
/* write trailing characters */
	for (; len--;)
	    *lp++ = c;
    }
    return p;
}
#endif
/*----------------------- end-of-file (C source) -----------------------*/
