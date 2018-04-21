/* drudg.f -- translated by f2c (version 20160102).
   You must link the resulting object file with libf2c:
	on Microsoft Windows system, link with libf2c.lib;
	on Linux or Unix systems, link with .../path/to/libf2c.a -lm
	or, if you install libf2c.a in a standard place, with -lf2c -lm
	-- in that order, at the end of the command line, as in
		cc *.o -lf2c -lm
	Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

		http://www.netlib.org/f2c/libf2c.zip
*/

#include "f2c.h"

/* Main program */ int MAIN__(void)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    integer s_rnge(char *, integer, char *, integer);

    /* Local variables */
    static integer i__;
    static char arg[256*7];
    extern /* Subroutine */ int getarg_(integer *, char *, ftnlen), fdrudg_(
	    char *, char *, char *, char *, char *, char *, char *, ftnlen, 
	    ftnlen, ftnlen, ftnlen, ftnlen, ftnlen, ftnlen);



    for (i__ = 1; i__ <= 7; ++i__) {
	getarg_(&i__, arg + (((i__1 = i__ - 1) < 7 && 0 <= i__1 ? i__1 : 
		s_rnge("arg", i__1, "drudg_", (ftnlen)8)) << 8), (ftnlen)256);
    }
    fdrudg_(arg, arg + 256, arg + 512, arg + 768, arg + 1024, arg + 1280, arg 
	    + 1536, (ftnlen)256, (ftnlen)256, (ftnlen)256, (ftnlen)256, (
	    ftnlen)256, (ftnlen)256, (ftnlen)256);

    return 0;
} /* MAIN__ */

/* Main program alias */ int drudg_ () { MAIN__ (); return 0; }
