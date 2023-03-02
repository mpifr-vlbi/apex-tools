/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20170709

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include "cid_counters.h"


extern int newsockfd;
extern char query_buf[];
extern int query_len;
extern double cmdListVal[];
extern int cmdListLen;
extern int cmdPortsVal[];
extern int cmdPortsLen;

/*void writen(), events(), usage(), query(), reset(), syntax(), usage();*/

#define YACC                            /* tells struct.h about extern */
#include "struct.h"

char textbuf[80];
extern int lineno;
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
	double    real;    /* real double precision value */
	int    integer;    /* integer value */
        long   longnum;    /* long value    */
	int        cmd;    /* command value */
	char     *text;    /* text string buffer */
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define APEX 257
#define COUNTERS 258
#define GPSMINUSFMOUT 259
#define GPSMINUSMASER 260
#define STATE 261
#define STATUS 262
#define RESET 263
#define CONFIGURE 264
#define ON 265
#define OFF 266
#define REFERENCE 267
#define TEXT 268
#define COMMENT 269
#define NUMBER 270
#define INDEX 271
#define COMMAND 272
#define LONGNUM 273
#define NEG 274
#define YYERRCODE 256
typedef short YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    6,    6,    6,    9,    6,    6,    7,    7,
    1,    2,    4,    5,    3,    3,    8,    8,   10,   11,
   12,   12,   12,   12,   12,   12,   12,   12,   13,   13,
   13,   13,   13,   13,   13,   13,   14,   14,   14,   14,
   14,   14,   14,   14,
};
static const YYINT yylen[] = {                            2,
    0,    2,    1,    2,    2,    0,    3,    2,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    3,    3,
    3,    3,    2,    2,    1,    1,    1,    1,    2,    2,
    1,    1,    1,    1,    2,    2,    2,    2,    1,    1,
    1,    1,    2,    2,
};
static const YYINT yydefred[] = {                         1,
    0,    0,    0,    0,    0,    0,    9,   10,    2,    3,
    6,   17,   18,    8,    0,    0,    5,    4,    0,   19,
    0,    0,    0,    0,   25,   26,   27,   28,   20,    7,
    0,    0,   24,   23,    0,    0,    0,   31,   32,   33,
   34,    0,   21,    0,    0,    0,   39,   40,   41,   42,
    0,   22,   35,   30,   29,   36,   43,   38,   37,   44,
};
static const YYINT yydgoto[] = {                          1,
    0,    0,    0,    0,    0,    9,   10,   11,   19,   12,
   13,   29,   43,   52,
};
static const YYINT yysindex[] = {                         0,
  -10,   25,  -56,  -51,   25,   25,    0,    0,    0,    0,
    0,    0,    0,    0, -243, -240,    0,    0,   25,    0,
  -42,  -41,  -27,  -26,    0,    0,    0,    0,    0,    0,
 -253, -233,    0,    0,  -24,  -23,  -22,    0,    0,    0,
    0,  -21,    0,  -20,  -19,  -18,    0,    0,    0,    0,
  -17,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yygindex[] = {                         0,
    0,    0,    0,    0,    0,    0,   -1,    0,    0,    0,
   32,    0,    0,    0,
};
#define YYTABLESIZE 262
static const YYINT yytable[] = {                          7,
   14,   15,    8,   17,   18,   35,   16,   36,   37,   38,
   39,   40,   41,   42,    4,   31,   32,   30,   21,   22,
   23,   24,   25,   26,   27,   28,   44,   45,   46,   47,
   48,   49,   50,   51,    7,   33,   34,    8,   53,   54,
   55,   56,   57,   58,   59,   60,   20,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    2,    3,    4,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    5,    0,
    0,    6,
};
static const YYINT yycheck[] = {                         10,
    2,   58,   13,    5,    6,  259,   58,  261,  262,  263,
  264,  265,  266,  267,  258,   58,   58,   19,  259,  260,
  261,  262,  263,  264,  265,  266,  260,  261,  262,  263,
  264,  265,  266,  267,   10,   63,   63,   13,   63,   63,
   63,   63,   63,   63,   63,   63,   15,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  256,  257,  258,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  269,   -1,
   -1,  272,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 274
#define YYUNDFTOKEN 291
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,"'\\r'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,
"':'",0,0,0,0,"'?'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'^'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,"APEX","COUNTERS","GPSMINUSFMOUT","GPSMINUSMASER","STATE","STATUS",
"RESET","CONFIGURE","ON","OFF","REFERENCE","TEXT","COMMENT","NUMBER","INDEX",
"COMMAND","LONGNUM","NEG",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : input",
"input :",
"input : input line",
"line : eol",
"line : COMMAND eol",
"line : COMMENT eol",
"$$1 :",
"line : command $$1 eol",
"line : error eol",
"eol : '\\n'",
"eol : '\\r'",
"num : NUMBER",
"index : INDEX",
"string : TEXT",
"longnum : LONGNUM",
"bool : ON",
"bool : OFF",
"command : apex",
"command : apexdevs",
"apex : APEX ':' apexdevs",
"apexdevs : COUNTERS ':' counters",
"counters : GPSMINUSFMOUT ':' gpsminusfmout",
"counters : GPSMINUSMASER ':' gpsminusmaser",
"counters : STATUS '?'",
"counters : STATE '?'",
"counters : RESET",
"counters : CONFIGURE",
"counters : ON",
"counters : OFF",
"gpsminusfmout : STATUS '?'",
"gpsminusfmout : STATE '?'",
"gpsminusfmout : RESET",
"gpsminusfmout : CONFIGURE",
"gpsminusfmout : ON",
"gpsminusfmout : OFF",
"gpsminusfmout : GPSMINUSFMOUT '?'",
"gpsminusfmout : REFERENCE '?'",
"gpsminusmaser : STATUS '?'",
"gpsminusmaser : STATE '?'",
"gpsminusmaser : RESET",
"gpsminusmaser : CONFIGURE",
"gpsminusmaser : ON",
"gpsminusmaser : OFF",
"gpsminusmaser : GPSMINUSMASER '?'",
"gpsminusmaser : REFERENCE '?'",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;

#include "lex_counters.h"	/* User coded lex routine must be included here */

#if YYDEBUG
#include <stdio.h>	/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yym = 0;
    yyn = 0;
    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        yychar = YYLEX;
        if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if (((yyn = yysindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if (((yyn = yyrindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag != 0) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab; /* redundant goto avoids 'unused label' warning */
yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if (((yyn = yysindex[*yystack.s_mark]) != 0) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym > 0)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);

    switch (yyn)
    {
case 2:
	{ writen(query_buf, query_len+1);
                         query_buf[0] = '\0'; query_len = 0;}
break;
case 3:
	{ }
break;
case 4:
	{ events("undef", textbuf); goto Error; }
break;
case 5:
	{ events("comment", yylval.text); }
break;
case 6:
	{ }
break;
case 8:
	{ Error: yyerrok; writen("?", 1); }
break;
case 11:
	{ if (yystack.l_mark[0].real > 1e+300) { yyval.real = 0; goto Error;} yyval.real = yystack.l_mark[0].real; }
break;
case 12:
	{ yyval.integer = yystack.l_mark[0].integer; }
break;
case 13:
	{ yyval.text = yystack.l_mark[0].text; }
break;
case 14:
	{ if (yystack.l_mark[0].longnum > 21474836) { yyval.longnum = 0; goto Error;} yyval.longnum = yystack.l_mark[0].longnum; }
break;
case 15:
	{ yyval.integer = SET; }
break;
case 16:
	{ yyval.integer = CLR; }
break;
case 19:
	{ }
break;
case 20:
	{ }
break;
case 21:
	{ }
break;
case 22:
	{ }
break;
case 23:
	{CountersStatus(yystack.l_mark[-1].cmd,0); }
break;
case 24:
	{CountersStatus(yystack.l_mark[-1].cmd,0); }
break;
case 25:
	{CountersConfigure(yystack.l_mark[0].cmd,0);}
break;
case 26:
	{CountersConfigure(yystack.l_mark[0].cmd,0);}
break;
case 27:
	{CountersEnable(yystack.l_mark[0].cmd,0);}
break;
case 28:
	{CountersEnable(yystack.l_mark[0].cmd,0);}
break;
case 29:
	{CountersStatus(yystack.l_mark[-1].cmd,1); }
break;
case 30:
	{CountersStatus(yystack.l_mark[-1].cmd,1); }
break;
case 31:
	{CountersConfigure(yystack.l_mark[0].cmd,1);}
break;
case 32:
	{CountersConfigure(yystack.l_mark[0].cmd,1);}
break;
case 33:
	{CountersEnable(yystack.l_mark[0].cmd,1);}
break;
case 34:
	{CountersEnable(yystack.l_mark[0].cmd,1);}
break;
case 35:
	{CountersComm(yystack.l_mark[-1].cmd,1); }
break;
case 36:
	{CountersCheckExtRef(yystack.l_mark[-1].cmd,1); }
break;
case 37:
	{CountersStatus(yystack.l_mark[-1].cmd,2); }
break;
case 38:
	{CountersStatus(yystack.l_mark[-1].cmd,2); }
break;
case 39:
	{CountersConfigure(yystack.l_mark[0].cmd,2);}
break;
case 40:
	{CountersConfigure(yystack.l_mark[0].cmd,2);}
break;
case 41:
	{CountersEnable(yystack.l_mark[0].cmd,2);}
break;
case 42:
	{CountersEnable(yystack.l_mark[0].cmd,2);}
break;
case 43:
	{CountersComm(yystack.l_mark[-1].cmd,2); }
break;
case 44:
	{CountersCheckExtRef(yystack.l_mark[-1].cmd,2); }
break;
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            yychar = YYLEX;
            if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if (((yyn = yygindex[yym]) != 0) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
