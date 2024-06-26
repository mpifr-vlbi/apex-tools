/*
 * $Id: rxvtlib.h.in,v 1.10 2003/02/28 00:58:43 gcw Exp $
 */

#ifndef _RXVTLIB_H_		/* include once only */
#define _RXVTLIB_H_

/*
 * section 1 generated by GNU autoconf for i686-pc-linux-gnu
 * this section may be changed as appropriate _before_ building
 */
/*****************************************************************************
 *                                 SECTION 1                                 *
 *****************************************************************************/

/*
 * The following line MUST not be changed without also changing
 * config.h in the main directory before building
 */
/* #undef MULTICHAR_SET */

/*****************************************************************************
 *                                 INCLUDES                                  *
 *****************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
/* #include <util.h> */
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/select.h>
/* #include <sys/strredir.h> */

#include <sys/wait.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>	/* Xlib, Xutil, Xresource, Xfuncproto */

/*
 * If we haven't pulled in typedef's like  int16_t  then do them ourself
 * type of (normal and unsigned) basic sizes
 */
/* typedef short int16_t; */
/* typedef unsigned short u_int16_t; */
/* typedef int int32_t; */
/* typedef unsigned int u_int32_t; */
/* typedef long long int64_t; */
/* typedef unsigned long long u_int64_t; */

/* whatever normal size corresponds to a integer pointer */
#define intp_t int32_t
/* whatever normal size corresponds to a unsigned integer pointer */
#define u_intp_t u_int32_t

/*****************************************************************************
 *                                 SECTION 2                                 *
 *                      DO NOT TOUCH ANYTHING BELOW HERE                     *
 *****************************************************************************/

struct rxvt_vars;		/* defined later on */
struct rxvt_hidden;		/* not defined here */

#define scrollbar_visible(rxvtvars)	((rxvtvars)->scrollBar.state)
#define menubar_visible(rxvtvars)	((rxvtvars)->menuBar.state)

typedef struct {
    int32_t         row;
    int32_t         col;
} row_col_t;

typedef unsigned char text_t;
#if defined(TTY_256COLOR) || defined(MULTICHAR_SET)
#define rend_t		u_int32_t
#else
#define rend_t		u_int16_t
#endif

/*
 * TermWin elements limits
 *  width     : 1 <= width
 *  height    : 1 <= height
 *  ncol      : 1 <= ncol       <= MAX(int16_t)
 *  nrow      : 1 <= nrow       <= MAX(int16_t)
 *  saveLines : 0 <= saveLines  <= MAX(int16_t)
 *  nscrolled : 0 <= nscrolled  <= saveLines
 *  view_start: 0 <= view_start <= nscrolled
 */

typedef struct {
    u_int16_t       width;	/* window width                    [pixels] */
    u_int16_t       height;	/* window height                   [pixels] */
    u_int16_t       fwidth;	/* font width                      [pixels] */
    u_int16_t       fheight;	/* font height                     [pixels] */
    u_int16_t       propfont;	/* font proportional flags                  */
    u_int16_t       ncol;	/* window columns              [characters] */
    u_int16_t       nrow;	/* window rows                 [characters] */
    u_int16_t       focus;	/* window has focus                         */
    u_int16_t       mapped;	/* window state mapped?                     */
    u_int16_t       int_bwidth; /* internal border width                    */
    u_int16_t       ext_bwidth; /* external border width                    */
    u_int16_t       lineSpace;	/* number of extra pixels between rows      */
    u_int16_t       saveLines;	/* number of lines that fit in scrollback   */
    u_int16_t       nscrolled;	/* number of line actually scrolled         */
    u_int16_t       view_start;	/* scrollback view starts here              */
    Window          parent[6];	/* parent identifiers - we're parent[0]     */
    Window          vt;		/* vt100 window                             */
    GC              gc;		/* GC for drawing text                      */
    XFontStruct    *font;	/* main font structure                      */
    XFontStruct    *boldFont;	/* bold font                                */
    XFontStruct    *boldFont_loaded;	/* bold font loaded                 */
    XFontStruct    *mfont;	/* Multichar font structure                 */
    XFontSet        fontset;
    Pixmap          pixmap;
} TermWin_t;

/*
 * screen accounting:
 * screen_t elements
 *   text:      Contains all text information including the scrollback buffer.
 *              Each line is length TermWin.ncol
 *   tlen:      The length of the line or -1 for wrapped lines.
 *   rend:      Contains rendition information: font, bold, colour, etc.
 * * Note: Each line for both text and rend are only allocated on demand, and
 *         text[x] is allocated <=> rend[x] is allocated  for all x.
 *   row:       Cursor row position                   : 0 <= row < TermWin.nrow
 *   col:       Cursor column position                : 0 <= col < TermWin.ncol
 *   tscroll:   Scrolling region top row inclusive    : 0 <= row < TermWin.nrow
 *   bscroll:   Scrolling region bottom row inclusive : 0 <= row < TermWin.nrow
 *
 * selection_t elements
 *   clicks:    1, 2 or 3 clicks - 4 indicates a special condition of 1 where
 *              nothing is selected
 *   beg:       row/column of beginning of selection  : never past mark
 *   mark:      row/column of initial click           : never past end
 *   end:       row/column of one character past end of selection
 * * Note: -TermWin.nscrolled <= beg.row <= mark.row <= end.row < TermWin.nrow
 * * Note: col == -1 ==> we're left of screen
 *
 * Layout of text/rend information in the screen_t text/rend structures:
 *   Rows [0] ... [TermWin.saveLines - 1]
 *     scrollback region : we're only here if TermWin.view_start != 0
 *   Rows [TermWin.saveLines] ... [TermWin.saveLines + TermWin.nrow - 1]
 *     normal `unscrolled' screen region
 */
typedef struct {
    text_t        **text;	/* _all_ the text                            */
    int16_t	   *tlen;	/* length of each text line                  */
    rend_t        **rend;	/* rendition, uses RS_ flags                 */
    row_col_t       cur;	/* cursor position on the screen             */
    u_int16_t       tscroll;	/* top of settable scroll region             */
    u_int16_t       bscroll;	/* bottom of settable scroll region          */
    u_int16_t       charset;	/* character set number [0..3]               */
    unsigned int    flags;	/* see below                                 */
    row_col_t	    s_cur;	/* saved cursor position                     */
    u_int16_t	    s_charset;	/* saved character set number [0..3]         */
    char            s_charset_char;
    rend_t          s_rstyle;	/* saved rendition style                     */
} screen_t;

typedef struct {
    unsigned char  *text;	/* selected text                             */
    u_int32_t       len;	/* length of selected text                   */
    enum {
	SELECTION_CLEAR = 0,	/* nothing selected                          */
	SELECTION_INIT,		/* marked a point                            */
	SELECTION_BEGIN,	/* started a selection                       */
	SELECTION_CONT,		/* continued selection                       */
	SELECTION_DONE		/* selection put in CUT_BUFFER0              */
    } op;			/* current operation                         */
    short           screen;	/* screen being used                         */
    short           clicks;	/* number of clicks                          */
    row_col_t       beg;	/* beginning of selection   <= mark          */
    row_col_t       mark;	/* point of initial click   <= end           */
    row_col_t       end;	/* one character past end point              */
} selection_t;

typedef enum {
    OLD_SELECT, OLD_WORD_SELECT, NEW_SELECT
} sstyle_t;

/* ------------------------------------------------------------------------- */

/* screen_t flags */
#define Screen_Relative		(1<<0)	/* relative origin mode flag         */
#define Screen_VisibleCursor	(1<<1)	/* cursor visible?                   */
#define Screen_Autowrap		(1<<2)	/* auto-wrap flag                    */
#define Screen_Insert		(1<<3)	/* insert mode (vs. overstrike)      */
#define Screen_WrapNext		(1<<4)	/* need to wrap for next char?       */
#define Screen_DefaultFlags	(Screen_VisibleCursor|Screen_Autowrap)

/* rxvt_vars.Options */
#define	Opt_console		(1LU<<0)
#define Opt_loginShell		(1LU<<1)
#define Opt_iconic		(1LU<<2)
#define Opt_visualBell		(1LU<<3)
#define Opt_mapAlert		(1LU<<4)
#define Opt_reverseVideo	(1LU<<5)
#define Opt_utmpInhibit		(1LU<<6)
#define Opt_scrollBar		(1LU<<7)
#define Opt_scrollBar_right	(1LU<<8)
#define Opt_scrollBar_floating	(1LU<<9)
#define Opt_meta8		(1LU<<10)
#define Opt_scrollTtyOutput	(1LU<<11)
#define Opt_scrollTtyKeypress	(1LU<<12)
#define Opt_transparent		(1LU<<13)
#define Opt_transparent_all	(1LU<<14)
#define Opt_mc_hack		(1LU<<15)
#define Opt_tripleclickwords	(1LU<<16)
#define Opt_scrollWithBuffer	(1LU<<17)
#define Opt_jumpScroll		(1LU<<18)
#define Opt_mouseWheelScrollPage (1LU<<19)
/* place holder used for parsing command-line options */
#define Opt_Reverse		(1LU<<30)
#define Opt_Boolean		(1LU<<31)

#define DEFAULT_OPTIONS		(Opt_scrollBar | Opt_scrollTtyOutput \
				 | Opt_jumpScroll)

#define PROPFONT_NORMAL		(1<<0)
#define PROPFONT_BOLD		(1<<1)
#define PROPFONT_MULTI		(1<<2)

/* ------------------------------------------------------------------------- */

typedef enum {
    EUCJ, SJIS,			/* Japanese KANJI methods                    */
    BIG5, CNS,			/* Chinese BIG5 methods: CNS not implemented */
    GB,				/* Chinese GB method                         */
    EUCKR,			/* Korean method                             */
    NOENC			/* no encoding                               */
} ENC_METHOD;

typedef struct {
    short          method;
    void           (*func)(unsigned char *, int);
    char          *name;
} KNOWN_ENCODINGS;

typedef struct {
    short           state;
    Window          win;
} menuBar_t;

typedef struct {
    char            state;	/* scrollbar state                          */
    char            init;	/* scrollbar has been initialised           */
    short           beg;	/* slider sub-window begin height           */
    short           end;	/* slider sub-window end height             */
    short           top;	/* slider top position                      */
    short           bot;	/* slider bottom position                   */
    short           style;	/* style: rxvt, xterm, next                 */
    short           width;	/* scrollbar width                          */
    Window          win;
    int             (*update)(struct rxvt_vars *, int, int, int, int);
} scrollBar_t;

typedef struct rxvt_vars {
/*
 * These ``hidden'' items are not for public consumption and
 * must not be accessed externally
 */
    struct rxvt_hidden *h;
/*
 * Exposed items
 *   Changes to structure here require library version number change
 */
    TermWin_t       TermWin;
    scrollBar_t     scrollBar;
    menuBar_t       menuBar;
    Display        *Xdisplay;
    unsigned long   Options;
    XSizeHints      szHint;
    Colormap        Xcmap;
    Pixel          *PixColors;
    short           numPixColors;
    Cursor          TermWin_cursor;	/* cursor for vt window */
    int             Xdepth;
    int             sb_shadow;	/* scrollbar shadow width                    */
    int             Xfd;	/* file descriptor of X server connection    */
    int             cmd_fd;	/* pty file descriptor; connected to command */
    int             tty_fd;	/* tty file descriptor; connected to child   */
    int             num_fds;	/* number of file descriptors being used     */
    int             numlock_state;
    text_t        **drawn_text;	/* text drawn on screen (characters)         */
    rend_t        **drawn_rend;	/* text drawn on screen (rendition)          */
    text_t        **buf_text;
    rend_t        **buf_rend;
    char           *tabs;	/* per location: 1 == tab-stop               */
    screen_t        screen;
    screen_t        swap;
    selection_t     selection;
    sstyle_t        selection_style;
    ENC_METHOD      encoding_method;
} rxvt_t;

/*****************************************************************************
 *                                PROTOTYPES                                 *
 *****************************************************************************/
void             rxvt_main_loop(rxvt_t *);
rxvt_t          *rxvt_init(int, const char *const *);

#endif				/* _RXVTLIB_H_ */
