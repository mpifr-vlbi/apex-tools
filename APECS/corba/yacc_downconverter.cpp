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
#include "cid_downconverter.h"


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
#define IFV1 258
#define CHAIN1 259
#define CHAIN2 260
#define STATE 261
#define SUBSTATE 262
#define STATUS 263
#define ATTEN 264
#define CMDCENTERFREQ 265
#define CENTERFREQ 266
#define CMDSYNTHFREQ 267
#define SYNTHFREQ 268
#define CMDATTEN 269
#define MODE 270
#define CMDMODE 271
#define REMOTE 272
#define LOCAL 273
#define IFINPUT 274
#define INPUT 275
#define BANDWIDTH 276
#define CMDBANDWIDTH 277
#define FREQAXISFLIPPED 278
#define RESET 279
#define LOCK 280
#define AUTOLEVEL 281
#define LEVEL 282
#define CMDINPUT 283
#define CMDIFINPUT 284
#define CONFIGURE 285
#define RX1 286
#define TEXT 287
#define COMMENT 288
#define NUMBER 289
#define INDEX 290
#define COMMAND 291
#define LONGNUM 292
#define NEG 293
#define ON 294
#define OFF 295
#define YYERRCODE 256
typedef short YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    6,    6,    6,    9,    6,    6,    7,    7,
    1,    2,    4,    5,    3,    3,    8,    8,   10,   11,
   12,   12,   12,   12,   12,   12,   12,   12,   12,   12,
   15,   15,   13,   13,   13,   13,   13,   13,   13,   13,
   13,   13,   13,   13,   13,   13,   13,   13,   13,   13,
   13,   13,   13,   13,   13,   13,   16,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   14,   14,   14,   14,   14,   14,   14,   14,   14,
   14,   17,
};
static const YYINT yylen[] = {                            2,
    0,    2,    1,    2,    2,    0,    3,    2,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    3,    3,
    3,    3,    2,    2,    2,    2,    2,    1,    1,    1,
    1,    1,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    1,    2,
    2,    2,    2,    2,    2,    2,    1,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    2,    2,    1,    2,    2,    2,    2,    2,
    2,    1,
};
static const YYINT yydefred[] = {                         1,
    0,    0,    0,    0,    0,    0,    9,   10,    2,    3,
    6,   17,   18,    8,    0,    0,    5,    4,    0,   19,
    0,    0,    0,    0,    0,    0,   28,   29,   30,   20,
    7,    0,    0,   24,   23,   25,   31,   32,   26,   27,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   49,    0,    0,    0,   21,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   75,    0,    0,    0,   22,   34,   33,
   43,   11,   35,   36,   39,   37,   38,   40,   41,   42,
   53,   50,   44,   45,   46,   47,   48,   56,   57,   51,
   52,   54,   55,   59,   58,   68,   60,   61,   64,   62,
   63,   65,   66,   67,   69,   76,   70,   71,   72,   73,
   74,   81,   82,   77,   78,   79,   80,
};
static const YYINT yydgoto[] = {                          1,
   84,    0,    0,    0,    0,    9,   10,   11,   19,   12,
   13,   30,   59,   78,   40,  101,  125,
};
static const YYINT yysindex[] = {                         0,
  -10,   14,  -45,  -44,   14,   14,    0,    0,    0,    0,
    0,    0,    0,    0, -238, -220,    0,    0,   14,    0,
  -33,  -32,  -35,  -31,  -30,  -51,    0,    0,    0,    0,
    0, -197, -173,    0,    0,    0,    0,    0,    0,    0,
  -26,  -19,  -18,  -62,  -17,  -61,  -16,  -59,  -14,  -11,
   -9,  -58,   -8,   -7,    0,   -6,  -48,  -57,    0,   -5,
   -3,   -1,  -56,   10,  -55,   11,  -54,   12,   13,   19,
  -53,   26,   34,    0,   35,  -47,  -52,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yyrindex[] = {                         0,
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
    0,    0,    0,    0,    0,    0,    0,
};
static const YYINT yygindex[] = {                         0,
  -29,    0,    0,    0,    0,    0,   16,    0,    0,    0,
   15,    0,    0,    0,    0,    0,    0,
};
#define YYTABLESIZE 281
static const YYINT yytable[] = {                          7,
   83,   86,    8,   89,   94,  102,  107,  110,  113,  118,
  126,   39,   15,   16,  100,  124,   87,   14,   90,    4,
   17,   18,   95,    7,   32,   33,    8,   34,  103,   20,
    0,   35,   36,  108,   31,  111,   79,  114,   21,   22,
   23,  119,   24,   80,   81,   85,   88,  127,   91,   25,
   26,   92,    0,   93,   96,   97,   98,  104,   27,  105,
   28,  106,    0,   41,   29,   42,   43,   44,   45,   46,
   47,   48,  109,  112,  115,  116,   49,   50,   51,   52,
   53,  117,   54,   55,   56,   57,   58,   60,  120,   61,
   62,   63,   64,   65,   66,   67,  121,  122,    0,    0,
   68,   69,   70,   71,   72,    0,   73,   74,   75,   76,
   77,    0,    0,    0,    0,    0,    0,    0,    0,    0,
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
   37,   38,    0,    0,    0,    0,   82,   82,    0,   82,
   82,   82,   82,   82,   82,   82,   82,   99,  123,    0,
    0,    0,    0,    0,    0,    2,    3,    4,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    5,    0,    0,
    6,
};
static const YYINT yycheck[] = {                         10,
   63,   63,   13,   63,   63,   63,   63,   63,   63,   63,
   63,   63,   58,   58,   63,   63,   46,    2,   48,  258,
    5,    6,   52,   10,   58,   58,   13,   63,   58,   15,
   -1,   63,   63,   63,   19,   65,   63,   67,  259,  260,
  261,   71,  263,   63,   63,   63,   63,   77,   63,  270,
  271,   63,   -1,   63,   63,   63,   63,   63,  279,   63,
  281,   63,   -1,  261,  285,  263,  264,  265,  266,  267,
  268,  269,   63,   63,   63,   63,  274,  275,  276,  277,
  278,   63,  280,  281,  282,  283,  284,  261,   63,  263,
  264,  265,  266,  267,  268,  269,   63,   63,   -1,   -1,
  274,  275,  276,  277,  278,   -1,  280,  281,  282,  283,
  284,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
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
  272,  273,   -1,   -1,   -1,   -1,  289,  289,   -1,  289,
  289,  289,  289,  289,  289,  289,  289,  286,  286,   -1,
   -1,   -1,   -1,   -1,   -1,  256,  257,  258,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  288,   -1,   -1,
  291,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 295
#define YYUNDFTOKEN 315
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
0,0,0,0,0,"APEX","IFV1","CHAIN1","CHAIN2","STATE","SUBSTATE","STATUS","ATTEN",
"CMDCENTERFREQ","CENTERFREQ","CMDSYNTHFREQ","SYNTHFREQ","CMDATTEN","MODE",
"CMDMODE","REMOTE","LOCAL","IFINPUT","INPUT","BANDWIDTH","CMDBANDWIDTH",
"FREQAXISFLIPPED","RESET","LOCK","AUTOLEVEL","LEVEL","CMDINPUT","CMDIFINPUT",
"CONFIGURE","RX1","TEXT","COMMENT","NUMBER","INDEX","COMMAND","LONGNUM","NEG",
"ON","OFF",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"illegal-symbol",
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
"apexdevs : IFV1 ':' ifv1",
"ifv1 : CHAIN1 ':' chain1",
"ifv1 : CHAIN2 ':' chain2",
"ifv1 : STATUS '?'",
"ifv1 : STATE '?'",
"ifv1 : MODE '?'",
"ifv1 : CMDMODE '?'",
"ifv1 : CMDMODE controlmode",
"ifv1 : RESET",
"ifv1 : AUTOLEVEL",
"ifv1 : CONFIGURE",
"controlmode : REMOTE",
"controlmode : LOCAL",
"chain1 : STATUS '?'",
"chain1 : STATE '?'",
"chain1 : CMDCENTERFREQ '?'",
"chain1 : CMDCENTERFREQ num",
"chain1 : CMDSYNTHFREQ '?'",
"chain1 : CMDSYNTHFREQ num",
"chain1 : CENTERFREQ '?'",
"chain1 : SYNTHFREQ '?'",
"chain1 : CMDATTEN '?'",
"chain1 : CMDATTEN num",
"chain1 : ATTEN '?'",
"chain1 : BANDWIDTH '?'",
"chain1 : CMDBANDWIDTH '?'",
"chain1 : CMDBANDWIDTH num",
"chain1 : FREQAXISFLIPPED '?'",
"chain1 : LOCK '?'",
"chain1 : AUTOLEVEL",
"chain1 : INPUT '?'",
"chain1 : CMDINPUT '?'",
"chain1 : CMDINPUT input1",
"chain1 : IFINPUT '?'",
"chain1 : CMDIFINPUT '?'",
"chain1 : CMDIFINPUT num",
"chain1 : LEVEL '?'",
"input1 : RX1",
"chain2 : STATUS '?'",
"chain2 : STATE '?'",
"chain2 : CMDCENTERFREQ '?'",
"chain2 : CMDCENTERFREQ num",
"chain2 : CMDSYNTHFREQ '?'",
"chain2 : CMDSYNTHFREQ num",
"chain2 : CENTERFREQ '?'",
"chain2 : SYNTHFREQ '?'",
"chain2 : CMDATTEN '?'",
"chain2 : CMDATTEN num",
"chain2 : ATTEN '?'",
"chain2 : IFINPUT '?'",
"chain2 : BANDWIDTH '?'",
"chain2 : CMDBANDWIDTH '?'",
"chain2 : CMDBANDWIDTH num",
"chain2 : FREQAXISFLIPPED '?'",
"chain2 : LOCK '?'",
"chain2 : AUTOLEVEL",
"chain2 : INPUT '?'",
"chain2 : CMDINPUT '?'",
"chain2 : CMDINPUT input2",
"chain2 : CMDIFINPUT '?'",
"chain2 : CMDIFINPUT num",
"chain2 : LEVEL '?'",
"input2 : RX1",

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

#include "lex_downconverter.h"	/* User coded lex routine must be included here */

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
	{IFV1Status(yystack.l_mark[-1].cmd,0); }
break;
case 24:
	{IFV1Status(yystack.l_mark[-1].cmd,0); }
break;
case 25:
	{IFV1Comm(yystack.l_mark[-1].cmd,0); }
break;
case 26:
	{IFV1Comm(yystack.l_mark[-1].cmd,0); }
break;
case 27:
	{ }
break;
case 28:
	{IFV1Reset(yystack.l_mark[0].cmd);}
break;
case 29:
	{IFV1AutoLevel(yystack.l_mark[0].cmd,0); }
break;
case 30:
	{IFV1Configure(yystack.l_mark[0].cmd);}
break;
case 31:
	{IFV1Set(yystack.l_mark[0].cmd,0,0);}
break;
case 32:
	{IFV1Set(yystack.l_mark[0].cmd,0,0);}
break;
case 33:
	{IFV1Status(yystack.l_mark[-1].cmd,1); }
break;
case 34:
	{IFV1Status(yystack.l_mark[-1].cmd,1); }
break;
case 35:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 36:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,1); }
break;
case 37:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 38:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,1); }
break;
case 39:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 40:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 41:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 42:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,1); }
break;
case 43:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 44:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 45:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 46:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,1); }
break;
case 47:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 48:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 49:
	{IFV1AutoLevel(yystack.l_mark[0].cmd,1); }
break;
case 50:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 51:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 52:
	{ }
break;
case 53:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 54:
	{IFV1Comm(yystack.l_mark[-1].cmd,1); }
break;
case 55:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,1);}
break;
case 56:
	{IFV1GetLevel(yystack.l_mark[-1].cmd,1); }
break;
case 57:
	{IFV1Set(yystack.l_mark[0].cmd,0,1);}
break;
case 58:
	{IFV1Status(yystack.l_mark[-1].cmd,2); }
break;
case 59:
	{IFV1Status(yystack.l_mark[-1].cmd,2); }
break;
case 60:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 61:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,2); }
break;
case 62:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 63:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,2); }
break;
case 64:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 65:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 66:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 67:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,2); }
break;
case 68:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 69:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 70:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 71:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 72:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,2); }
break;
case 73:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 74:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 75:
	{IFV1AutoLevel(yystack.l_mark[0].cmd,2); }
break;
case 76:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 77:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 78:
	{ }
break;
case 79:
	{IFV1Comm(yystack.l_mark[-1].cmd,2); }
break;
case 80:
	{IFV1Set(yystack.l_mark[-1].cmd,yystack.l_mark[0].real,2);}
break;
case 81:
	{IFV1GetLevel(yystack.l_mark[-1].cmd,2); }
break;
case 82:
	{IFV1Set(yystack.l_mark[0].cmd,0,2);}
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
